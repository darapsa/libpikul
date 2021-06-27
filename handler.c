#include "handler.h"

extern inline void handle(const char *, size_t, struct container *);

void recurse(struct json_object *outer, const char *keys[], struct json_object **services)
{
	struct json_object *inner = NULL;
	json_object_object_get_ex(outer, *keys, &inner);
	if (*++keys)
		recurse(inner, keys, services);
	else
		*services = inner;
}
