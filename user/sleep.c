#include "kernel/types.h"
#include "user/user.h"


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(2, "Usage: sleep <ticks>\n");
        exit(1);
    }

    for (int i = 0; i < strlen(argv[1]); i++) {
        if (argv[1][i] < '0' || argv[1][i] > '9') {
            fprintf(2, "Argument <ticks> should be a number\n");
            exit(1);
        }
    }

    int ticks = atoi(argv[1]);
    sleep(ticks);
    exit(0);
}