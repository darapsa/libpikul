#include <string.h>
#include <json.h>
#include "pikul.h"

extern json_tokener *tokener;
void recurse(struct json_object *, const char *[], struct json_object **);

inline void handle_services(const char *contents, size_t num_bytes, const char *status_trail[],
		const char *trail[], const char *attributes[], struct pikul_services **services)
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
	struct json_object *status = NULL;
	recurse(response, status_trail, &status);
	if (json_object_get_int(status) != 200)
		return;
	struct json_object *array = NULL;
	recurse(response, trail, &array);
	size_t length = json_object_array_length(array);
	*services = malloc(sizeof(struct pikul_services) + sizeof(struct pikul_service *[length]));
	(*services)->length = length;
	enum { CODE, NAME, ETD, COST };
	for (size_t i = 0; i < length; i++) {
		(*services)->list[i] = malloc(sizeof(struct pikul_service));
		struct pikul_service *service = (*services)->list[i];
		json_object *object = json_object_array_get_idx(array, i);
		struct json_object_iterator iterator = json_object_iter_begin(object);
		struct json_object_iterator iterator_end = json_object_iter_end(object);
		while (!json_object_iter_equal(&iterator, &iterator_end)) {
			const char *name = json_object_iter_peek_name(&iterator);
			json_object *value = json_object_iter_peek_value(&iterator);
			if (!strcmp(name, attributes[COST]))
				service->cost = json_object_get_double(value);
			else {
				int len = json_object_get_string_len(value);
				if (len) {
					char *string = malloc(len + 1);
					strcpy(string, json_object_get_string(value));
					if (!strcmp(name, attributes[CODE]))
						service->code = string;
					else if (!strcmp(name, attributes[NAME]))
						service->name = string;
					else if (!strcmp(name, attributes[ETD]))
						service->etd = string;
				}
			}
			json_object_iter_next(&iterator);
		}
	}
}
