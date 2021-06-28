#include "shipping.h"
#include "handler.h"

CURL *curl;
json_tokener *tokener;
static struct shipping shipping;

extern inline void headers(struct shipping *shipping, const char *fields[], char *provisions[]);
extern inline void handle(const char *, size_t, struct container *);

extern void anteraja_init(char *[], struct shipping *);
extern void anteraja_services(const char *, const char *, double,
		struct shipping *, char **, char **);
extern size_t anteraja_services_handle(const char *, size_t, size_t, struct pikul_services **);

void pikul_init(enum pikul_company company, char *provisions[])
{
	curl_global_init(CURL_GLOBAL_SSL);
	curl = curl_easy_init();
#ifdef DEBUG
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif
	shipping.company = company;
	switch (company) {
		case PIKUL_COMPANY_ANTERAJA:
			anteraja_init(provisions, &shipping);
			break;
		default:
			break;
	}
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, shipping.headers);
	tokener = json_tokener_new();
}

void recurse(struct json_object *outer, const char *keys[], struct json_object **services)
{
	struct json_object *inner = NULL;
	json_object_object_get_ex(outer, *keys, &inner);
	if (*++keys)
		recurse(inner, keys, services);
	else
		*services = inner;
}

struct pikul_services *pikul_services(const char *origin, const char *destination, double weight)
{
	struct pikul_services *services = NULL;
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &services);
	char *url;
	char *post = NULL;
	size_t (*handler)(const char *, size_t, size_t, struct pikul_services **);
	switch (shipping.company) {
		case PIKUL_COMPANY_ANTERAJA:
			anteraja_services(origin, destination, weight, &shipping, &url, &post);
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

void pikul_cleanup()
{
	free(shipping.base);
	json_tokener_free(tokener);
	curl_slist_free_all(shipping.headers);
	curl_easy_cleanup(curl);
	curl_global_cleanup();
}
