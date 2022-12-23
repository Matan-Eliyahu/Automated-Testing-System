#include <fcntl.h>
#include <unistd.h>

void main(int argc, char* argv[])
{
    if (argc != 3)
    {
        _exit(1);
    }

    int fd1 = open(argv[1], O_RDONLY);
    if (fd1 < 0)
    {
        _exit(1);
    }

    int fd2 = open(argv[2], O_RDONLY);
    if (fd2 < 0)
    {
        close(fd1);
        _exit(1);
    }

    char bufferFd1;
    char bufferFd2;

    int countReaded1 = read(fd1, &bufferFd1, 1);
    int countReaded2 = read(fd2, &bufferFd2, 1);

    while (countReaded1 == 1 && countReaded2 == 1)
    {
        if (bufferFd1 != bufferFd2)
        {
            close(fd1);
            close(fd2);
            _exit(1);
        }

        countReaded1 = read(fd1, &bufferFd1, 1);
        countReaded2 = read(fd2, &bufferFd2, 1);
    }

    close(fd1);
    close(fd2);

    if (countReaded1 == 0 && countReaded2 == 0)
    {
        _exit(2);
    }

    _exit(1);
}