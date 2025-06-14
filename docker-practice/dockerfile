# Define a imagem base (Fedora)
FROM fedora:latest

# Atualiza os pacotes e instala utilitários básicos
RUN dnf -y update && \
    dnf -y install findutils && \
    dnf -y install fish && \
    dnf -y install gcc && \
    dnf clean all

# Cria um diretório para a aplicação
RUN mkdir -p /app

# Define o diretório de trabalho padrão
WORKDIR /app

# Comando padrão ao iniciar o container
CMD ["fish"]