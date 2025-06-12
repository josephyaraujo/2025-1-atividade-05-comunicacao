#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>

#define FILENAME "thread_communication.txt"

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int ready;
} SyncData;

void* reader_thread(void* arg) {
    SyncData* sync = (SyncData*)arg;
    char buffer[256];

    pthread_mutex_lock(&sync->mutex);
    
    // Espera até que o escritor sinalize que o arquivo está pronto
    while (!sync->ready) {
        pthread_cond_wait(&sync->cond, &sync->mutex);
    }

    printf("[READER] Thread leitora iniciada. Lendo arquivo...\n");

    // Lê o arquivo
    int fd = open(FILENAME, O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    close(fd);

    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        printf("[READER] Mensagem recebida: %s", buffer);
    } else {
        printf("[READER] Arquivo vazio ou erro na leitura.\n");
    }

    pthread_mutex_unlock(&sync->mutex);
    return NULL;
}

int main() {
    SyncData sync = {
        .mutex = PTHREAD_MUTEX_INITIALIZER,
        .cond = PTHREAD_COND_INITIALIZER,
        .ready = 0
    };

    // Cria thread leitora
    pthread_t reader;
    pthread_create(&reader, NULL, reader_thread, &sync);

    // Espera a thread terminar
    pthread_join(reader, NULL);

    return 0;
}