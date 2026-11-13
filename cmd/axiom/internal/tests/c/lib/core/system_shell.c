/* system's command reaches the command processor as it is (sh -c on Linux
   and macOS, cmd.exe /s /c "..." on Windows): a quoted file name with a
   space in a redirection works in both, and the status comes back (POSIX's
   encoding, the exit status times 256, or Windows' exit status). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    char line[16] = "";
    FILE *f;
    int r = system("echo ab>\"f g.txt\"");
    printf("ran %d\n", r);
    f = fopen("f g.txt", "r");
    if (f != NULL) {
        fgets(line, sizeof line, f);
        fclose(f);
    }
    printf("[%s]\n", strtok(line, " \n"));
    remove("f g.txt");
    r = system("exit 3");
    printf("status %d\n", r == 3 * 256 || r == 3);
    printf("processor %d\n", system(NULL) != 0);
    return 0;
}
// native-only
// expect: ran 0
// expect: [ab]
// expect: status 1
// expect: processor 1
