#ifdef DEBUG
#include <stdio.h>
#endif
#include <string.h>
#include <json.h>
#include "private.h"

#define SELECT \
"<select name=\"%s\" %s>\n\
%s\
\t\t\t\t\t\t\t\t\t\t\t</select>"
#define SELECT_NUM_PARAMS 3
#define OPTION \
"\t\t\t\t\t\t\t\t\t\t\t\t<option value=\"%s\"%s>%s%s</option>\n"
#define OPTION_NUM_PARAMS 4

CURL *curl;
json_tokener *tokener;
struct shipping shipping;

const char **anteraja_init(char *[]);
const char **anteraja_services(const char *, const char *, double);
void anteraja_order(const char *, const char *, const char *, const char *, const char *, const char *,
		const char *, const char *, const char *, const char *, const char *, const char *, int,
		char **[], double);
void anteraja_cleanup();

static void recurse(struct json_object *outer, const char *trail[], struct json_object **last)
{
	struct json_object *inner = NULL;
	json_object_object_get_ex(outer, *trail, &inner);
	if (*++trail)
		recurse(inner, trail, last);
	else
		*last = inner;
}

static size_t handle(char *contents, size_t size, size_t nmemb, void *data)
{
	size_t realsize = size * nmemb;
#ifdef DEBUG
	contents[realsize] = '\0';
	fprintf(stderr, "%s\n", contents);
#endif
	json_object *response = json_tokener_parse_ex(tokener, contents, realsize);
	enum json_tokener_error error = json_tokener_get_error(tokener);
	if (!response) {
		if (error == json_tokener_continue)
			return realsize;
		else {
			json_tokener_reset(tokener);
			return realsize;
		}
	} else if (!json_object_is_type(response, json_type_object) || error != json_tokener_success)
		return realsize;
	struct json_object *status = NULL;
	recurse(response, shipping.status_trail, &status);
	if (json_object_get_int(status) != 200)
		return realsize;
	switch (shipping.mode) {
		case SERVICES:
			;
			struct json_object *array = NULL;
			recurse(response, shipping.trail, &array);
			size_t length = json_object_array_length(array);
			shipping.data = malloc(sizeof(struct pikul_services)
					+ sizeof(struct pikul_service *[length]));
			struct pikul_services *services  = (struct pikul_services *)shipping.data;
			services->length = length;
			const char **attributes = (const char **)data;
			enum { CODE, NAME, ETD, COST };
			for (size_t i = 0; i < length; i++) {
				services->list[i] = malloc(sizeof(struct pikul_service));
				struct pikul_service *service = services->list[i];
				json_object *object = json_object_array_get_idx(array, i);
				struct json_object_iterator iterator = json_object_iter_begin(object);
				struct json_object_iterator iterator_end = json_object_iter_end(object);
				while (!json_object_iter_equal(&iterator, &iterator_end)) {
					const char *name = json_object_iter_peek_name(&iterator);
					json_object *value = json_object_iter_peek_value(&iterator);
					if (!strcmp(name, attributes[COST]))
						service->cost = json_object_get_double(value);
					else {
						int len = json_object_get_string_len(value);
						if (len) {
							char *string = malloc(len + 1);
							strcpy(string, json_object_get_string(value));
							if (!strcmp(name, attributes[CODE]))
								service->code = string;
							else if (!strcmp(name, attributes[NAME]))
								service->name = string;
							else if (!strcmp(name, attributes[ETD]))
								service->etd = string;
						}
					}
					json_object_iter_next(&iterator);
				}
			}
			break;
		case ORDER:
			;
			struct json_object *string = NULL;
			recurse(response, shipping.trail, &string);
			shipping.data = malloc(json_object_get_string_len(string) + 1);
			strcpy(shipping.data, json_object_get_string(string));
			break;
		default:
			break;
	}
	return realsize;
}

void pikul_init(enum pikul_company company, char *provisions[])
{
	curl_global_init(CURL_GLOBAL_SSL);
	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handle);
#ifdef DEBUG
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif
	shipping.company = company;
	shipping.headers = NULL;
	const char **fields;
	switch (company) {
		case PIKUL_ANTERAJA:
			fields = anteraja_init(provisions);
			break;
		default:
			break;
	}
	while (*fields) {
		char header[strlen(*fields) + strlen(":") + strlen(*provisions) + 1];
		sprintf(header, "%s:%s", *fields++, *provisions++);
		shipping.headers = curl_slist_append(shipping.headers, header);
	}
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, shipping.headers);
	tokener = json_tokener_new();
}

struct pikul_services *pikul_services(const char *origin, const char *destination, double weight)
{
	shipping.post = NULL;
	const char **attributes;
	switch (shipping.company) {
		case PIKUL_ANTERAJA:
			attributes = anteraja_services(origin, destination, weight);
			break;
		default:
			break;
	}
	curl_easy_setopt(curl, CURLOPT_URL, shipping.url);
	if (shipping.post)
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, shipping.post);
	shipping.mode = SERVICES;
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, attributes);
	curl_easy_perform(curl);
	if (shipping.post)
		free(shipping.post);
	free(shipping.url);
	return (struct pikul_services *)shipping.data;
}

static inline void free_service(struct pikul_service *service)
{
	if (service->etd)
		free(service->etd);
	if (service->name)
		free(service->name);
	free(service->code);
	free(service);
}

void pikul_free_services(struct pikul_services *services)
{
	for (size_t i = 0; i < services->length; i++)
		free_service(services->list[i]);
	free(services);
}

static int servicecmp(const void *service1, const void *service2)
{
	return strcmp((*(struct pikul_service * const *)service1)->code,
			(*(struct pikul_service * const *)service2)->code);
}

char *pikul_html(const char *origin, const char *destination, double weight,
                const char *widget, const char *extra, const char *name, const char *value,
                char *code_prefixes[], char *name_prefixes[])
{
	struct pikul_services *services = pikul_services(origin, destination, weight);
        char *html;
        if (!strcmp(widget, "select")) {
                char *options = NULL;
		if (!services || !services->length) {
			static const char *empty = "<option value=\"\">Not enough information</option>";
			options = malloc(strlen(empty) + 1);
			strcpy(options, empty);
		} else {
			for (size_t i = 0; i < services->length; i++) {
				struct pikul_service *service = services->list[i];
				char *code_prefix = code_prefixes[shipping.company];
				char *name_prefix = name_prefixes[shipping.company];
				size_t code_length = strlen(code_prefix) + strlen(service->code);
				char code[code_length + 1];
				sprintf(code, "%s%s", code_prefix, service->code);
				_Bool selected = !strcmp(code, value);
				size_t length = strlen(OPTION) + code_length
					+ (selected ? strlen(" selected") : 0)
					+ strlen(name_prefix) + strlen(service->name)
					- OPTION_NUM_PARAMS * strlen("%s");
				char option[length + 1];
				sprintf(option, OPTION, code, selected ? " selected" : "",
						name_prefix, service->name);
				if (options)
					options = realloc(options, strlen(options) + length + 1);
				else {
					options = malloc(length + 1);
					memset(options, '\0', strlen(options));
				}
				strcat(options, option);
			}
                }
                html = malloc(strlen(SELECT) + strlen(name) + (extra ? strlen(extra) : 0) + strlen(options)
                                - SELECT_NUM_PARAMS * strlen("%s") + 1);
                sprintf(html, SELECT, name, extra ? extra : "", options);
        }
        return html;
}

double pikul_cost(const char *origin, const char *destination, double weight, const char *code)
{
	struct pikul_services *services = pikul_services(origin, destination, weight);
	if (!services || !services->length)
		return .0;
	qsort(services->list, services->length, sizeof(struct pikul_service *), servicecmp);
	struct pikul_service *key_service = malloc(sizeof(struct pikul_service));
	memset(key_service, '\0', sizeof(struct pikul_service));
	key_service->code = malloc(strlen(code) + 1);
	strcpy(key_service->code, code);
	double cost = (*(struct pikul_service **)bsearch(&key_service, services->list,
				services->length, sizeof(struct pikul_service *), servicecmp))->cost;
	free_service(key_service);
	pikul_free_services(services);
	return cost;
}

char *pikul_order(const char *order_number, const char *service, const char *sender_name,
		const char *sender_phone, const char *origin, const char *sender_address,
		const char *sender_postal, const char *receiver_name, const char *receiver_phone,
		const char *destination, const char *receiver_address, const char *receiver_postal,
		int nitems, char **items[], double subtotal)
{
	shipping.post = NULL;
	switch (shipping.company) {
		case PIKUL_ANTERAJA:
			anteraja_order(order_number, service, sender_name, sender_phone, origin,
                                        sender_address, sender_postal, receiver_name, receiver_phone,
					destination, receiver_address, receiver_postal, nitems, items,
					subtotal);
			break;
		default:
			break;
	}
	curl_easy_setopt(curl, CURLOPT_URL, shipping.url);
	if (shipping.post) {
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, shipping.post);
#ifdef DEBUG
		fprintf(stderr, "POST: %s\n", shipping.post);
#endif
	}
	shipping.mode = ORDER;
	curl_easy_perform(curl);
	if (shipping.post)
		free(shipping.post);
	free(shipping.url);
	return (char *)shipping.data;
}

void pikul_cleanup()
{
	switch (shipping.company) {
		case PIKUL_ANTERAJA:
			anteraja_cleanup();
			break;
		default:
			break;
	}
	/*
	if (shipping.data)
                switch (shipping.mode) {
                        case SERVICES:
                                pikul_free_services(shipping.data);
                                break;
                        case ORDER:
                                free(shipping.data);
                                break;
                        default:
                                break;
                }
		*/
	free(shipping.base);
	json_tokener_free(tokener);
	curl_slist_free_all(shipping.headers);
	curl_easy_cleanup(curl);
	curl_global_cleanup();
}
