#ifndef PIKUL_H
#define PIKUL_H

enum pikul_company {
	PIKUL,
	PIKUL_ANTERAJA,
	PIKUL_END
};

struct pikul_service {
	char *code;
	char *name;
	char *etd;
	double cost;
};

#ifdef __cplusplus
extern "C" {
#endif

void pikul_init(enum pikul_company company, char *provisions[]);
struct pikul_service **pikul_services(enum pikul_company company,
		const char *origin, const char *destination, double weight);
void pikul_free_services(struct pikul_service **services);
char *pikul_html(char *origins[], char *destinations[], double weight,
                const char *widget, const char *extra, const char *name, const char *value,
                char *code_prefixes[], char *name_prefixes[]);
double pikul_cost(enum pikul_company company, const char *service,
		const char *origin, const char *destination, double weight);
char *pikul_order(enum pikul_company company, const char *order_number, const char *service,
		const char *sender_name, const char *sender_phone, const char *origin,
		const char *sender_address, const char *sender_postal,
		const char *receiver_name, const char *receiver_phone, const char *destination,
		const char *receiver_address, const char *receiver_postal,
                int nitems, char **items[], double subtotal);
void pikul_cleanup();

#ifdef __cplusplus
}
#endif

#endif
