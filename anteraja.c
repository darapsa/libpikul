#include "shipping.h"
#include "handler.h"

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
\"shipper\":{\
\"name\":\"%s\",\
\"phone\":\"%s\",\
\"district\":\"%s\",\
\"address\":\"%s\"\
},\
\"receiver\":{\
\"name\":\"%s\",\
\"phone\":\"%s\",\
\"district\":\"%s\",\
\"address\":\"%s\"\
},\
\"items\":[\
%s\
],\
\"declared_value\":%d\
}"
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

extern CURL *curl;

static const char *status_trail[] = { "status", NULL };
static char *prefix = NULL;

void anteraja_init(char *provisions[])
{
	enum { BASE_PATH, ACCESS_KEY_ID, SECRET_ACCESS_KEY, PREFIX };
	shipping.base = malloc(strlen(provisions[BASE_PATH]) + 1);
	strcpy(shipping.base, provisions[BASE_PATH]);
	headers((const char *[]){ "access-key-id", "secret-access-key", NULL }, &provisions[ACCESS_KEY_ID]);
	shipping.headers = curl_slist_append(shipping.headers, "Content-Type:application/json");
	if (provisions[PREFIX]) {
		prefix = malloc(strlen(provisions[PREFIX]) + 1);
		strcpy(prefix, provisions[PREFIX]);
	}
}

void anteraja_services(const char *origin, const char *destination, double weight, char **url, char **post)
{
	*url = malloc(strlen(shipping.base) + strlen(SERVICES_PATH) + 1);
	sprintf(*url, "%s%s", shipping.base, SERVICES_PATH);
	*post = malloc(strlen(SERVICES_POST) + strlen(origin) + strlen(destination) + SERVICES_WEIGHT
			- 2 * strlen("%s") - strlen("%d") + 1);
	sprintf(*post, SERVICES_POST, origin, destination, weight < 1.0 ? 1000 : (int)weight * 1000);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, *post);
}

size_t anteraja_services_handle(const char *contents, size_t size, size_t nmemb,
		struct pikul_services **services)
{
	size_t realsize = size * nmemb;
	handle(SERVICES, contents, realsize, status_trail, (const char *[]){
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

void anteraja_order(const char *trx_id, const char *service, const char *sender_name,
		const char *sender_phone, const char *origin, const char *sender_address,
		const char *receiver_name, const char *receiver_phone, const char *destination,
		const char *receiver_address, int nitems, char **items[], double subtotal,
		char **url, char **post)
{
	*url = malloc(strlen(shipping.base) + strlen(ORDER_PATH) + 1);
	sprintf(*url, "%s%s", shipping.base, ORDER_PATH);
	enum { SKU, QUANTITY, DESCRIPTION, PRICE, WEIGHT };
	char *json = NULL;
	for (int i = 0; i < nitems; i++) {
		size_t length = strlen(ORDER_ITEM) + strlen(items[i][DESCRIPTION]) + ORDER_ITEM_QUANTITY
			+ ORDER_ITEM_PRICE + ORDER_ITEM_WEIGHT - strlen("%s") - 3 * strlen("%d")
			+ strlen(",");
		char item[length + 1];
		sprintf(item, ORDER_ITEM, items[i][DESCRIPTION], atoi(items[i][QUANTITY]),
				atoi(items[i][PRICE]), atoi(items[i][WEIGHT]) * 1000);
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
	*post = malloc(strlen(ORDER_POST) + strlen(prefix) + strlen(trx_id) + strlen(service)
			+ strlen(sender_name) + strlen(sender_phone) + strlen(origin)
			+ strlen(sender_address) + strlen(receiver_name) + strlen(receiver_phone)
			+ strlen(destination) + strlen(receiver_address) + strlen(json) + ORDER_SUBTOTAL
			- 12 * strlen("%s") - strlen("%d") + 1);
	sprintf(*post, ORDER_POST, prefix, trx_id, service, sender_name, sender_phone, origin,
			sender_address, receiver_name, receiver_phone, destination, receiver_address, json,
			(int)subtotal);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, *post);
}

size_t anteraja_order_handle(const char *contents, size_t size, size_t nmemb, char **waybill)
{
	size_t realsize = size * nmemb;
	handle(ORDER, contents, realsize, status_trail, (const char *[]){
			"content",
			"waybill_no",
			NULL
			}, NULL, waybill);
	return realsize;
}

void anteraja_cleanup()
{
	if (prefix)
		free(prefix);
}
