/* main's arguments one after another in memory, each with its 0, as the
   system's start-up has them: a program that retitles its process writes a
   title over the space from argv[0] to the end of the last argument. */
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    int contiguous = 1;
    for (int i = 1; i < argc; i++)
        if (argv[i] != argv[i - 1] + strlen(argv[i - 1]) + 1) contiguous = 0;
    size_t cap = (size_t)(argv[argc - 1] + strlen(argv[argc - 1]) + 1 - argv[0]);
    char saved[256];
    snprintf(saved, sizeof saved, "%s", argv[2]);
    memset(argv[0], 0, cap);
    memcpy(argv[0], "retitled", cap > 8 ? 8 : cap - 1);
    printf("%d %d %s %s\n", argc, contiguous, saved, argv[0]);
    return argv[argc] != NULL;
}
