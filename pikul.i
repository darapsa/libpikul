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
%typemap(out) char ** {
        int len = 0;
        while ($1[len])
                len++;
        SV **svs = malloc(len * sizeof(SV *));
        for (int i = 0; i < len; i++) {
                svs[i] = sv_newmortal();
                sv_setpv((SV *)svs[i], $1[i]);
        };
        AV *myav = av_make(len, svs);
        free(svs);
        $result = newRV_noinc((SV *)myav);
        sv_2mortal($result);
        argvi++;
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

%rename("%(strip:[pikul_])s") "";
void pikul_init(enum pikul_company, char *[]);
char **pikul_codes(const char *, const char *, double);
double pikul_cost(const char *, const char *, double, const char *);
char *pikul_order(const char *, const char *, const char *, const char *, const char *, const char *,
                const char *, const char *, const char *, const char *, const char *, const char *, int,
                char **[], double);
void pikul_cleanup();
