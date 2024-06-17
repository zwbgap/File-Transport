#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1000

int main() {
    int sockfd, connfd;
    struct sockaddr_in servaddr, cliaddr;
    char buffer[BUFFER_SIZE];
    socklen_t len;
    ssize_t bytes_received, total_bytes_received = 0;

    // 创建套接字
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    // 绑定套接字
    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // 监听连接
    if (listen(sockfd, 5) < 0) {
        perror("listen failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    len = sizeof(cliaddr);
    // 接受连接
    if ((connfd = accept(sockfd, (struct sockaddr *)&cliaddr, &len)) < 0) {
        perror("accept failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // //接收数据
    // while ((bytes_received = recv(connfd, buffer, BUFFER_SIZE, 0)) > 0) {
    //     total_bytes_received += bytes_received;
    // }

    // if (bytes_received < 0) {
    //     perror("recv failed");
    // } else {
    //     printf("Received total %zd bytes\n", total_bytes_received);
    // }


    bytes_received = recv(connfd, buffer, BUFFER_SIZE, 0);
    printf("Received total %zd bytes\n", bytes_received);

    close(connfd);
    close(sockfd);
    return 0;
}