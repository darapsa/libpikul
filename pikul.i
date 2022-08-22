%module Pikul
%{
#include "pikul.h"
%}

#ifdef SWIGJAVASCRIPT
%inline %{
        char *provisions[3];
        void provisions_set(int i, char *val)
        {
                if (provisions[i])
                        free(provisions[i]);
                provisions[i] = (char *)malloc(strlen(val) + 1);
                strcpy(provisions[i], val);
        }
        char *provisions_get(int i)
        {
                return provisions[i];
        }
        void provisions_clear()
        {
                for (int i = 0; i < 3; i++)
                        if (provisions[i]) {
                                free(provisions[i]);
                                provisions[i] = NULL;
                        }
        }
%}
#endif

#ifdef SWIGPERL
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
#endif

%rename("%(strip:[pikul_])s") "";
%include "pikul.h"
