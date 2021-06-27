#ifndef PIKUL_H
#define PIKUL_H

enum pikul_company {
	PIKUL_COMPANY_ANTERAJA,
	PIKUL_COMPANY_SICEPAT
};

struct pikul_service {
	char *code;
	char *name;
	char *etd;
	double cost;
};

struct pikul_services {
	size_t length;
	struct pikul_service *list[];
};

#ifdef __cplusplus
extern "C" {
#endif

void pikul_init(enum pikul_company company, char *provisions[]);
struct pikul_services *pikul_services(const char *origin, const char *destination, double weight);
void pikul_free_services(struct pikul_services *services);
double pikul_cost(const char *origin, const char *destination, double weight, const char *service);
void pikul_cleanup();

#ifdef __cplusplus
}
#endif

#endif
