#ifdef DEBUG
#include <stdio.h>
#endif
#include "shipping.h"
#include "handler.h"

#define POST \
"{\
\"origin\":\"%s\",\
\"destination\":\"%s\",\
\"weight\":%d\
}"
extern CURL *curl;

static const char *status_trail[] = { "status", NULL };

void anteraja_init(char *provisions[], struct shipping *shipping)
{
	enum { BASE_PATH, ACCESS_KEY };
	shipping->base = malloc(strlen(provisions[BASE_PATH]) + 1);
	strcpy(shipping->base, provisions[BASE_PATH]);
	headers((const char *[]){ "access-key-id", "secret-access-key", NULL },
			&provisions[ACCESS_KEY], shipping);
	shipping->headers = curl_slist_append(shipping->headers, "Content-Type:application/json");
}

void anteraja_services(const char *origin, const char *destination, double weight,
		struct shipping *shipping, char **url, char **post)
{
	static const char *path = "serviceRates";
	*url = malloc(strlen(shipping->base) + strlen(path) + 1);
	sprintf(*url, "%s%s", shipping->base, path);
	*post = malloc(strlen(POST) + strlen(origin) + strlen(destination) + strlen("50000")
			- 2 * strlen("%s") - strlen("%d") + 1);
	sprintf(*post, POST, origin, destination, (int)weight * 1000);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, *post);
}

size_t anteraja_services_handle(const char *contents, size_t size, size_t nmemb,
		struct pikul_services **services)
{
	size_t realsize = size * nmemb;
#ifdef DEBUG
	((char *)contents)[realsize] = '\0';
	fprintf(stderr, "%s\n", contents);
#endif
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
