#ifdef DEBUG
#include <stdio.h>
#endif
#include <string.h>
#include <curl/curl.h>
#include <json.h>
#include "pikul.h"

extern struct shipping {
	enum pikul_company company;
	char *base;
	struct curl_slist *headers;
} shipping;
extern json_tokener *tokener;

void recurse(struct json_object *, const char *[], struct json_object **);

enum type { SERVICES, ORDER };

inline void headers(const char *fields[], char *provisions[])
{
	shipping.headers = NULL;
	while (*fields) {
		char header[strlen(*fields) + strlen(*provisions) + 2];
		sprintf(header, "%s:%s", *fields++, *provisions++);
		shipping.headers = curl_slist_append(shipping.headers, header);
	}
}

inline void handle(enum type type, const char *contents, size_t num_bytes, const char *status_trail[],
		const char *trail[], const char *attributes[], void *data)
{
#ifdef DEBUG
	((char *)contents)[num_bytes] = '\0';
	fprintf(stderr, "%s\n", contents);
#endif
	json_object *response = json_tokener_parse_ex(tokener, contents, num_bytes);
	enum json_tokener_error error = json_tokener_get_error(tokener);
	if (!response) {
		if (error == json_tokener_continue)
			return;
		else {
			json_tokener_reset(tokener);
			return;
		}
	} else if (!json_object_is_type(response, json_type_object) || error != json_tokener_success)
		return;
	struct json_object *status = NULL;
	recurse(response, status_trail, &status);
	if (json_object_get_int(status) != 200)
		return;
	switch (type) {
		case SERVICES:
			;
			struct json_object *array = NULL;
			recurse(response, trail, &array);
			size_t length = json_object_array_length(array);
			struct pikul_services **services  = (struct pikul_services **)data;
			*services = malloc(sizeof(struct pikul_services)
					+ sizeof(struct pikul_service *[length]));
			(*services)->length = length;
			enum { CODE, NAME, ETD, COST };
			for (size_t i = 0; i < length; i++) {
				(*services)->list[i] = malloc(sizeof(struct pikul_service));
				struct pikul_service *service = (*services)->list[i];
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
			recurse(response, trail, &string);
			char **tracking_number = (char **)data;
			*tracking_number = malloc(json_object_get_string_len(string) + 1);
			strcpy(*tracking_number, json_object_get_string(string));
			break;
		default:
			break;
	}
}
