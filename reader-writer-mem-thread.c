#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <pthread.h>

#define SHM_SIZE 1024
#define SHM_KEY 1234

typedef struct {
    char* shm;
    int shmid;
    pthread_mutex_t mutex;
    pthread_cond_t cond_writer;
    pthread_cond_t cond_reader;
    int data_ready;
    int data_processed;
} shared_data;

void* writer_thread(void* arg) {
    shared_data* sd = (shared_data*)arg;
    
    pthread_mutex_lock(&sd->mutex);
    
    // Escreve na memória compartilhada
    strcpy(sd->shm, "Olá, Memória Compartilhada com Threads!");
    sd->data_ready = 1;
    
    printf("Escritor: Mensagem escrita na memória compartilhada.\n");
    pthread_cond_signal(&sd->cond_reader);
    
    // Espera confirmação do leitor
    while (!sd->data_processed) {
        pthread_cond_wait(&sd->cond_writer, &sd->mutex);
    }
    
    pthread_mutex_unlock(&sd->mutex);
    
    printf("Escritor: Finalizado.\n");
    return NULL;
}

void* reader_thread(void* arg) {
    shared_data* sd = (shared_data*)arg;
    
    pthread_mutex_lock(&sd->mutex);
    
    // Espera dados ficarem prontos
    while (!sd->data_ready) {
        pthread_cond_wait(&sd->cond_reader, &sd->mutex);
    }
    
    // Lê a mensagem
    printf("Leitor: Mensagem lida: \"%s\"\n", sd->shm);
    
    // Sinaliza processamento
    sd->data_processed = 1;
    pthread_cond_signal(&sd->cond_writer);
    
    pthread_mutex_unlock(&sd->mutex);
    
    printf("Leitor: Finalizado.\n");
    return NULL;
}

int main() {
    pthread_t writer, reader;
    shared_data sd;
    
    // Inicializa mutex e variáveis de condição
    pthread_mutex_init(&sd.mutex, NULL);
    pthread_cond_init(&sd.cond_writer, NULL);
    pthread_cond_init(&sd.cond_reader, NULL);
    sd.data_ready = 0;
    sd.data_processed = 0;
    
    // Cria memória compartilhada
    sd.shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (sd.shmid == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    
    sd.shm = shmat(sd.shmid, NULL, 0);
    if (sd.shm == (char*)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    
    // Cria threads
    pthread_create(&writer, NULL, writer_thread, &sd);
    pthread_create(&reader, NULL, reader_thread, &sd);
    
    // Espera threads terminarem
    pthread_join(writer, NULL);
    pthread_join(reader, NULL);
    
    // Limpeza
    shmdt(sd.shm);
    shmctl(sd.shmid, IPC_RMID, NULL);
    pthread_mutex_destroy(&sd.mutex);
    pthread_cond_destroy(&sd.cond_writer);
    pthread_cond_destroy(&sd.cond_reader);
    
    return EXIT_SUCCESS;
}