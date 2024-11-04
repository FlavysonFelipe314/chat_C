#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

#define PORTA 8080
#define TAMANHO_BUFFER 1024

DWORD WINAPI receber_mensagens(void *arg) {
    int socket = *(int *)arg;
    char buffer[TAMANHO_BUFFER];
    int bytes_recebidos;

    while ((bytes_recebidos = recv(socket, buffer, TAMANHO_BUFFER, 0)) > 0) {
        buffer[bytes_recebidos] = '\0';
    
        printf("\n%s\n", buffer);
    }

    return 0;
}

void exibir_cabecalho() {
    printf("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
    printf("         CENTRAL DE SUPORTE\n");
    printf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
    printf("- /exit: para sair\n\n");
}

int main() {
    WSADATA wsa;
    SOCKET cliente_socket;
    struct sockaddr_in endereco_servidor;
    DWORD threadID;
    char mensagem[TAMANHO_BUFFER];
    char nome[50];

    printf("Inicializando o Winsock...\n");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Erro ao inicializar o Winsock. Código de erro: %d\n", WSAGetLastError());
        return 1;
    }

    cliente_socket = socket(AF_INET, SOCK_STREAM, 0);
    endereco_servidor.sin_family = AF_INET;
    endereco_servidor.sin_addr.s_addr = inet_addr("127.0.0.1");
    endereco_servidor.sin_port = htons(PORTA);

    connect(cliente_socket, (struct sockaddr *)&endereco_servidor, sizeof(endereco_servidor));

    printf("Digite seu nome: ");
    fgets(nome, sizeof(nome), stdin);
    nome[strcspn(nome, "\n")] = '\0';
    send(cliente_socket, nome, strlen(nome), 0);

    CreateThread(NULL, 0, receber_mensagens, &cliente_socket, 0, &threadID);

    exibir_cabecalho();
    printf("Bem-vindo ao chat, %s! Digite suas mensagens:\n", nome);

    while (1) {
        fgets(mensagem, TAMANHO_BUFFER, stdin);
    
        if (strncmp(mensagem, "/exit", 5) == 0) {
            break;
        }
    
        send(cliente_socket, mensagem, strlen(mensagem), 0);
    }

    closesocket(cliente_socket);
    WSACleanup();
    return 0;
}