#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

/* get file name from path */
char *fmtname(char *path)
{
    char *p;
    for (p = path + strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;
    return p;
}

int find(char *path, const char *fname)
{
    char buf[512], *p; // file path
    int fd;
    int fnum = 0;     // file fname num
    struct dirent de; // file name and its length
    struct stat st;   // file/directory info

    if ((fd = open(path, 0)) < 0)
    {
        fprintf(2, "find: cannot open %s\n", path);
        return 0;
    }

    if (fstat(fd, &st) < 0)
    {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return 0;
    }

    switch (st.type)
    {
    case T_FILE:
        if (strcmp(fmtname(path), fname) == 0)
        {
            printf("%s\n", path);
            fnum++;
        }
        break;
    case T_DIR:
        if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf))
        {
            printf("find: path too long\n");
            break;
        }
        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';
        while (read(fd, &de, sizeof(de)) == sizeof(de))
        {
            if (de.inum == 0 || strcmp(".", de.name) == 0 || strcmp("..", de.name) == 0)
                continue;

            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            fnum += find(buf, fname);
        }
        break;
    }
    close(fd);
    return fnum;
}

int main(int argc, char *argv[])
{
    int num = 0;
    if (argc < 2)
    {
        fprintf(2, "find:parameter error\n");
        exit(1);
    }
    else if (argc == 2)
    {
        num = find(".", argv[1]);
        if (num == 0)
        {
            fprintf(2, "find: Error or No such file\n");
            exit(1);
        }
    }
    else
    {
        num = find(argv[1], argv[2]);
        if (num == 0)
        {
            fprintf(2, "find: Error or No such file\n");
            exit(1);
        }
    }

    exit(0);
}