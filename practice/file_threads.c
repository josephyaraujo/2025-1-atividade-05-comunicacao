#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#define NUM_OPERATIONS 1000000

void *writer(void *arg) {
    FILE *file = fopen("test.txt", "w");
    if (file == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < NUM_OPERATIONS; i++) {
        fprintf(file, "%d\n", i);
    }

    fclose(file);
    return NULL;
}

void *reader(void *arg) {
    FILE *file = fopen("test.txt", "r");
    if (file == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    int value;
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        if (fscanf(file, "%d", &value) != 1) {
            break;
        }
    }

    fclose(file);
    return NULL;
}

int main() {
    pthread_t writer_thread, reader_thread;
    struct timeval start, end;
    double elapsed_time;

    gettimeofday(&start, NULL);

    if (pthread_create(&writer_thread, NULL, writer, NULL) != 0) {
        perror("Failed to create writer thread");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&reader_thread, NULL, reader, NULL) != 0) {
        perror("Failed to create reader thread");
        exit(EXIT_FAILURE);
    }

    pthread_join(writer_thread, NULL);
    pthread_join(reader_thread, NULL);

    gettimeofday(&end, NULL);
    elapsed_time = (end.tv_sec - start.tv_sec) + 
                  (end.tv_usec - start.tv_usec) / 1000000.0;

    printf("File operations with threads took %.4f seconds\n", elapsed_time);

    return 0;
}