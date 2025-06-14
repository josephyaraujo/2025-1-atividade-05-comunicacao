#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#define NUM_OPERATIONS 1000000
#define SHM_SIZE (NUM_OPERATIONS * sizeof(int))

void *writer(void *arg) {
    int *shared_mem = (int *)arg;
    
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        shared_mem[i] = i;
    }
    
    return NULL;
}

void *reader(void *arg) {
    int *shared_mem = (int *)arg;
    int value;
    
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        value = shared_mem[i];
    }
    
    return NULL;
}

int main() {
    pthread_t writer_thread, reader_thread;
    struct timeval start, end;
    double elapsed_time;

    // Create shared memory
    int fd = shm_open("/shared_mem", O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open failed");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(fd, SHM_SIZE) == -1) {
        perror("ftruncate failed");
        exit(EXIT_FAILURE);
    }

    int *shared_mem = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared_mem == MAP_FAILED) {
        perror("mmap failed");
        exit(EXIT_FAILURE);
    }

    gettimeofday(&start, NULL);

    if (pthread_create(&writer_thread, NULL, writer, shared_mem) != 0) {
        perror("Failed to create writer thread");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&reader_thread, NULL, reader, shared_mem) != 0) {
        perror("Failed to create reader thread");
        exit(EXIT_FAILURE);
    }

    pthread_join(writer_thread, NULL);
    pthread_join(reader_thread, NULL);

    gettimeofday(&end, NULL);
    elapsed_time = (end.tv_sec - start.tv_sec) + 
                  (end.tv_usec - start.tv_usec) / 1000000.0;

    printf("Shared memory operations with threads took %.4f seconds\n", elapsed_time);

    // Cleanup
    munmap(shared_mem, SHM_SIZE);
    shm_unlink("/shared_mem");

    return 0;
}