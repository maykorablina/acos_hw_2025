#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "%s\n", argv[0]);
        exit(1);
    }

    int fd = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open");
        exit(1);
    }

    dup2(fd, STDOUT_FILENO);
    close(fd);

    execlp(argv[1], argv[1], NULL);
    perror("execlp");
    return 1;
}
