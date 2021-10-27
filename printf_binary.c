#if __STDC_VERSION__ >= 199901L
 
#include <limits.h>
#include <wchar.h>
#include <stdio.h>
#include <printf.h>
#include <locale.h>
 
static int printf_binary_handler(FILE *s, const struct printf_info *info, const void *const *args)
{
    const char *g = 0;
    struct lconv *loc = 0;
    int group = 0, arr = 0, total = 0, len, digits;
    unsigned long long value = info->is_long_double ? *(const unsigned long long *)args[0] :
                               info->is_long        ? *(const unsigned long *)     args[0] :
                               info->is_char        ? *(const unsigned char *)     args[0] :
                               info->is_short       ? *(const unsigned short *)    args[0] :
                                                      *(const unsigned int *)      args[0] ;
 
    char buf[sizeof value * CHAR_BIT], *p = buf;
 
    while(value) *p++ = '0' + (value & 1), value >>= 1;
    len = p - buf;
    digits = info->prec < 0 ? 1 : info->prec;
    if(len > digits) digits = len;
    if(info->alt) {
        if(digits += -digits & 3) total = digits + (digits - 1) / 4 * !!info->group;
    } else {
        total = digits;
        group = info->group && (loc = localeconv(), *(g = loc->grouping));
        if(group) {
            while(*g > 0 && *g < CHAR_MAX) {
                if(digits - arr <= *g) break;
                ++total;
                arr += *g++;
            }
            if(!*g) total += (digits - arr - 1) / g[-1];
        }
    }
    while(!info->left && info->width > total++) fprintf(s, "%lc", (wint_t)info->pad);
    if(info->alt)
        while(digits) {
            fputc(digits-- > len ? '0' : *--p, s);
            if(digits && !(digits % 4) && info->group) fputc('.', s);
        }
    else if(group) {
        if(*g) {
            while(digits > arr) fputc(digits-- > len ? '0' : *--p, s);
        } else {
            int j = (digits - arr) % g[-1];
            if(!j) j = g[-1];
 
            while(j--) fputc(digits-- > len ? '0' : *--p, s);
            while(digits > arr) {
                fputs(loc->thousands_sep, s);
                for(j = 0; j < g[-1]; ++j) fputc(digits-- > len ? '0' : *--p, s);
            }
        }
 
        while(digits) {
            int i = *--g;
            fputs(loc->thousands_sep, s);
            while(i--) fputc(digits-- > len ? '0' : *--p, s);
        }
 
    } else while(digits) fputc(digits-- > len ? '0' : *--p, s);
 
    while(info->left && info->width > total++) fputc(' ', s);
    return total - 1;
}
 
static int printf_binary_arginfo(const struct printf_info *info, size_t n, int *types, int *sizes)
{
    if(n < 1) return -1;
    (void)sizes;
 
    types[0] = info->is_long_double ? PA_INT | PA_FLAG_LONG_LONG :
               info->is_long        ? PA_INT | PA_FLAG_LONG      :
               info->is_char        ? PA_CHAR                    :
               info->is_short       ? PA_INT | PA_FLAG_SHORT     :
                                      PA_INT                     ;
    return 1;
}
 
__attribute__ (( constructor )) static void printf_binary_register(void)
{
    setlocale(LC_ALL, "");
    register_printf_specifier('b', printf_binary_handler, printf_binary_arginfo);
}
 
#endif
 
int main(void)
{
    const char *const fmt[] = {
        "%-#'0b","%-#'b","%-#0b","%-#b","%-'0b","%-'b","%-0b","%-b","%#'0b","%#'b","%#0b","%#b","%'0b","%'b","%0b","%b",
        "%-#'0.0b","%-#'.0b","%-#0.0b","%-#.0b","%-'0.0b","%-'.0b","%-0.0b","%-.0b","%#'0.0b","%#'.0b","%#0.0b","%#.0b","%'0.0b","%'.0b","%0.0b","%.0b",
        "%-#'0.3b","%-#'.3b","%-#0.3b","%-#.3b","%-'0.3b","%-'.3b","%-0.3b","%-.3b","%#'0.3b","%#'.3b","%#0.3b","%#.3b","%'0.3b","%'.3b","%0.3b","%.3b",
        "%-#'0.5b","%-#'.5b","%-#0.5b","%-#.5b","%-'0.5b","%-'.5b","%-0.5b","%-.5b","%#'0.5b","%#'.5b","%#0.5b","%#.5b","%'0.5b","%'.5b","%0.5b","%.5b",
        "%-#'07b","%-#'7b","%-#07b","%-#7b","%-'07b","%-'7b","%-07b","%-7b","%#'07b","%#'7b","%#07b","%#7b","%'07b","%'7b","%07b","%7b",
        "%-#'07.0b","%-#'7.0b","%-#07.0b","%-#7.0b","%-'07.0b","%-'7.0b","%-07.0b","%-7.0b","%#'07.0b","%#'7.0b","%#07.0b","%#7.0b","%'07.0b","%'7.0b","%07.0b","%7.0b",
        "%-#'07.3b","%-#'7.3b","%-#07.3b","%-#7.3b","%-'07.3b","%-'7.3b","%-07.3b","%-7.3b","%#'07.3b","%#'7.3b","%#07.3b","%#7.3b","%'07.3b","%'7.3b","%07.3b","%7.3b",
        "%-#'07.5b","%-#'7.5b","%-#07.5b","%-#7.5b","%-'07.5b","%-'7.5b","%-07.5b","%-7.5b","%#'07.5b","%#'7.5b","%#07.5b","%#7.5b","%'07.5b","%'7.5b","%07.5b","%7.5b",
        NULL
    };
    int i, x;
    const char *const *p;
    for(p = fmt; *p; ++p) {
        printf("fmt: %s\n", *p);
        for(i = 0; i < 300; ++i) {
            printf("'"); x = printf(*p, i); printf("' %d", x);
            puts("");
        }
    }
    return 0;
}
