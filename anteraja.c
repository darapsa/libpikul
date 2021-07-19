#include <stdlib.h>
#include <string.h>
#include "private.h"

#define SERVICES_PATH "serviceRates"
#define SERVICES_POST \
"{\
\"origin\":\"%s\",\
\"destination\":\"%s\",\
\"weight\":%d\
}"
#define SERVICES_WEIGHT 5

#define ORDER_PATH "order"
#define ORDER_POST \
"{\
\"booking_id\":\"%s-%s\",\
\"service_code\":\"%s\",\
\"parcel_total_weight\":%d,\
\"shipper\":{\
\"name\":\"%s\",\
\"phone\":\"%s\",\
\"district\":\"%s\",\
\"address\":\"%s\",\
\"postcode\":\"%s\"\
},\
\"receiver\":{\
\"name\":\"%s\",\
\"phone\":\"%s\",\
\"district\":\"%s\",\
\"address\":\"%s\",\
\"postcode\":\"%s\"\
},\
\"items\":[\
%s\
],\
\"use_insurance\":%s,\
\"declared_value\":%d\
}"
#define ORDER_WEIGHT 5
#define ORDER_INSURANCE strlen("false")
#define ORDER_SUBTOTAL 9

#define ORDER_ITEM \
"{\
\"item_name\":\"%s\",\
\"item_quantity\":%d,\
\"declared_value\":%d,\
\"weight\":%d\
}"
#define ORDER_ITEM_QUANTITY 2
#define ORDER_ITEM_PRICE 9
#define ORDER_ITEM_WEIGHT 5

static char *prefix = NULL;

const char **anteraja_init(char *provisions[])
{
	enum { ACCESS_KEY_ID, SECRET_ACCESS_KEY, BASE_PATH, PREFIX };
	shipping.base = malloc(strlen(provisions[BASE_PATH]) + 1);
	strcpy(shipping.base, provisions[BASE_PATH]);
	if (provisions[PREFIX]) {
		prefix = malloc(strlen(provisions[PREFIX]) + 1);
		strcpy(prefix, provisions[PREFIX]);
	}
	shipping.headers = curl_slist_append(shipping.headers, "Content-Type:application/json");
	static const char *status_trail[] = { "status", NULL };
	shipping.status_trail = status_trail;
	static const char *fields[] = { "access-key-id", "secret-access-key", NULL };
	return fields;
}

const char **anteraja_services(const char *origin, const char *destination, double weight)
{
	shipping.url = malloc(strlen(shipping.base) + strlen(SERVICES_PATH) + 1);
	sprintf(shipping.url, "%s%s", shipping.base, SERVICES_PATH);
	shipping.post = malloc(strlen(SERVICES_POST) + strlen(origin) + strlen(destination)
			+ SERVICES_WEIGHT - 2 * strlen("%s") - strlen("%d") + 1);
	sprintf(shipping.post, SERVICES_POST, origin, destination,
			weight < 1.0 ? 1000 : (int)weight * 1000);
	static const char *trail[] = {
		"content",
		"services",
		NULL
	};
	shipping.trail = trail;
	static const char *attributes[] = {
		"product_code",
		"product_name",
		"etd",
		"rates"
	};
	return attributes;
}

void anteraja_order(const char *order_number, const char *service, const char *sender_name,
		const char *sender_phone, const char *origin, const char *sender_address,
		const char *sender_postal, const char *receiver_name, const char *receiver_phone,
		const char *destination, const char *receiver_address, const char *receiver_postal,
		int nitems, char **items[], double subtotal)
{
	shipping.url = malloc(strlen(shipping.base) + strlen(ORDER_PATH) + 1);
	sprintf(shipping.url, "%s%s", shipping.base, ORDER_PATH);
	enum { SKU, QUANTITY, DESCRIPTION, PRICE, WEIGHT };
	char *json = NULL;
	double total_weight = .0;
	for (int i = 0; i < nitems; i++) {
		size_t length = strlen(ORDER_ITEM) + strlen(items[i][DESCRIPTION]) + ORDER_ITEM_QUANTITY
			+ ORDER_ITEM_PRICE + ORDER_ITEM_WEIGHT - strlen("%s") - 3 * strlen("%d")
			+ strlen(",");
		char item[length + 1];
		int quantity = atoi(items[i][QUANTITY]);
		if (quantity > 1)
			nitems -= quantity - 1;
		double price = atof(items[i][PRICE]);
		if (price < 1000.0)
			price = 1000.0;
		double weight = atof(items[i][WEIGHT]) * 1000.0;
		if (weight < 100.0)
			weight = 100.0;
		total_weight += weight * quantity;
		sprintf(item, ORDER_ITEM, items[i][DESCRIPTION], quantity, (int)price, (int)weight);
		if (json)
			json = realloc(json, strlen(json) + length + 1);
		else {
			json = malloc(length + 1);
			memset(json, '\0', strlen(json));
		}
		strcat(json, item);
		if (i + 1 < nitems)
			strcat(json, ",");
		else
			json[strlen(json)] = '\0';
	}
	shipping.post = malloc(strlen(ORDER_POST) + strlen(prefix) + strlen(order_number) + strlen(service)
			+ ORDER_WEIGHT + strlen(sender_name) + strlen(sender_phone) + strlen(origin)
			+ strlen(sender_address) + strlen(sender_postal) + strlen(receiver_name)
			+ strlen(receiver_phone) + strlen(destination) + strlen(receiver_address)
			+ strlen(receiver_postal) + strlen(json) + ORDER_INSURANCE + ORDER_SUBTOTAL
			- 15 * strlen("%s") - 2 * strlen("%d") + 1);
	sprintf(shipping.post, ORDER_POST, prefix, order_number, service, (int)total_weight, sender_name,
			sender_phone, origin, sender_address, sender_postal, receiver_name, receiver_phone,
			destination, receiver_address, receiver_postal, json,
			total_weight < 1000.0 ? "true" : "false", subtotal < 1000.0 ? 1000 : (int)subtotal);
	static const char *trail[] = {
		"content",
		"waybill_no",
		NULL
	};
	shipping.trail = trail;
}

void anteraja_cleanup()
{
	if (prefix)
		free(prefix);
}
