#include <curl/curl.h>
#include "pikul.h"

struct shipping {
	enum pikul_company company;
	char *base;
	struct curl_slist *headers;
};

void headers(struct shipping *, const char *[], char *[]);
