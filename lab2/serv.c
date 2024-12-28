#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>

#define SERVER_PORT 8080

volatile sig_atomic_t received_sighup = 0;

void sighup_handler(int signum) {
    received_sighup = 1;
}

int main() {
    struct sigaction sa;
    sigset_t sigmask;
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len;
    fd_set read_fds;
    int max_fd;
    char buffer[1024];
    ssize_t bytes_read;


    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sighup_handler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGHUP, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }


    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("socket");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_socket);
        return 1;
    }

    if (listen(server_socket, 5) == -1) {
        perror("listen");
        close(server_socket);
        return 1;
    }

    sigemptyset(&sigmask);
    sigaddset(&sigmask, SIGHUP);
    if (sigprocmask(SIG_BLOCK, &sigmask, NULL) == -1) {
        perror("sigprocmask");
        close(server_socket);
        return 1;
    }

    printf("Сервер запущен, ожидание соединений...\n");

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(server_socket, &read_fds);
        max_fd = server_socket;

        if (pselect(max_fd + 1, &read_fds, NULL, NULL, NULL, &sigmask) == -1) {
            if (errno == EINTR && received_sighup) {
                printf("Получен сигнал SIGHUP, продолжаем работу.\n");
                received_sighup = 0;
                continue;
            }
            else {
                perror("pselect");
                close(server_socket);
                return 1;
            }
        }

        if (FD_ISSET(server_socket, &read_fds)) {
            client_addr_len = sizeof(client_addr);
            client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
            if (client_socket == -1) {
                perror("accept");
                continue;
            }
            printf("Новое соединение от %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            while ((client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len)) != -1) {
                close(client_socket);
            }
        }

        if (FD_ISSET(client_socket, &read_fds)) {
            bytes_read = read(client_socket, buffer, sizeof(buffer));
            if (bytes_read == -1) {
                perror("read");
                close(client_socket);
                continue;
            }
            else if (bytes_read == 0) {
                printf("Клиент закрыл соединение.\n");
                close(client_socket);
                continue;
            }
            printf("Получено %zd байт данных.\n", bytes_read);
        }
    }

    close(server_socket);
    return 0;
}