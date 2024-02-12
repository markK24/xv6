#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    int ctop[2]; // Child to parent
    int ptoc[2]; // Parent to child
    if (pipe(ctop) < 0 || pipe(ptoc) < 0) {
        fprintf(2, "pingpong: pipe failed\n");
        exit(1);
    }

    int pid = fork();
    if (pid < 0) {
        fprintf(2, "pingpong: fork failed\n");
        exit(2);
    }
    if (pid == 0) {
        // Child
        if (close(ctop[0]) < 0 || close(ptoc[1]) < 0) {
            fprintf(2, "pingpong (child): close failed\n");
            exit(3);
        }

        char data;

        if (read(ptoc[0], &data, 1) != 1) {
            fprintf(2, "pingpong (child): read failed\n");
            exit(4);
        }

        fprintf(1, "%d: received ping\n", getpid());

        if (write(ctop[1], &data, 1) != 1) {
            fprintf(2, "pingpong (child): write failed\n");
            exit(4);
        }

        exit(0);
    }

    // Parent
    if (close(ctop[1]) < 0 || close(ptoc[0]) < 0) {
        fprintf(2, "pingpong (parent): close failed\n");
        exit(3);
    }

    char data = 27;

    if (write(ptoc[1], &data, 1) != 1) {
        fprintf(2, "pingpong (parent): write failed\n");
        exit(4);
    }

    if (read(ctop[0], &data, 1) != 1) {
        fprintf(2, "pingpong (parent): read failed\n");
        exit(4);
    }

    fprintf(1, "%d: received pong\n", getpid());
    exit(0);
}