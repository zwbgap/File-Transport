#include "server_work.h"  // 包含头文件 "server_work.h"，其中可能包含了一些函数和宏定义

int p_id = 0;  // put 上来的数组 p_files 存储下标
int g_id = 0;  // get 请求时的 g_files 数组下标

struct fileinfo p_files[FILE_MAX];  // 存储 put 上来的文件信息的数组
struct fileinfo g_files[FILE_MAX];  // 存储客户端下载文件信息的数组
struct fileinfo file;  // 临时存放本次交互的文件信息
extern int listenfd;  // 声明外部变量 listenfd，表示监听套接字文件描述符

struct sockaddr_in cliaddr;  // 客户端地址结构体
socklen_t clilen = sizeof(struct sockaddr_in);  // 客户端地址结构体长度

/*
    broadcast server ip and port
*/
void *broad() {
    struct ip_port ip;  // 创建 ip_port 结构体实例
    strcpy(ip.ip, IP);  // 复制 IP 地址到 ip 结构体
    ip.port = PORT;  // 设置端口号

    int ip_len = sizeof(struct ip_port);  // 计算 ip_port 结构体的大小
    char msg[ip_len + 1];  // 创建消息缓冲区
    bzero(msg, ip_len + 1);  // 将消息缓冲区清零
    memcpy(msg, &ip, ip_len);  // 将 ip_port 结构体复制到消息缓冲区

    int brdcFd = Socket(PF_INET, SOCK_DGRAM, 0);  // 创建 UDP 套接字

    int optval = 1;  // 设置套接字选项
    setsockopt(brdcFd, SOL_SOCKET, SO_BROADCAST | SO_REUSEADDR, &optval, sizeof(int));  // 设置广播和地址重用选项
    struct sockaddr_in theirAddr;  // 创建地址结构体
    memset(&theirAddr, 0, sizeof(struct sockaddr_in));  // 清空地址结构体
    theirAddr.sin_family = AF_INET;  // 设置地址族
    theirAddr.sin_addr.s_addr = inet_addr("255.255.255.255");  // 设置广播地址
    theirAddr.sin_port = htons(4001);  // 设置端口号

    for (;;) {  // 无限循环
        int sendBytes;
        if ((sendBytes = sendto(brdcFd, msg, sizeof(msg), 0, (struct sockaddr *)&theirAddr, sizeof(struct sockaddr))) == -1) {  // 发送广播消息
            printf("sendto fail, errno=%d\n", errno);  // 发送失败，打印错误信息
            break;  // 跳出循环
        }
        sleep(3);  // 休眠 3 秒
    }
    close(brdcFd);  // 关闭套接字
    return (void*)0;  // 返回空指针
}

int recv_cmd(struct command *cmd) {
    int connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, (socklen_t *)&clilen);  // 接受客户端连接
    int cmdlen = sizeof(struct command);  // 获取命令结构体的大小
    char buf[cmdlen + 1];  // 创建缓冲区
    bzero(buf, cmdlen + 1);  // 将缓冲区清零
    Readn(connfd, buf, cmdlen);  // 从套接字读取命令
    memcpy(cmd, buf, cmdlen);  // 将缓冲区内容复制到命令结构体
    //close(connfd);  // 关闭连接
    return connfd;
}

void recv_fileinfo(struct command *cmd) {
    printf("############ recv File information ############\n");
    bzero(&file, sizeof(struct fileinfo));  // 清空文件信息结构体
    int connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, (socklen_t *)&clilen);  // 接受客户端连接

    /** 接收文件信息 **/
    char fileinfo_buf[256] = {'\0'};  // 创建文件信息缓冲区
    int fileinfo_len = sizeof(struct fileinfo);  // 获取文件信息结构体的大小
    Readn(connfd, fileinfo_buf, fileinfo_len);  // 从套接字读取文件信息

    memcpy(&file, fileinfo_buf, fileinfo_len);  // 将缓冲区内容复制到文件信息结构体
    printf("file from client:\n");
    printf("\t%s\n\tsize=%dM\n\tmd5=%s\n\tlseek=%d\n", file.filename, file.filesize/M, file.md5, file.pos);  // 打印文件信息

    /** 放进上传数组里面 **/
    if (get_cmd(cmd->cmd) == PUT) {  // 如果是上传命令
        /** 遍历，找到相等的 **/
        for (int i = 0; i < FILE_MAX; i++) {
            if (strcmp(file.filename, p_files[i].filename) == 0) {  // 找到相同的文件名
                if (Open(file.filename) == -1)  // 上传的文件被外部删除
                    break;
                p_id = i;  // 设置文件下标
                file.pos = p_files[i].pos;  // 设置文件偏移位置
                file.used = 1;  // 设置文件已使用标志
                break;
            }
        }
        /** 没找到，判断为新上传的文件 **/
        if (file.used == 0) {
            createfile(file.filename, file.filesize);  // 创建空洞文件
            file.used = 1;  // 设置文件已使用标志
            while (p_files[p_id].used) {
                ++p_id;
                p_id = p_id % FILE_MAX;  // 环绕数组
            }
            memcpy(&p_files[p_id], &file, fileinfo_len);  // 将文件信息复制到上传数组
        }
        printf("Saving: p_id:%d %s %d %s lseek=%d\n", p_id, p_files[p_id].filename, p_files[p_id].filesize, p_files[p_id].md5, p_files[p_id].pos);  // 打印保存信息
    } else {  // 下载请求，在下载数组里搜
        for (int i = 0; i < FILE_MAX; i++) {
            if (strcmp(file.filename, g_files[i].filename) == 0) {  // 找到相同的文件名
                g_id = i;  // 设置文件下标
                memcpy(&file, &g_files[i], fileinfo_len);  // 将下载数组的文件信息复制到临时文件信息结构体
                file.used = 1;  // 设置文件已使用标志
                break;
            }
        }

        /** 没找到找，文件不存在或不完整，拒绝下载 **/
        if (file.used == 0) {
            printf("404 Not Found\n");  // 打印文件未找到信息
        }
    }

    char send_buf[256] = {'\0'};  // 创建发送缓冲区
    memcpy(send_buf, &file, fileinfo_len);  // 将文件信息复制到发送缓冲区
    printf("file to client:\n");
    printf("\t%s\n\tsize=%dM\n\tmd5=%s\n\tlseek=%d\n", file.filename, file.filesize/M, file.md5, file.pos);  // 打印发送的文件信息
    Writen(connfd, send_buf, fileinfo_len);  // 将文件信息发送回客户端，实现断点续传
    printf("########### send back File information ###########\n");
    close(connfd);  // 关闭连接
    return;
}

void recv_block(struct command *cmd) {
    printf("\n############ 接收文件 ############\n");
    int b_tcp = get_cmd(cmd->mode) == UDP ? UDP : TCP;  // 根据命令模式确定使用的协议

    int fd = Open(file.filename);  // 打开文件
    char *begin = (char *)mmap(NULL, file.filesize, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);  // 将文件映射到内存
    close(fd);  // 关闭文件描述符
    char *seek = begin + file.pos;  // 设置文件指针位置

    int n = 0;
    int left = file.filesize - file.pos;  // 计算剩余文件大小
    if (left < 1) {
        printf("秒传！\n");  // 文件已完整，无需传输
        goto over;  // 跳转到结束标签
    }

    if (b_tcp == TCP) {
        printf("======== TCP ========\n");
        int connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, (socklen_t *)&clilen);  // 接受客户端连接
        while (left > 0) {
            n = read(connfd, seek, left);  // 从套接字读取数据
            if (n < 0) {
                printf("连接异常！\n");  // 读取失败
                break;
            }
            file.pos += n;  // 更新文件偏移位置
            p_files[p_id].pos += n;  // 更新上传数组中的文件偏移位置
            left -= n;  // 更新剩余大小
            seek += n;  // 更新文件指针位置
            printf("recv:%d  left:%dM  lseek:%d\n", n, left / M, p_files[p_id].pos);  // 打印接收信息
            progress_bar((100 * ((file.filesize - left) / M)) / (file.filesize / M));  // 更新进度条
        }
        printf("======= TCP OK =======\n");
        close(connfd);  // 关闭连接
    } else {
        printf("======== UDP ========\n");
        bzero(&cliaddr, clilen);  // 清空客户端地址结构体
        int connfd = Server_init(UDP);  // 初始化 UDP 服务器
        while (left > 0) {
            n = recv_by_udp(connfd, seek, (struct sockaddr *)&cliaddr);  // 接收 UDP 数据
            if (n == -1) break;
            file.pos += n;  // 更新文件偏移位置
            p_files[p_id].pos += n;  // 更新上传数组中的文件偏移位置
            left -= n;  // 更新剩余大小
            seek += n;  // 更新文件指针位置
            progress_bar((100 * ((file.filesize - left) / M)) / (file.filesize / M));  // 更新进度条
        }
        printf("======= UDP OK =======\n");
        reset_udp_id();  // 重置 UDP ID
        close(connfd);  // 关闭连接
    }
over:
    munmap((void *)begin, file.filesize);  // 解除文件映射

    /** 效验完整性 **/
    printf("传输完毕，正在校验文件完整性 ..\n");
    char md5[33] = {'\0'};
    Md5(file.filename, md5);  // 计算文件 MD5

    if (strcmp(file.md5, md5) == 0) {
        printf("文件完好，校验完毕！\n");
        printf("已完成上传 100%%\n");
        p_files[p_id].intact = 1;  // 标记文件完整
        p_files[p_id].pos = file.filesize;  // 更新文件偏移位置

        // 防止重复加入下载队列
        int b_already = 0;
        for (int i = 0; i < FILE_MAX; i++) {
            if (strcmp(file.filename, g_files[i].filename) == 0) {
                g_id = i;
                b_already = 1;
                break;
            }
        }
        // 防止重复加入下载队列
        if (b_already == 0) {
            while (g_files[g_id].used == 1) {
                ++g_id;
                g_id = g_id % FILE_MAX;
            }
            memcpy(&g_files[g_id], &p_files[p_id], sizeof(struct fileinfo));  // 将上传完成的文件加入下载队列
            g_files[g_id].pos = 0;  // 初始化偏移位置为 0
        }
        printf("上传结束,加入下载队列 g_files[%d]\n%s %d MD5:%s lseek=%d\n", g_id, g_files[g_id].filename, g_files[g_id].filesize, g_files[g_id].md5, g_files[g_id].pos);
    } else {
        printf("文件缺损！！上传未完成...\n");
        printf("上传进度 %d%%\n", (int)((100 * (p_files[p_id].pos / M)) / (p_files[p_id].filesize / M)));
    }

    printf("############ 接收完毕 ############\n");
    return;
}

void send_file(struct command *cmd) {
    if (file.used == 0) {
        printf("can't find %s !\n", file.filename);
        return;  // 服务器不存在此文件
    }

    int fd = Open(cmd->filename);
    if (fd == -1) {
        printf("can't find %s !\n", file.filename);
        return;
    }
    printf("\n############ 发送文件 ############\n");
    printf("%s size:%d MD5:%s 下载位置:%d\n", file.filename, file.filesize, file.md5, file.pos);
    char *begin = (char *)mmap(NULL, file.filesize, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);  // 将文件映射到内存
    close(fd);  // 关闭文件描述符
    int n = 0;
    int left = file.filesize - file.pos;  // 计算剩余文件大小
    if (left < 1) {
        printf("秒传！\n");  // 文件已完整，无需传输
        goto over;  // 跳转到结束标签
    }
    char *seek = begin + file.pos;  // 设置文件指针位置
    int b_tcp = get_cmd(cmd->mode) == UDP ? UDP : TCP;  // 根据命令模式确定使用的协议

    if (b_tcp == TCP) {
        printf("======== TCP ========\n");
        int connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);  // 接受客户端连接
        while (left > 0) {
            int bytes = MIN(left, M);  // 计算每次发送的字节数
            n = write(connfd, seek, bytes);  // 发送数据
            left -= n;  // 更新剩余大小
            g_files[g_id].pos += n;  // 更新下载数组中的文件偏移位置
            file.pos += n;  // 更新文件偏移位置
            seek += n;  // 更新文件指针位置
            printf("send:%d  left:%dM  lseek:%d\n", n, left / M, g_files[g_id].pos);  // 打印发送信息
            progress_bar((100 * ((file.filesize - left) / M)) / (file.filesize / M));  // 更新进度条
        }
        printf("====== TCP 发送结束 ======\n");
        close(connfd);  // 关闭连接
    } else {
        printf("======== UDP ========\n");
        bzero(&cliaddr, clilen);  // 清空客户端地址结构体
        reset_udp_id();  // 重置 UDP ID
        int connfd = Server_init(UDP);  // 初始化 UDP 服务器

        /** 接收客户端发来的 download 字符，顺便存储客户端地址信息 **/
        char buf[10] = {'\0'};
        socklen_t clilen = sizeof(struct sockaddr_in);
        recvfrom(connfd, buf, 10, 0, (struct sockaddr *)&cliaddr, &clilen);  // 接收客户端地址信息
        printf("%s...\n", buf);

        while (left > 0) {
            n = send_by_udp(connfd, seek, left, (struct sockaddr *)&cliaddr);  // 发送 UDP 数据
            if (n == -1) break;
            file.pos += n;  // 更新文件偏移位置
            g_files[g_id].pos += n;  // 更新下载数组中的文件偏移位置
            left -= n;  // 更新剩余大小
            seek += n;  // 更新文件指针位置
            progress_bar((100 * ((file.filesize - left) / M)) / (file.filesize / M));  // 更新进度条
        }
        printf("====== UDP 发送结束 ======\n");
        close(connfd);  // 关闭连接
    }
over:
    printf("下载进度 %d%%\n", (int)((100 * (g_files[g_id].pos / M)) / (g_files[g_id].filesize / M)));  // 打印下载进度
    munmap((void *)begin, file.filesize);  // 解除文件映射
    printf("############ 发送完毕 ############\n");

    return;
}

/**
 * 获取文件列表
 * @param client_sock_fd 客户端套接字
 * 该函数获取服务器上的文件列表，并发送给客户端。
 */
void send_file_list(int clientfd) {
    struct dirent *entry;
    DIR *dp = opendir(".");

    if (dp == NULL) {
        perror("opendir");
        return;
    }

    char file_list[1024] = {0};
    while ((entry = readdir(dp))) {
        if (entry->d_type == DT_REG) {  // 只获取常规文件
            strcat(file_list, entry->d_name);
            strcat(file_list, "\n");
        }
    }

    closedir(dp);

    // 发送文件列表给客户端
    if (send(clientfd, file_list, strlen(file_list), 0) == -1) {
        perror("send");
    }
}
