#include <curl/curl.h>
#include "pikul.h"

extern struct shipping {
	enum pikul_company company;
	char *base;
	struct curl_slist *headers;
	const char **status_trail;
	char *url;
	char *post;
	enum {
		SERVICES,
		ORDER
	} mode;
	const char **trail;
	void *data;
} shipping;
