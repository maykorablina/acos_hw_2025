// create dir
// mkdir -p 09_IPC
// cd 09_IPC

#include <stdio.h>
#include <unistd.h>

int main() {
    int i;
    for (i = 0;; i++) {
        sleep(1);
        printf("%d\n", i);
    }
    return 0;
}

// компиляция
// gcc endless.c -o endless

// запуск на фоне
// ./endless &

// возобновление 
// bg

//убить процесс
// kill -SIGINT pid
