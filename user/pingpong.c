#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    int pfc[2]; // pipe for father to child
    int pcf[2]; // pipe for child to father
    char byte = 'M';
    char buf[2];
    if (pipe(pfc) < 0)
    {
        fprintf(2, "pingpong:create pipe pfc failed\n");
        exit(1);
    }
    if (pipe(pcf) < 0)
    {
        fprintf(2, "pingpong:create pipe pcf failed\n");
        exit(1);
    }

    if (fork() == 0)
    {
        write(pcf[1], &byte, 1);
        read(pfc[0], buf, 1);
        if (buf[0] == byte)
            fprintf(1, "%d: received ping\n", getpid());
        else
            printf("receive from father error\n");
    }
    else
    {
        write(pfc[1], &byte, 1);
        read(pcf[0], buf, 1);
        sleep(1);
        if (buf[0] == byte)
            fprintf(1, "%d: received pong\n", getpid());
        else
            printf("receive from child error\n");
    }

    close(pfc[0]);
    close(pfc[1]);
    close(pcf[0]);
    close(pcf[1]);
    wait(0);
    exit(0);
}