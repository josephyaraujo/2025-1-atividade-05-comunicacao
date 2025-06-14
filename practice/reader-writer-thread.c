#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>

#define FILENAME "comunicacao.txt"
#define TEMPNAME "comunicacao.lida"

// Estrutura para compartilhar dados entre threads
typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond_writer;
    pthread_cond_t cond_reader;
    int file_ready;
    int file_processed;
} thread_data;

// Função da thread escritora
void* writer(void* arg) {
    thread_data* data = (thread_data*)arg;
    FILE* file;
    const char* message = "Olá, comunicação via arquivo com threads!\n";

    pthread_mutex_lock(&data->mutex);
    
    // Escreve no arquivo
    file = fopen(FILENAME, "w");
    if (!file) {
        perror("Escritor: fopen falhou");
        exit(EXIT_FAILURE);
    }
    fwrite(message, 1, strlen(message), file);
    fclose(file);

    printf("Escritor: Mensagem escrita no arquivo.\n");
    
    // Sinaliza que o arquivo está pronto
    data->file_ready = 1;
    pthread_cond_signal(&data->cond_reader);
    
    // Espera o leitor processar
    while (!data->file_processed) {
        pthread_cond_wait(&data->cond_writer, &data->mutex);
    }
    
    pthread_mutex_unlock(&data->mutex);
    printf("Escritor: Finalizado.\n");
    return NULL;
}

// Função da thread leitora
void* reader(void* arg) {
    thread_data* data = (thread_data*)arg;
    FILE* file;
    char buffer[1024];

    pthread_mutex_lock(&data->mutex);
    
    // Espera o arquivo ficar pronto
    while (!data->file_ready) {
        pthread_cond_wait(&data->cond_reader, &data->mutex);
    }

    // Lê o arquivo
    file = fopen(FILENAME, "r");
    if (!file) {
        perror("Leitor: fopen falhou");
        exit(EXIT_FAILURE);
    }
    
    size_t bytes = fread(buffer, 1, sizeof(buffer) - 1, file);
    buffer[bytes] = '\0';
    fclose(file);

    printf("Leitor: Mensagem recebida:\n%s", buffer);
    
    // Renomeia o arquivo (simula processamento)
    if (rename(FILENAME, TEMPNAME) != 0) {
        perror("Leitor: rename falhou");
        exit(EXIT_FAILURE);
    }
    printf("Leitor: Arquivo renomeado para %s\n", TEMPNAME);
    
    // Sinaliza que processou
    data->file_processed = 1;
    pthread_cond_signal(&data->cond_writer);
    
    pthread_mutex_unlock(&data->mutex);
    printf("Leitor: Finalizado.\n");
    return NULL;
}

int main() {
    pthread_t writer_thread, reader_thread;
    thread_data data = {
        .mutex = PTHREAD_MUTEX_INITIALIZER,
        .cond_writer = PTHREAD_COND_INITIALIZER,
        .cond_reader = PTHREAD_COND_INITIALIZER,
        .file_ready = 0,
        .file_processed = 0
    };

    // Remove arquivos residuais de execuções anteriores
    remove(FILENAME);
    remove(TEMPNAME);

    // Cria as threads
    if (pthread_create(&writer_thread, NULL, writer, &data) != 0 ||
        pthread_create(&reader_thread, NULL, reader, &data) != 0) {
        perror("pthread_create falhou");
        exit(EXIT_FAILURE);
    }

    // Espera as threads terminarem
    pthread_join(writer_thread, NULL);
    pthread_join(reader_thread, NULL);

    // Limpeza
    pthread_mutex_destroy(&data.mutex);
    pthread_cond_destroy(&data.cond_writer);
    pthread_cond_destroy(&data.cond_reader);

    // Remove arquivos temporários
    remove(TEMPNAME);

    return EXIT_SUCCESS;
}