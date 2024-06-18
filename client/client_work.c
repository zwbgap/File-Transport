#include "client_work.h"  // 包含头文件 "client_work.h"，其中可能包含了一些函数和宏定义

struct fileinfo file;  // 定义一个 fileinfo 结构体的实例，用于存储文件信息

struct sockaddr_in serv_addr;  // 定义一个 sockaddr_in 结构体的实例，用于存储服务器地址信息

// 定义一个菜单函数，参数为指向 ip_port 结构体的指针

void menu(struct ip_port *ip) {
    printf("************************************************************\n");
    printf("*           Welcome to server: %s:%d     *\n", ip->ip, ip->port);
    printf("*                                                          *\n");
    printf("*  1.download files in TCP mode: file -get -t              *\n");
    printf("*  2.download files in UDP mode: file -get -u              *\n");
    printf("*  3.upload files in TCP mode: file -put -t                *\n");
    printf("*  4.upload files in UDP mode: file -put -u                *\n");
    printf("*  5.get server file list: ls                              *\n");  // 新增选项
    printf("*  6.exit: q quit or exit                                  *\n");
    printf("************************************************************\n");
}

void get_server_list(struct ip_port *ip_p) {
    printf("searching available servers...\n");

    // 创建一个UDP套接字
    int sockListen = Socket(AF_INET, SOCK_DGRAM, 0);

    // 设置套接字选项，允许地址重用
    int set = 1;
    setsockopt(sockListen, SOL_SOCKET, SO_REUSEADDR, &set, sizeof(int));

    // 定义接收地址结构体，并初始化
    struct sockaddr_in recvAddr;
    memset(&recvAddr, 0, sizeof(struct sockaddr_in));
    recvAddr.sin_family = AF_INET;
    recvAddr.sin_port = htons(4001);
    recvAddr.sin_addr.s_addr = INADDR_ANY;

    // 绑定套接字到指定的地址和端口
    Bind(sockListen, (struct sockaddr *)&recvAddr, sizeof(struct sockaddr));

    // 定义接收缓冲区和相关变量
    int recvbytes;
    socklen_t addrLen = sizeof(struct sockaddr_in);
    int ip_len = sizeof(struct ip_port);
    struct ip_port servers[16];  // 存储找到的服务器信息
    int indx = 0;  // 服务器计数
    struct ip_port ip;  // 临时存储接收到的服务器信息
    char msg[ip_len + 1];  // 接收消息的缓冲区

    // 设置搜索时间为5秒
    struct timeval start;
    struct timeval end;
    gettimeofday(&start, NULL);  // 获取当前时间

    for (;;) {  // 无限循环接收数据包
recv:
        bzero(&ip, ip_len);  // 清空ip结构体
        bzero(msg, ip_len + 1);  // 清空消息缓冲区

        // 接收数据包
        if ((recvbytes = recvfrom(sockListen, msg, 128, 0, (struct sockaddr *)&recvAddr, &addrLen)) != -1) {
            gettimeofday(&end, NULL);  // 获取当前时间
            int timer = end.tv_sec - start.tv_sec;  // 计算时间差

            // 如果时间超过5秒或者找到的服务器数超过15，退出循环
            if (timer > 5 || indx > 15)
                break;

            memcpy(&ip, msg, recvbytes);  // 将接收到的数据拷贝到ip结构体

            // 检查是否已经存在相同的服务器IP
            for (int i = 0; i < indx; i++) {
                if (strcmp(ip.ip, servers[i].ip) == 0) {
                    goto recv;  // 如果存在相同的IP，丢弃数据包并继续接收
                }
            }

            // 将新的服务器信息添加到服务器列表中
            strcpy(servers[indx].ip, ip.ip);
            servers[indx].port = ip.port;

            indx += 1;  // 增加服务器计数
        } else {
            printf("recvfrom timeout\n");
            break;  // 接收超时，退出循环
        }
    }

    close(sockListen);  // 关闭套接字

    // 打印找到的服务器列表
    printf("============== available servers ==============\n");
    for (int i = 1; i < indx; i++)
        printf("\t[%d]IP:%s  port:%d\n", i, servers[i].ip, servers[i].port);
    printf("===============================================\n");

    // 询问用户选择要连接的服务器
    printf("which one do you want to connect? ");
    int slct;
input:
    scanf("%d", &slct);  // 获取用户输入
    if (slct >= 0 && slct < indx) {
        strcpy(ip_p->ip, servers[slct].ip);  // 保存用户选择的服务器IP
        ip_p->port = servers[slct].port;  // 保存用户选择的服务器端口
        printf("you select: %s : %d\n", ip_p->ip, ip_p->port);
    } else {
        printf("invalid input!retry...\n");
        goto input;  // 输入无效，重新输入
    }
    getchar();  // 吃掉回车符
    return;
}


int split(struct command *cmd, char *line) {
    // 检查输入参数是否为 NULL
    if (cmd == NULL || line == NULL) return -1;

    // 检查命令长度是否超过 256 个字符
    if (strlen(line) > 256) {
        printf("command is too long!\n");
        return -1;
    }

    char *pch;
    // 使用 strtok 函数按空格、逗号和短横线分割字符串
    pch = strtok(line, " -,");
    int index = 0;

    /** 最多只截获三个参数，多余的丢弃 **/
    while (pch != NULL) {
        if (index == 0)
            strcpy(cmd->filename, pch);  // 第一个参数为 filename
        if (index == 1)
            strcpy(cmd->cmd, pch);  // 第二个参数为 cmd
        if (index == 2)
            strcpy(cmd->mode, pch);  // 第三个参数为 mode
        pch = strtok(NULL, " -,");
        index++;
        if (index > 2)
            break;  // 只处理前三个参数，多余的丢弃
    }

    // 确保 cmd 和 mode 的长度不超过 3
    cmd->cmd[3] = '\0';
    cmd->mode[3] = '\0';

    /** 只获取到了一个参数 **/
    int b_exit = strcmp(cmd->filename, "q") == 0 || strcmp(cmd->filename, "quit") == 0 || strcmp(cmd->filename, "exit") == 0;
   
    if (index == 1 && !b_exit) {
        printf("input error!\n");
        return -1;
    }

    // 上传一个不存在的文件
    if (index > 1 && strcmp(cmd->cmd, "put") == 0 && Open(cmd->filename) == -1) {
        printf("Can't find %s!\n", cmd->filename);
        return -1;
    }

    /** 获取到了两个参数 **/
    int b_get = strcmp(cmd->cmd, "get") == 0 || strcmp(cmd->cmd, "put") == 0;
     int b_ls = strcmp(cmd->cmd, "l") == 0;
    if (index == 2 && !b_get && !b_ls) {
        printf("put or get error!\n");
        return -1;
    }

    /** 获取到了三个参数 **/
    int b_mod = strcmp(cmd->mode, "t") == 0 || strcmp(cmd->mode, "tcp") == 0 || strcmp(cmd->mode, "u") == 0 || strcmp(cmd->mode, "udp") == 0;
    if (index == 3) {
        if (!b_get) {
            printf("put or get error!\n");
            return -1;
        }
        if (!b_mod) {
            printf("tcp or udp error!\n");
            return -1;
        }
    }
    return 0;
}

char *input(char *str) {
    // 使用 readline 获取用户输入
    str = readline(RED "cmd" DEFAULT "@" BLUE "ubuntu:" BLACK "~$ " DEFAULT);
    if (str && *str)
        add_history(str);  // 将输入添加到历史记录
    return str;
}

void get_input(struct command *cmd, struct ip_port *ip) {
    // 清空 file 结构体
    bzero(&file, sizeof(struct fileinfo));
    char inputline[1024] = {'\0'};  // 输入行缓冲区
    char *cmdline = NULL;

getcmd:
    bzero(cmd, sizeof(struct command));  // 清空 cmd 结构体
    cmdline = input(inputline);  // 获取用户输入
    int err = split(cmd, cmdline);  // 解析用户输入
    if (err != 0)
        goto getcmd;  // 如果解析错误，重新获取输入

    /** 执行操作，主功能 **/
    exec_cmd(cmd, ip);  // 执行命令
}

void exec_cmd(struct command *cmd, struct ip_port *ip) {
    // 如果命令是退出命令，则退出程序
    if (get_cmd(cmd->filename) == EXIT)
        exit(0);
    if (get_cmd(cmd->filename) == LS){
        get_file_list(cmd, ip);
    }
    /** 以下两步用TCP传送 **/
    send_cmd(cmd, ip);  // 发送命令
    send_fileinfo(cmd, ip);  // 发送文件信息

    switch (get_cmd(cmd->cmd)) {
        /** 下载 **/
        case GET:
            recv_file(cmd, ip);  // 接收文件
            break;

        /** 上传 **/
        case PUT:
            send_block(cmd, ip);  // 发送文件块
            break;

        case UNKNOWN:
            get_input(cmd, ip);  // 获取新的输入命令
            break;

        default:
            get_input(cmd, ip);  // 获取新的输入命令
            break;
    }
}

void send_cmd(struct command *cmd, struct ip_port *ip) {
    usleep(100000);  // 先让服务器进入accept等待客户端状态
    int sock_fd = Client_init(ip, TCP);  // 初始化TCP客户端
    int cmdlen = sizeof(struct command);
    char buf[cmdlen + 1];
    bzero(buf, cmdlen + 1);
    memcpy(buf, cmd, cmdlen);  // 将命令复制到缓冲区
    Writen(sock_fd, buf, cmdlen);  // 发送命令
    close(sock_fd);  // 关闭套接字
}

void send_fileinfo(struct command *cmd, struct ip_port *ip) {
    printf("\n########### 发送文件信息 ###########\n");
    usleep(100000);  // 先让服务器进入accept等待客户端状态
    int sock_fd = Client_init(ip, TCP);  // 初始化TCP客户端
    strcpy(file.filename, cmd->filename);  // 复制文件名

    /** 若是上传，还需读入大小、MD5 **/
    if (get_cmd(cmd->cmd) == PUT) {
        file.filesize = get_filesize(cmd->filename);  // 获取文件大小
        char md5[33] = {'\0'};
        Md5(file.filename, md5);  // 计算文件的MD5值
        strcpy(file.md5, md5);  // 复制MD5值
    }
    file.pos = 0;
    file.intact = 0;
    file.used = 0;

    char send_buf[256] = {'\0'};
    int fileinfo_len = sizeof(struct fileinfo);
    memcpy(send_buf, &file, fileinfo_len);  // 将文件信息复制到缓冲区
    Writen(sock_fd, send_buf, fileinfo_len);  // 发送文件信息

    printf("=== To-->\n\t%s\n\tsize=%dM\n\tmd5=%s\n\tlseek=%d\n", file.filename, file.filesize / M, file.md5, file.pos);

    /** 接受回传信息，返回偏移量 **/
    bzero(send_buf, 256);
    bzero(&file, fileinfo_len);
    Readn(sock_fd, send_buf, fileinfo_len);  // 接收服务器回传的文件信息
    memcpy(&file, send_buf, fileinfo_len);  // 将接收到的信息复制到文件信息结构体
    printf("== Recv-->\n\t%s\n\tsize=%dM\n\tmd5=%s\n\tlseek=%d\n", file.filename, file.filesize / M, file.md5, file.pos);
    printf("############ 发送完毕 ############\n");
    close(sock_fd);  // 关闭套接字
    return;
}


void recv_file(struct command *cmd, struct ip_port *ip) {
    int b_tcp = get_cmd(cmd->mode) == UDP ? UDP : TCP;
    if (file.used == 0) {
        printf("server: 404 Not Found\n");
        get_input(cmd, ip);  // 获取新的输入命令
    }
    printf("\n############ 开始下载 ############\n");
    printf("%s size:%d MD5:%s 下载位置:%d\n", file.filename, file.filesize, file.md5, file.pos);

    /** 创建下载路径 **/
    char path[100] = {'\0'};
    getcwd(path, sizeof(path));  // 获取当前工作目录
    strcat(path, "/download/");

    if (access(path, 0) == -1) {
        printf("mkdir %s\n", path);
        if (mkdir(path, 0777))
            printf("mkdir %s failed!!\n", path);
    }
    strcat(path, file.filename);
    printf("location: %s\n", path);
    if (file.pos == 0)
        createfile(path, file.filesize);  // 创建文件

    /** I/O映射 **/
    int fd = Open(path);  // 打开文件
    char *begin = (char *)mmap(NULL, file.filesize, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);  // 内存映射
    close(fd);
    char *seek = begin + file.pos;

    int n = 0;
    int left = file.filesize - file.pos;
    if (left < 1) {
        printf("秒传！\n");
        goto over;
    }
    if (b_tcp == TCP) {
        printf("======== TCP ========\n");
        usleep(100000);  // 先让服务器进入accept
        int sock_fd = Client_init(ip, TCP);  // 初始化TCP客户端
        while (left > 0) {
            n = read(sock_fd, seek, left);  // 读取数据
            file.pos += n;
            left -= n;
            seek += n;
            printf("recv:%d  left:%dM  lseek:%d\n", n, left / M, file.pos);
            if (file.filesize > M)
                progress_bar((100 * ((file.filesize - left) / M)) / (file.filesize / M));
        }
        printf("======= TCP OK =======\n");
        close(sock_fd);  // 关闭套接字
    } else {
        printf("======== UDP ========\n");
        udp_serv_init(&serv_addr, ip);  // 初始化UDP服务器地址
        int sock_fd = Socket(AF_INET, SOCK_DGRAM, 0);

        char buf[10] = "Download";  // 发送 使服务器先保存客户端地址
        int serv_len = sizeof(struct sockaddr_in);
        sendto(sock_fd, buf, 10, 0, (struct sockaddr *)&serv_addr, serv_len);
        if (Readable_timeo(sock_fd, 8) == 0) {
            printf("连接超时!\n");
            reset_udp_id();
            close(sock_fd);
            munmap((void *)begin, file.filesize);
            get_input(cmd, ip);  // 获取新的输入命令
        }
        while (left > 0) {
            n = recv_by_udp(sock_fd, seek, (struct sockaddr *)&serv_addr);
            if (n == -1) break;
            file.pos += n;
            left -= n;
            seek += n;
            if (file.filesize > M)
                progress_bar((100 * ((file.filesize - left) / M)) / (file.filesize / M));
        }
        printf("======= UDP OK =======\n");
        reset_udp_id();
        close(sock_fd);  // 关闭套接字
    }
over:
    munmap((void *)begin, file.filesize);  // 及时释放内存，Md5函数要申请大块内存
    /** 效验完整性 **/
    printf("下载完毕，正在校验完整性...\n");
    char md5[33] = {'\0'};
    Md5(path, md5);  // 计算下载文件的MD5值

    if (strcmp(file.md5, md5) == 0) {
        printf("文件完好！\n");
        printf("已完成下载 100%%\n");
    } else {
        printf("文件缺损！！下载未完成...\n");
        if (file.filesize > M)
            printf("下载进度 %d%%\n", (int)((100 * (file.pos / M)) / (file.filesize / M)));
    }

    printf("############ 下载结束 ############\n");
    get_input(cmd, ip);  // 获取新的输入命令
}

void send_block(struct command *cmd, struct ip_port *ip) {
    printf("\n############ 开始上传 ############\n");
    int b_tcp = get_cmd(cmd->mode) == UDP ? UDP : TCP;

    int fd = Open(cmd->filename);  // 打开文件
    char *begin = (char *)mmap(NULL, file.filesize, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);  // 内存映射
    close(fd);
    char *seek = begin + file.pos;
    int n = 0;
    int left = file.filesize - file.pos;
    if (left < 1) {
        printf("秒传！\n");
        goto over;
    }
    if (b_tcp == TCP) {
        printf("======== TCP ========!\n");
        usleep(100000);  // 先让服务器进入accept
        int sock_fd = Client_init(ip, TCP);  // 初始化TCP客户端
        while (left > 0) {
            int bytes = MIN(left, M);
            n = write(sock_fd, seek, bytes);  // 每次最多发送一兆
            left -= n;
            seek += n;
            printf("send:%d  left:%dM\n", n, left / M);
            if (file.filesize > M)
                progress_bar((100 * ((file.filesize - left) / M)) / (file.filesize / M));
        }
        printf("======= TCP OK =======!\n");
        close(sock_fd);  // 关闭套接字
    } else {
        printf("======== UDP ========\n");
        reset_udp_id();  // 重置UDP ID
        udp_serv_init(&serv_addr, ip);  // 初始化UDP服务器地址
        int sock_fd = Socket(AF_INET, SOCK_DGRAM, 0);

        while (left > 0) {
            n = send_by_udp(sock_fd, seek, left, (struct sockaddr *)&serv_addr);  // 通过UDP发送数据
            left -= n;
            seek += n;
            if (file.filesize > M)
                progress_bar((100 * ((file.filesize - left) / M)) / (file.filesize / M));
        }
        printf("======= UDP OK =======\n");
        close(sock_fd);  // 关闭套接字
    }
over:
    printf("############ 上传结束 ############\n");
    munmap((void *)begin, file.filesize);  // 解除内存映射
    get_input(cmd, ip);  // 获取新的输入命令
}

void udp_serv_init(struct sockaddr_in *server_addr, struct ip_port *ip) {
    /* 服务端地址 */
    bzero(server_addr, sizeof(struct sockaddr_in));  // 清空地址结构体
    server_addr->sin_family = AF_INET;
    server_addr->sin_addr.s_addr = inet_addr(ip->ip);  // 设置服务器IP地址
    server_addr->sin_port = htons(UDP_PORT);  // 设置服务器端口
}



void get_file_list(struct command *cmd, struct ip_port *ip){
    // 发送 "ls" 命令到服务器
    send_cmd(cmd, ip);  

    // 接收服务器发送的文件列表
    char file_list[1024];  // 假设文件列表不超过1024个字符
    int sock_fd = Client_init(ip, TCP);  // 假设是通过TCP接收
    if (sock_fd < 0) {
        perror("Client_init");
        return;
    }

    bzero(file_list, sizeof(file_list));  // 清空文件列表缓冲区
    if (Readn(sock_fd, file_list, sizeof(file_list) - 1) < 0) {  // 读取文件列表
        perror("Readn");
        close(sock_fd);
        return;
    }

    close(sock_fd);  // 关闭套接字

    // 打印文件列表
    printf("Server file list:\n%s\n", file_list);
    get_input(cmd, ip);  // 继续等待用户输入
}