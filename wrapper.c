#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>

#ifdef TARGET_VARS
typedef struct
{
    const char* envname;
    const char* template;
} envupdate_t;

#include TARGET_VARS

char* evaluate(const char* template, const char* value)
{
    char* r;
    char* o;
    const char* p;
    size_t l = 0;
    size_t lv = value ? strlen(value) : 0;

    for(p = template; *p; ++p) {
        if(*p == '%')
            l += lv;
        else
            ++l;
    }

    r = malloc(l + 1);
    for(o = r, p = template; *p; ++p) {
        if(*p == '%') {
            if(lv) {
                memcpy(o, value, lv);
                o += lv;
            }
        } else
            *o++ = *p;
    }
    *o = 0;
    return r;
}

char* concat(const char* s1, const char* s2, const char* s3)
{
    size_t l1 = strlen(s1);
    size_t l2 = strlen(s2);
    size_t l3 = strlen(s3);
    char* r = malloc(l1 + l2 + l3 + 1);
    r[0] = 0;
    if(s1) strcat(r, s1);
    if(s2) strcat(r, s2);
    if(s3) strcat(r, s3);
    return r;
}

void update_env(void)
{
    envupdate_t* e;
    for(e = envupdates; e->envname; ++e)
    {
        char* v = evaluate(e->template, getenv(e->envname));
        char* p = concat(e->envname, "=", v);
        putenv(p);
    }
}
#endif

#ifdef __MINGW32__
typedef const char** args_t;
#else
typedef const char** args_t;
#endif

int main(int argc, char** argv)
{
    int i;
    args_t args = malloc(sizeof(char*) * (argc + 1));
    args[0] = TARGET;
    for(i = 1; i < argc; ++i)
        args[i] = argv[i];
    args[argc] = NULL;
#ifdef TARGET_VARS
    update_env();
#endif
    errno = 0;
    int rc = spawnv(_P_WAIT, args[0], args);
    if(rc == -1 && errno)
        fprintf(stderr, "Error: unable to run %s (errno=%d)\n", args[0], errno);
    free(args);
    return rc;
}
