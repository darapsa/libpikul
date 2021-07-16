#include "shipping.h"
#include "handler.h"

#define SERVICES_PATH "serviceRates"
#define SERVICES_POST \
"{\
\"origin\":\"%s\",\
\"destination\":\"%s\",\
\"weight\":%d\
}"

extern CURL *curl;

static const char *status_trail[] = { "status", NULL };

void anteraja_init(char *provisions[])
{
	enum { BASE_PATH, ACCESS_KEY_ID, SECRET_ACCESS_KEY, PREFIX };
	shipping.base = malloc(strlen(provisions[BASE_PATH]) + 1);
	strcpy(shipping.base, provisions[BASE_PATH]);
	headers((const char *[]){ "access-key-id", "secret-access-key", NULL }, &provisions[ACCESS_KEY_ID]);
	shipping.headers = curl_slist_append(shipping.headers, "Content-Type:application/json");
}

void anteraja_services(const char *origin, const char *destination, double weight, char **url, char **post)
{
	*url = malloc(strlen(shipping.base) + strlen(SERVICES_PATH) + 1);
	sprintf(*url, "%s%s", shipping.base, SERVICES_PATH);
	*post = malloc(strlen(SERVICES_POST) + strlen(origin) + strlen(destination) + strlen("50000")
			- 2 * strlen("%s") - strlen("%d") + 1);
	sprintf(*post, SERVICES_POST, origin, destination, weight < 1.0 ? 1000 : (int)weight * 1000);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, *post);
}

size_t anteraja_services_handle(const char *contents, size_t size, size_t nmemb,
		struct pikul_services **services)
{
	size_t realsize = size * nmemb;
	handle_services(contents, realsize, status_trail, (const char *[]){
			"content",
			"services",
			NULL
			}, (const char *[]){
			"product_code",
			"product_name",
			"etd",
			"rates"
			}, services);
	return realsize;
}
