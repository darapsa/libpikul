#include <curl/curl.h>
#include <json.h>
#include "pikul.h"

extern struct shipping {
	CURL *handle;
	char *base;
	struct curl_slist *headers;
	const char **status_trail;
	char *url;
	char *post;
	json_tokener *tokener;
	enum {
		SERVICES,
		ORDER
	} mode;
	const char **trail;
	void *data;
} *shipping_list[];
