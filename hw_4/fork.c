#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();
    if (pid == 0) {
        printf("Hello from child\n");
        pid_t pid2 = fork();
        if (pid2 == 0) {
            printf("Hello from grandchild\n");
            exit(0);
        } else {
            wait(NULL);
            exit(0);
        }
    } else {
        wait(NULL);
        printf("Hello fromp arent\n");
    }
    return 0;
}
