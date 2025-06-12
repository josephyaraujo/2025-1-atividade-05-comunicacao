#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#define SHM_NAME "/shared_mem_thread"
#define SHM_SIZE 1024

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    char message[SHM_SIZE];
    int ready;
} SharedData;

void* reader_thread(void* arg) {
    SharedData* data = (SharedData*)arg;

    pthread_mutex_lock(&data->mutex);
    printf("[READER] Thread leitora aguardando mensagem...\n");

    // Espera até que a mensagem esteja pronta
    while (!data->ready) {
        pthread_cond_wait(&data->cond, &data->mutex);
    }

    printf("[READER] Mensagem recebida: %s", data->message);
    pthread_mutex_unlock(&data->mutex);

    return NULL;
}

int main() {
    // Abre memória compartilhada
    int fd = shm_open(SHM_NAME, O_RDWR, 0666);
    SharedData* data = mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    // Cria thread leitora
    pthread_t reader;
    pthread_create(&reader, NULL, reader_thread, data);

    // Espera a thread terminar
    pthread_join(reader, NULL);

    // Libera recursos
    munmap(data, sizeof(SharedData));
    close(fd);

    return 0;
}