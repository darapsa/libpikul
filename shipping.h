#include <string.h>
#include <curl/curl.h>
#include "pikul.h"

extern struct shipping {
	enum pikul_company company;
	char *base;
	struct curl_slist *headers;
} shipping;

inline void headers(const char *fields[], char *provisions[])
{
	shipping.headers = NULL;
	while (*fields) {
		char header[strlen(*fields) + strlen(*provisions) + 2];
		sprintf(header, "%s:%s", *fields++, *provisions++);
		shipping.headers = curl_slist_append(shipping.headers, header);
	}
}
