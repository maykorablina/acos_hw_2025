#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <interval>\n", argv[0]);
        return 1;
    }

    int interval = atoi(argv[1]);
    if (interval <= 0) {
        fprintf(stderr, "Invalid interval: %s\n", argv[1]);
        return 1;
    }

    int counter = 0;
    pid_t pid = getpid();

    while (1) {
        printf("%d: %d\n", pid, counter++);
        sleep(interval);
    }

    return 0;
}
