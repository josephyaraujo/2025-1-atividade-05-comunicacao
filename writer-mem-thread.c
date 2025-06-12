#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

void* writer_thread(void* arg) {
    SharedData* data = (SharedData*)arg;

    pthread_mutex_lock(&data->mutex);
    printf("[WRITER] Thread escritora iniciada. Digite uma mensagem:\n");

    // Lê mensagem do usuário
    fgets(data->message, SHM_SIZE, stdin);
    data->ready = 1;

    printf("[WRITER] Mensagem enviada: %s", data->message);
    
    // Avisa ao leitor que a mensagem está pronta
    pthread_cond_signal(&data->cond);
    pthread_mutex_unlock(&data->mutex);

    return NULL;
}

int main() {
    // Cria/abre memória compartilhada
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(fd, sizeof(SharedData));
    SharedData* data = mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    // Inicializa mutex e condição (compartilhados entre threads)
    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&data->mutex, &mutex_attr);

    pthread_condattr_t cond_attr;
    pthread_condattr_init(&cond_attr);
    pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(&data->cond, &cond_attr);

    data->ready = 0;

    // Cria thread escritora
    pthread_t writer;
    pthread_create(&writer, NULL, writer_thread, data);

    // Espera a thread terminar
    pthread_join(writer, NULL);

    // Libera recursos
    munmap(data, sizeof(SharedData));
    close(fd);
    shm_unlink(SHM_NAME);

    return 0;
}