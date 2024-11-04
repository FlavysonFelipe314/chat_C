#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

#define PORTA 8080
#define MAX_CLIENTES 10
#define TAMANHO_BUFFER 1024

typedef struct {
    int socket;
    char nome[50];
} Cliente;

Cliente clientes[MAX_CLIENTES];
int contador_clientes = 0;
CRITICAL_SECTION criticalSection;

void enviar_mensagem_todos(char *mensagem, int remetente_socket) {
    EnterCriticalSection(&criticalSection);
    for (int i = 0; i < contador_clientes; i++) {
        if (clientes[i].socket != remetente_socket) {
            send(clientes[i].socket, mensagem, strlen(mensagem), 0);
        }
    }
    LeaveCriticalSection(&criticalSection);
}

DWORD WINAPI gerenciar_cliente(void *arg) {
    int cliente_socket = *(int *)arg;
    char buffer[TAMANHO_BUFFER];
    int bytes_recebidos;
    char nome[50];

    // Recebe o nome do cliente
    if ((bytes_recebidos = recv(cliente_socket, nome, sizeof(nome), 0)) <= 0) {
        closesocket(cliente_socket);
        return 0;
    }
    nome[bytes_recebidos] = '\0';

    EnterCriticalSection(&criticalSection);
    strcpy(clientes[contador_clientes].nome, nome);
    clientes[contador_clientes++].socket = cliente_socket;
    LeaveCriticalSection(&criticalSection);

    char mensagem_entrada[TAMANHO_BUFFER];
    snprintf(mensagem_entrada, TAMANHO_BUFFER, "usuario [%s] entrou na sala.\n", nome);
    enviar_mensagem_todos(mensagem_entrada, cliente_socket);
    printf("%s", mensagem_entrada);

    while ((bytes_recebidos = recv(cliente_socket, buffer, TAMANHO_BUFFER, 0)) > 0) {
        buffer[bytes_recebidos] = '\0';
        char mensagem[TAMANHO_BUFFER + 50];
        snprintf(mensagem, sizeof(mensagem), "[%s] enviou: %s", nome, buffer);
        enviar_mensagem_todos(mensagem, cliente_socket);
    }

    EnterCriticalSection(&criticalSection);
    for (int i = 0; i < contador_clientes; i++) {
        if (clientes[i].socket == cliente_socket) {
            snprintf(mensagem_entrada, TAMANHO_BUFFER, "usuario [%s] saiu da sala.\n", clientes[i].nome);
            enviar_mensagem_todos(mensagem_entrada, cliente_socket);
            printf("%s", mensagem_entrada);
            for (int j = i; j < contador_clientes - 1; j++) {
                clientes[j] = clientes[j + 1];
            }
            contador_clientes--;
            break;
        }
    }
    LeaveCriticalSection(&criticalSection);

    closesocket(cliente_socket);
    return 0;
}

int main() {
    WSADATA wsa;
    SOCKET servidor_socket, cliente_socket;
    struct sockaddr_in endereco_servidor, endereco_cliente;
    int tam_endereco_cliente = sizeof(endereco_cliente);
    DWORD threadID;

    printf("Inicializando o Winsock...\n");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Erro ao inicializar o Winsock. Código de erro: %d\n", WSAGetLastError());
        return 1;
    }

    servidor_socket = socket(AF_INET, SOCK_STREAM, 0);
    endereco_servidor.sin_family = AF_INET;
    endereco_servidor.sin_addr.s_addr = INADDR_ANY;
    endereco_servidor.sin_port = htons(PORTA);

    bind(servidor_socket, (struct sockaddr *)&endereco_servidor, sizeof(endereco_servidor));
    listen(servidor_socket, MAX_CLIENTES);

    printf("Servidor de chat iniciado na porta %d\n", PORTA);
    InitializeCriticalSection(&criticalSection);

    while (1) {
        cliente_socket = accept(servidor_socket, (struct sockaddr *)&endereco_cliente, &tam_endereco_cliente);

        CreateThread(NULL, 0, gerenciar_cliente, &cliente_socket, 0, &threadID);
    }

    DeleteCriticalSection(&criticalSection);
    closesocket(servidor_socket);
    WSACleanup();
    return 0;
}