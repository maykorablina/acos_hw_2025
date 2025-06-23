#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_THREADS 4

int *array;
int N;
long long sum = 0;
pthread_mutex_t sum_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    int start;
    int end;
} ThreadData;

void *sumArrayPart(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    long long local_sum = 0;

    for (int i = data->start; i < data->end; ++i) {
        local_sum += array[i];
    }

    int s = pthread_mutex_lock(&sum_mutex);
    if (s != 0) {
        perror("pthread_mutex_lock");
        pthread_exit(NULL);
    }

    sum += local_sum;

    s = pthread_mutex_unlock(&sum_mutex);
    if (s != 0) {
        perror("pthread_mutex_unlock");
        pthread_exit(NULL);
    }

    pthread_exit(NULL);
}

int main() {
    printf("Enter array size N (> 16): a");
    if (scanf("%d", &N) != 1 || N < 16) {
        fprintf(stderr, "Bad input,N must be > 16\n");
        return EXIT_FAILURE;
    }

    array = malloc(N * sizeof(int));
    if (array == NULL) {
        perror("malloc");
        return EXIT_FAILURE;
    }

    srand(time(NULL));
    for (int i = 0; i < N; ++i) {
        array[i] = rand() % 100;
    }

    printf("Array contents:\n");
    for (int i = 0; i < N; ++i) {
        printf("%d ", array[i]);
    }
    printf("\n");

    pthread_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];
    int part_size = N / NUM_THREADS;

    for (int i = 0; i < NUM_THREADS; ++i) {
        thread_data[i].start = i * part_size;
        thread_data[i].end = (i == NUM_THREADS - 1) ? N : (i + 1) * part_size;
        int s = pthread_create(&threads[i], NULL, sumArrayPart, &thread_data[i]);
        if (s != 0) {
            perror("pthread_create");
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < NUM_THREADS; ++i) {
        int s = pthread_join(threads[i], NULL);
        if (s != 0) {
            perror("pthread_join");
            return EXIT_FAILURE;
        }
    }

    printf("Sum = %lld\n", sum);

    free(array);
    return EXIT_SUCCESS;
}
