#include <string.h>
#include <json.h>
#include "pikul.h"

enum {
	KEY_CODE,
	KEY_NAME,
	KEY_ETD,
	KEY_COST,
	KEY_OBJECTS
};

struct container {
	struct pikul_services **services;
	const char **keys[7];
};

extern json_tokener *tokener;
void recurse(struct json_object *, const char *[], struct json_object **);

inline void handle(const char *contents, size_t num_bytes, struct container *container)
{
	json_object *response = json_tokener_parse_ex(tokener, contents, num_bytes);
	enum json_tokener_error error = json_tokener_get_error(tokener);
	if (!response) {
		if (error == json_tokener_continue)
			return;
		else {
			json_tokener_reset(tokener);
			return;
		}
	} else if (!json_object_is_type(response, json_type_object) || error != json_tokener_success)
		return;
	struct json_object *services = NULL;
	recurse(response, &(*container->keys)[KEY_OBJECTS], &services);
	size_t length = json_object_array_length(services);
	*(container->services) = malloc(sizeof(struct pikul_services)
			+ sizeof(struct pikul_service *[length]));
	(*(container->services))->length = length;
	for (size_t i = 0; i < length; i++) {
		(*(container->services))->list[i] = malloc(sizeof(struct pikul_service));
		struct pikul_service *service = (*(container->services))->list[i];
		json_object *object = json_object_array_get_idx(services, i);
		struct json_object_iterator iterator = json_object_iter_begin(object);
		struct json_object_iterator iterator_end = json_object_iter_end(object);
		while (!json_object_iter_equal(&iterator, &iterator_end)) {
			const char *key = json_object_iter_peek_name(&iterator);
			json_object *val = json_object_iter_peek_value(&iterator);
			if (!strcmp(key, (*container->keys)[KEY_COST]))
				service->cost = json_object_get_double(val);
			else {
				int len = json_object_get_string_len(val);
				if (len) {
					char *value = malloc(len + 1);
					strcpy(value, json_object_get_string(val));
					if (!strcmp(key, (*container->keys)[KEY_CODE]))
						service->code = value;
					else if (!strcmp(key, (*container->keys)[KEY_NAME]))
						service->name = value;
					else if (!strcmp(key, (*container->keys)[KEY_ETD]))
						service->etd = value;
				}
			}
			json_object_iter_next(&iterator);
		}
	}
}
