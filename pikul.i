%module Pikul
%{
#include "pikul.h"
%}

%typemap(in) char *[] {
        AV *tempav = (AV *)SvRV($input);
        I32 len = av_len(tempav);
        $1 = (char **)malloc((len + 2) * sizeof(char *));
        int i;
        for (i = 0; i <= len; i++) {
                SV **tv = av_fetch(tempav, i, 0);
                $1[i] = (char *)SvPV(*tv, PL_na);
        }
        $1[i] = NULL;
};
%typemap(freearg) char *[] {
        free($1);
}

%typemap(in) char **[] {
        AV *tempav = (AV *)SvRV($input);
        I32 len = av_len(tempav);
        $1 = (char ***)malloc((len + 2) * sizeof(char **));
        int i;
        for (i = 0; i <= len; i++) {
                AV *av = (AV *)av_fetch(tempav, i, 0);
                I32 avlen = av_len(av);
                $1[i] = (char **)malloc((avlen + 2) * sizeof(char *));
                int j;
                for (j = 0; j <= avlen; j++) {
                        SV **tv = av_fetch(av, j, 0);
                        $1[i][j] = (char *)SvPV(*tv, PL_na);
                }
                $1[i][j] = NULL;
        }
};
%typemap(freearg) char **[] {
        free($1);
}

%rename("%(strip:[pikul_])s") "";
void pikul_init(enum pikul_company company, char *provisions[]);
double pikul_cost(const char *origin, const char *destination, double weight, const char *service);
char *pikul_order(const char *trx_id, const char *service, const char *sender_name,
                const char *sender_phone, const char *origin, const char *sender_address,
                const char *receiver_name, const char *receiver_phone, const char *destination,
                const char *receiver_address, int nitems, char **items[], _Bool insurance, double subtotal);
void pikul_cleanup();
