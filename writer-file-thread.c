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

void* writer_thread(void* arg) {
    SyncData* sync = (SyncData*)arg;
    char message[256];

    pthread_mutex_lock(&sync->mutex);
    printf("[WRITER] Thread escritora iniciada. Digite uma mensagem:\n");

    // Lê mensagem do usuário
    fgets(message, sizeof(message), stdin);

    // Escreve no arquivo
    int fd = open(FILENAME, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    write(fd, message, strlen(message));
    close(fd);

    printf("[WRITER] Mensagem escrita no arquivo.\n");
    sync->ready = 1; // Sinaliza que o arquivo está pronto

    // Avisa ao leitor que pode prosseguir
    pthread_cond_signal(&sync->cond);
    pthread_mutex_unlock(&sync->mutex);

    return NULL;
}

int main() {
    SyncData sync = {
        .mutex = PTHREAD_MUTEX_INITIALIZER,
        .cond = PTHREAD_COND_INITIALIZER,
        .ready = 0
    };

    // Cria thread escritora
    pthread_t writer;
    pthread_create(&writer, NULL, writer_thread, &sync);

    // Espera a thread terminar
    pthread_join(writer, NULL);

    return 0;
}