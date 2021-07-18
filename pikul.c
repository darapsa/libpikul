#include "shipping.h"
#include "handler.h"

CURL *curl;
json_tokener *tokener;
struct shipping shipping;

extern inline void headers(const char *[], char *[]);
extern inline void handle(enum type, const char *, size_t, const char *[], const char *[], const char *[],
		void *);

extern void anteraja_init(char *[]);
extern void anteraja_services(const char *, const char *, double, char **, char **);
extern size_t anteraja_services_handle(const char *, size_t, size_t, struct pikul_services **);
extern void anteraja_order(const char *, const char *, const char *, const char *, const char *,
		const char *, const char *, const char *, const char *, const char *, const char *,
		const char *, int, char **[], double, char **, char **);
extern size_t anteraja_order_handle(const char *, size_t size, size_t nmemb, char **);
extern void anteraja_cleanup();

void pikul_init(enum pikul_company company, char *provisions[])
{
	curl_global_init(CURL_GLOBAL_SSL);
	curl = curl_easy_init();
#ifdef DEBUG
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif
	shipping.company = company;
	switch (company) {
		case PIKUL_ANTERAJA:
			anteraja_init(provisions);
			break;
		default:
			break;
	}
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, shipping.headers);
	tokener = json_tokener_new();
}

void recurse(struct json_object *outer, const char *trail[], struct json_object **last)
{
	struct json_object *inner = NULL;
	json_object_object_get_ex(outer, *trail, &inner);
	if (*++trail)
		recurse(inner, trail, last);
	else
		*last = inner;
}

struct pikul_services *pikul_services(const char *origin, const char *destination, double weight)
{
	struct pikul_services *services = NULL;
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &services);
	char *url;
	char *post = NULL;
	size_t (*handler)(const char *, size_t, size_t, struct pikul_services **);
	switch (shipping.company) {
		case PIKUL_ANTERAJA:
			anteraja_services(origin, destination, weight, &url, &post);
			handler = anteraja_services_handle;
			break;
		default:
			break;
	}
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handler);
	curl_easy_perform(curl);
	if (post)
		free(post);
	free(url);
	return services;
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
	char *tracking_number = NULL;
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &tracking_number);
	char *url;
	char *post = NULL;
	size_t (*handler)(const char *, size_t, size_t, char **);
	switch (shipping.company) {
		case PIKUL_ANTERAJA:
			anteraja_order(order_number, service, sender_name, sender_phone, origin,
                                        sender_address, sender_postal, receiver_name, receiver_phone,
					destination, receiver_address, receiver_postal, nitems, items,
					subtotal, &url, &post);
			handler = anteraja_order_handle;
			break;
		default:
			break;
	}
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handler);
	curl_easy_perform(curl);
	if (post)
		free(post);
	free(url);
	return tracking_number;
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
	free(shipping.base);
	json_tokener_free(tokener);
	curl_slist_free_all(shipping.headers);
	curl_easy_cleanup(curl);
	curl_global_cleanup();
}
