#include <stdio.h>
#include <stdlib.h>

int main() {
    int N, M;
    scanf("%d", &N);
    scanf("%d", &M);

    int **matrix = (int **)malloc(N * sizeof(int *));
    for (int i = 0; i < N; i++) {
        matrix[i] = (int *)malloc(M * sizeof(int));
    }
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            scanf("%d", &matrix[i][j]);
        }
    }

    int **transpose = (int **)malloc(M * sizeof(int *));

    for (int i = 0; i < M; i++) {
        transpose[i] = (int *)malloc(N * sizeof(int));
    }
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            transpose[j][i] = matrix[i][j];
        }
    }
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            printf("%d ", transpose[i][j]);
        }
        printf("\n");
    }

    for (int i = 0; i < N; i++) {
        free(matrix[i]);
    }
    free(matrix);

    for (int i = 0; i < M; i++) {
        free(transpose[i]);
    }
    free(transpose);
    return 0;
}
