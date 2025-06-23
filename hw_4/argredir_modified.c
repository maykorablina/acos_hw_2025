#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "%s in-out file command\n", argv[0]);
        return EXIT_FAILURE;
    }

    int in = open(argv[1], O_RDONLY);
    if (in < 0) {
        perror("open infile");
        return EXIT_FAILURE;
    }

    int out = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (out < 0) {
        perror("open outfile");
        close(in);
        return EXIT_FAILURE;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        close(in);
        close(out);
        return EXIT_FAILURE;
    }

    if (pid == 0) {  // child
        if (dup2(in, STDIN_FILENO) < 0) {
            perror("dup2 infile");
            exit(EXIT_FAILURE);
        }

        if (dup2(out, STDOUT_FILENO) < 0) {
            perror("dup2 outfile");
            exit(EXIT_FAILURE);
        }

        close(in);
        close(out);

        execvp(argv[3], &argv[3]);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else {  // parent
        close(in);
        close(out);

        int status;
        if (waitpid(pid, &status, 0) < 0) {
            perror("waitpid");
            return EXIT_FAILURE;
        }

        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            fprintf(stderr, "Child process didnt terminate normally\n");
            return EXIT_FAILURE;
        }
    }
}
