#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define DATA_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in servaddr;
    char data[DATA_SIZE];

    // 初始化数据
    memset(data, 'A', DATA_SIZE);

    // 创建套接字
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    // 连接服务器
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("connection failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // 发送数据
    ssize_t bytes_sent = send(sockfd, data, DATA_SIZE, 0);
    if (bytes_sent < 0) {
        perror("send failed");
    } else {
        printf("Sent %zd bytes\n", bytes_sent);
    }

    close(sockfd);
    return 0;
}