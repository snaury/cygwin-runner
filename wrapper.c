#include <stdio.h>
#include <process.h>

int main(int argc, char** argv)
{
    int i;
    char** args = malloc(sizeof(char*) * (argc + 1));
    args[0] = TARGET;
    for(i = 1; i < argc; ++i)
        args[i] = argv[i];
    args[argc] = NULL;
    int rc = execv(args[0], args);
    free(args);
    return rc;
}
