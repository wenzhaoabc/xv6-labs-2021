#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void prime(int input_fd)
{
    int base;
    if (read(input_fd, &base, sizeof(int)) <= 0)
        exit(0);
    printf("prime %d\n", base);

    int p2[2]; // pipe from left to right
    if (pipe(p2) < 0)
    {
        fprintf(2, "primes:create pipe p2 failed\n");
        exit(1);
    }
    if (fork() == 0)
    {
        close(p2[1]);
        prime(p2[0]);
    }
    else
    {
        close(p2[0]);
        int number;
        while (read(input_fd, &number, sizeof(int)))
        {
            if (number % base != 0)
                write(p2[1], &number, sizeof(int));
        }
        close(p2[1]);
    }

    wait(0);
    exit(0);
}

int main(int argc, char *argv[])
{
    int p1[2];
    if (pipe(p1) < 0)
    {
        fprintf(2, "primes:create pipe p1 failed\n");
        exit(1);
    }

    int pid = fork();
    if (pid == 0) // child process
    {
        close(p1[1]);
        prime(p1[0]);
    }
    else if (pid > 0) // father process
    {
        close(p1[0]);
        for (int i = 2; i <= 35; i++)
            write(p1[1], &i, sizeof(int));
        close(p1[1]);
    }
    else
        fprintf(2, "primes:call fork error");

    wait(0);
    exit(0);
}