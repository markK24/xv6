#include "kernel/types.h"
#include "user/user.h"

void sieve(int fd_parent);

int main(int argc, char *argv[]) {
    int fd[2];
    if (pipe(fd) < 0) {
        fprintf(2, "primes: pipe failed\n");
        exit(1);
    }
    int child = fork();
    if (child < 0) {
        fprintf(2, "primes: fork failed\n");
        exit(2);
    }
    if (child == 0) {
        // Child
        if (close(fd[1]) < 0) {
            fprintf(2, "primes (child): close failed\n");
            exit(3);
        }
        sieve(fd[0]);

        // Unreachable
        exit(4);
    }
    // Parent
    if (close(fd[0]) < 0) {
        fprintf(2, "primes (parent): close failed\n");
        exit(3);
    }

    for (int i = 2; i <= 35; i++) {
        if (write(fd[1], &i, sizeof(i)) != sizeof(i)) {
            fprintf(2, "primes: write failed\n");
            exit(4);
        }
    }
    if (close(fd[1]) < 0) {
        fprintf(2, "primes: close failed\n");
        exit(3);
    }
    while (wait(0) >= 0);
    exit(0);
}

__attribute__((noreturn))
void sieve(int fd_parent) {
    int first;

    if (read(fd_parent, &first, sizeof(first)) != sizeof(first)) {
        fprintf(2, "primes: read failed\n");
        exit(1);
    }

    fprintf(1, "prime %d\n", first);

    int fd_child = -1;

    while (1) {
        int i;

        int res = read(fd_parent, &i, sizeof(i));
        if (res < 0) {
            fprintf(2, "primes: read failed\n");
            exit(1);
        }
        if (res == 0) {
            if (fd_child != -1) {
                if (close(fd_child) < 0) {
                    fprintf(2, "primes: close failed\n");
                    exit(2);
                }
            }
            while (wait(0) >= 0);
            exit(0);
        }

        if (i % first != 0) {
            if (fd_child == -1) {
                int fd[2];
                if (pipe(fd) < 0) {
                    fprintf(2, "primes: pipe failed\n");
                    exit(2);
                }

                int pid = fork();
                if (pid < 0) {
                    fprintf(2, "primes: fork failed\n");
                    exit(3);
                }
                if (pid == 0) {
                    // Child
                    if (close(fd[1]) < 0) {
                        fprintf(2, "primes: close failed\n");
                        exit(4);
                    }
                    sieve(fd[0]);

                    // Unreachable
                    exit(5);
                }
                // Parent
                if (close(fd[0]) < 0) {
                    fprintf(2, "primes: close failed\n");
                    exit(4);
                }
                fd_child = fd[1];
            }

            if (write(fd_child, &i, sizeof(i)) != sizeof(i)) {
                fprintf(2, "primes: write failed\n");
                exit(3);
            }
        }
    }
    // Unreachable
    exit(5);
}