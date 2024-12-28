#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[1024];
    ssize_t bytes_sent;

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("socket");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(SERVER_PORT);

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(client_socket);
        return 1;
    }

    printf("Подключение к серверу установлено.\n");

    sprintf(buffer, "Hello, world!");
    bytes_sent = write(client_socket, buffer, strlen(buffer));
    if (bytes_sent == -1) {
        perror("write");
        close(client_socket);
        return 1;
    }
    printf("Данные отправлены: %s\n", buffer);

    printf("Отправляем сигнал SIGHUP серверу...\n");
    kill(getpid(), SIGHUP);

    close(client_socket);
    return 0;
}