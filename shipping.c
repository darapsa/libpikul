#include <string.h>
#include "shipping.h"

void headers(struct shipping *shipping, const char *fields[], char *provisions[])
{
	shipping->headers = NULL;
	while (*fields) {
		char header[strlen(*fields) + strlen(*provisions) + 2];
		sprintf(header, "%s:%s", *fields++, *provisions++);
		shipping->headers = curl_slist_append(shipping->headers, header);
	}
}
