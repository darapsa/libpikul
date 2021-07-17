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
        AV *items = (AV *)SvRV($input);
        I32 nitems_min1 = av_len(items);
        $1 = (char ***)malloc((nitems_min1 + 2) * sizeof(char **));
        int i;
        for (i = 0; i <= nitems_min1; i++) {
                SV **refptr = av_fetch(items, i, 0);
                AV *flat = (AV *)SvRV(*refptr);
                I32 nattrs_min1 = av_len(flat);
                $1[i] = (char **)malloc((nattrs_min1 + 2) * sizeof(char *));
                int j;
                for (j = 0; j <= nattrs_min1; j++) {
                        SV **tv = av_fetch(flat, j, 0);
                        $1[i][j] = (char *)SvPV(*tv, PL_na);
                }
                $1[i][j] = NULL;
        }
        $1[i] = NULL;
};
%typemap(freearg) char **[] {
        free($1);
}

%typemap(in) _Bool {
        $1 = $input;
}

%rename("%(strip:[pikul_])s") "";
void pikul_init(enum pikul_company company, char *provisions[]);
double pikul_cost(const char *origin, const char *destination, double weight, const char *service);
char *pikul_order(const char *order_number, const char *service, const char *sender_name,
                const char *sender_phone, const char *origin, const char *sender_address,
                const char *sender_postal, const char *receiver_name, const char *receiver_phone,
                const char *destination, const char *receiver_address, const char *receiver_postal,
                int nitems, char **items[], _Bool insurance, double subtotal);
void pikul_cleanup();
