#include "shipping.h"
#include "handler.h"

extern CURL *curl;

void anteraja_init(char *provisions[], struct shipping *shipping)
{
	shipping->base = malloc(strlen(*provisions) + 1);
	strcpy(shipping->base, *provisions);
	headers(shipping, (const char *[]){ "access-key-id", "secret-access-key", NULL }, ++provisions);
	shipping->headers = curl_slist_append(shipping->headers, "Content-Type:application/json");
}

void anteraja_services_request(const char *origin, const char *destination, double weight,
		struct shipping *shipping, char **url, char **post)
{
	static const char *path = "serviceRates";
	*url = malloc(strlen(shipping->base) + strlen(path) + 1);
	sprintf(*url, "%s%s", shipping->base, path);
	static const char *format = "{"
		"\"origin\": \"xx.xx.xx\","
		"\"destination\": \"xx.xx.xx\","
		"\"weight\": xxxxx"
		"}";
	*post = malloc(strlen(format) + 1);
	sprintf(*post, "{\"origin\": \"%s\",\"destination\": \"%s\",\"weight\": %d}",
			origin, destination, (int)weight);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, *post);
}

size_t anteraja_services_handle(const char *contents, size_t size, size_t nmemb,
		struct pikul_services **services)
{
	size_t realsize = size * nmemb;
	handle(contents, realsize, &(struct container){ services, (const char *[]){
		"product_code",
		"product_name",
		"etd",
		"rates",
		"content",
		"services",
		NULL
	}});
	return realsize;
}
