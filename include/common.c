#include "common.h"

/** UDP包全局变量 **/
int     send_id = 0;    /** UDP发送id **/
int     receive_id = 0; /** UDP接收id **/
int     id = 1;         /** UDP确认 **/

/**
 * 创建套接字
 * @param family 套接字地址族
 * @param type 套接字类型
 * @param protocol 协议
 * @return 套接字描述符
 */
 //SOCK_STREAM 代表流套接字，通常用于 TCP 连接，
 //SOCK_DGRAM 代表数据报套接字，通常用于 UDP 连接。
int Socket(int family, int type, int protocol){
    int   n;
    //调用系统的 socket 函数来创建一个新的套接字
    if ((n = socket(family, type, protocol)) < 0){
        perror("socket error");
        exit(0);
    }
    //返回套接字的文件描述符
    return n;
}

/**
 * 绑定套接字
 * @param fd 套接字描述符
 * @param sa 套接字地址
 * @param salen 地址长度
 */
void Bind(int fd, const struct sockaddr * sa, socklen_t salen){
    if (bind(fd, sa, salen) < 0){
        perror("bind error");
        exit(1);
    }
}

/**
 * 监听套接字
 * @param fd 套接字描述符
 * @param backlog 监听队列的最大长度
 */
void Listen(int fd, int backlog){
    char     *ptr;
    //LISTENQ 666
    //检查环境变量
    if ((ptr = getenv("LISTENQ")) != NULL)
        backlog = atoi(ptr);
    //调用系统的 listen 函数来监听套接字。
    //backlog 参数指定了内核为相应套接字排队的最大连接个数
    if (listen(fd, backlog) < 0){
        perror("listen error");
        exit(2);
    }
}

/**
 * 接受连接
 * @param fd 套接字描述符
 * @param sa 套接字地址
 * @param salenptr 地址长度指针
 * @return 客户端套接字描述符
 */
int Accept(int fd, struct sockaddr * sa, socklen_t * salenptr){
    int     n;
    //调用系统的 accept 函数来接受客户端的连接请求
    if ((n = accept(fd, sa, salenptr)) < 0){
        perror("Accept Error!");
        exit(3);
    }
    //如果接受连接成功，accept 函数会返回一个新的套接字描述符，这个描述符代表了与客户端的连接。
    //Accept 函数最后会返回这个新的套接字描述符。
    return (n);
}

/**
 * 连接服务器
 * @param fd 套接字描述符
 * @param sa 套接字地址
 * @param salen 地址长度
 */
void Connect(int fd, const struct sockaddr * sa, socklen_t salen){
    if (connect(fd, sa, salen) < 0){
        perror("Connect Error");
        exit(4);
    }
}

/**
 * 客户端初始化
 * @param ip_p 服务器IP和端口
 * @param mode 通信模式
 * @return 套接字描述符
 */
int Client_init(struct ip_port *ip_p, int mode){
    //存储套接字描述符
    int   sockfd = -1;
    
    if (mode == TCP) {
        char    ip[12] = {'\0'};
        int     port = ip_p->port;
        strcpy(ip,ip_p->ip);

        sockfd = Socket(AF_INET, SOCK_STREAM, 0);   

        struct sockaddr_in servaddr;
        //bzero 函数将 servaddr 结构体的所有字段初始化为 0
        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port   = htons(port);
        //AF_INET 表示 IPv4 地址。
        //inet_pton 函数会将转换后的 IP 地址存储在这个结构体中。
        inet_pton(AF_INET, ip, &servaddr.sin_addr);
        Connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    }
    else if (mode == UDP) 
        sockfd = Socket(AF_INET, SOCK_DGRAM, 0);
    return sockfd;
}

/**
 * 服务器初始化
 * @param mode 通信模式
 * @return 套接字描述符
 */
int Server_init(int mode){
    int     listen_fd = -1;
    int     port = PORT;
    
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    
    if (mode == TCP) {
        listen_fd = Socket(AF_INET, SOCK_STREAM, 0);
    }
    else if (mode == UDP) {
        listen_fd = Socket(AF_INET, SOCK_DGRAM, 0);
        port = UDP_PORT;
    }   
    
    inet_pton(AF_INET, IP, &server_addr.sin_addr);
    server_addr.sin_port = htons(port);
    

    Bind(listen_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if (mode == TCP) 
        Listen(listen_fd, LISTENQ);
    return listen_fd;
}


/**
 * 打开文件
 * @param filename 文件名
 * @return 文件描述符
 */
int Open(char * filename){
    //存储文件描述符
    //文件描述符是通过 open 函数打开文件时返回的。
    int     fd  = 0;

    if ((fd = open(filename, O_RDWR)) == -1){
        printf("open error\n");
        return -1;
    }

    return fd;
}

/**
 * 写入文件
 * @param fd 文件描述符
 * @param vptr 写入内容
 * @param n 写入长度
 * @return 写入长度
 */
int Writen(int fd, const void * vptr, const int n){
    int      nleft;    //需要写入的数据的长度
    int      nwritten; //nwritten 用于存储每次调用 write 函数实际写入的数据的长度
    char     *ptr;     //指向的是还需要写入的数据的首地址

    ptr = (char *)vptr;
    nleft = n;

    while (nleft > 0){
        if ((nwritten = write(fd, ptr, nleft)) < 0){
            if (nwritten == n)
                return - 1;
            else 
                break;
        }
        else if (nwritten == 0){
            break;
        }

        nleft -= nwritten;
        ptr += nwritten;
    }
    //等于n说明全部写入成功,返回写入长度
    return (n - nleft);
}


/**
 * 读取文件
 * @param fd 文件描述符
 * @param vptr 读取内容
 * @param n 读取长度
 * @return 读取长度
 */
int Readn(int fd, const void * vptr, const int n){
    int   nleft;   //需要读取的数据的长度
    int   nread;   //nread 用于存储每次调用 read 函数实际读取的数据的长度

    nleft = n;
    char *ptr = (char*)vptr;

    while (nleft > 0){
        if ((nread = read(fd, ptr, nleft)) < 0){
            if (nleft == n)
                return (-1);
            else 
                break;
        }
        else if (nread == 0){
            break;
        }

        nleft -= nread;
        ptr += nread;
    }

    return (n - nleft);
}


/**
 * 创建文件
 * @param filename 文件名
 * @param size 文件大小
 * @return 0
 */
int createfile(char * filename, int size){
    int   fd = open(filename, O_RDWR | O_CREAT);
    //模式被设置为 O_RDWR | O_CREAT
    //表示以读写方式打开文件，如果文件不存在则创建它。
    fchmod(fd, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    //函数调用 fchmod 函数来设置文件的权限.
    //S_IRUSR 表示用户拥有读权限，S_IWUSR 表示用户拥有写权限，S_IRGRP 表示组拥有读权限，S_IROTH 表示其他人拥有读权限。
    lseek(fd, size - 1, SEEK_SET);
    //函数调用 lseek 函数来移动文件的读写位置。
    //size - 1，表示移动到文件的末尾。
    write(fd, "", 1);
    //函数调用 write 函数来写入一个空字符到文件的末尾
    close(fd);
    return 0;
}


/**
 * 获取文件大小
 * @param path 文件路径
 * @return 文件大小
 */
size_t get_filesize(char *path){
    int fd = Open(path);
    //存储文件的状态信息。
    struct stat st;
    //调用 fstat 函数来获取文件的状态信息
    fstat(fd, &st);
    close(fd);
    return st.st_size;
}


/**
 * 计算文件MD5值
 * @param filename 文件名
 * @param md5 MD5值
 * @return MD5值
 */
char * Md5(char * filename, char *md5){
    int  fd = Open(filename);
    //动态分配内存，存储 MD5 值的数组
    unsigned char *md = (unsigned char *) malloc(16*sizeof(char));
    if (md == NULL || md5==NULL){
        perror("malloc md5");
        exit (-1);
    }

    char tmp[3] = {'\0'};

    size_t          file_size = get_filesize(filename);
    //调用 mmap 函数来映射文件到内存。
    unsigned char   *mbegin  = (unsigned char *)mmap(NULL, file_size, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
    //函数调用 MD5 函数来计算文件的 MD5 值
    md = MD5(mbegin, file_size, md);

    for(int i = 0; i < 16;i++){
        sprintf(tmp, "%02X", md[i]);//X 16进制大写
        strcat(md5, tmp);//追加到md5
    }
    //调用 munmap 函数来取消文件的内存映射
    munmap(mbegin,file_size);
    //释放动态分配的内存
    free(md);
    md=NULL;
    close(fd);
    return md5;
}


/**
 * 获取命令类型
 * @param str 命令字符串
 * @return 命令类型
 */
int get_cmd(char *str){
    if (str == NULL || *str == '\0')
        return EMPTY;
    
    if (strcmp(str, "q") == 0 || strcmp(str, "quit") == 0 || strcmp(str, "exit") == 0)
        return EXIT;

    if(strcmp(str, "ls") == 0 || strcmp(str, "list") == 0 || strcmp(str, "dir") == 0)
        return LS;

    if (strcmp(str, "get")  == 0)
        return GET;
        
    if (strcmp(str, "put")  == 0)
        return PUT;
        
    if (strcmp(str, "t") == 0 || strcmp(str, "tcp") == 0)
        return TCP;

    if (strcmp(str, "u") == 0 || strcmp(str, "udp") == 0)
        return UDP;

    return UNKNOWN;

}


/**
 * UDP发送
 * @param fd 套接字描述符
 * @param seek 发送内容
 * @param left 剩余长度
 * @param addr 套接字地址
 * @return 发送长度
 */
int send_by_udp(int fd, char *seek, int left, struct sockaddr *addr){
    struct PackInfo        pack_info;
    struct RecvPack        data;
    bzero(&pack_info, sizeof(pack_info));
    bzero(&data, sizeof(data));
    socklen_t   serv_len = sizeof(struct sockaddr_in);
    //确保写入的数据既不会超过剩余要发送的数据的长度，也不会超过缓冲区的大小。
    int         buf_size = MIN(left, BUFFER_SIZE);
    int n=-1;
    int res = 0; //成功发送出去的字节数
    memcpy(data.buf, seek, buf_size);//先拷贝进来

    if (receive_id == send_id) {
        ++send_id;
        data.head.id        = send_id; /* 发送id放进包头,用于标记顺序 */
        data.head.buf_size  = buf_size; /* 记录数据长度 */
        printf("#1 Pack_id: %d\n",send_id);
        n=sendto(fd, (char *) &data, sizeof(data), 0, (struct sockaddr *) addr, serv_len);
        if (n > 0){
            /* 接收确认消息 */
            printf("#2 Recv Ack..\n");
            if (Readable_timeo(fd, 10)==0){
                printf("!! Timeout !!\n");
                return -1;
            }  
            recvfrom(fd, (char *) &pack_info, sizeof(pack_info), 0, (struct sockaddr *) addr, &serv_len);
            receive_id  = pack_info.id;
            res = buf_size;
        }
        else 
            return 0;
    }
    else {
        /* 如果接收的id和发送的id不相同,重新发送 */
        printf("!!!!#1 Pack_id: %d\n",send_id);
        if (sendto(fd, (char *) &data, sizeof(data), 0, (struct sockaddr *) addr, serv_len) < 0)
            perror("Send File Failed:");
    
        /* 接收确认消息 */
        printf("!!!!#2 Recv Ack..\n");
        if (Readable_timeo(fd, 10)==0){
            printf("!! Timeout !!\n");
            return -1;
        } 
        recvfrom(fd, (char *) &pack_info, sizeof(pack_info), 0, (struct sockaddr *) addr, &serv_len);
        receive_id = pack_info.id;
    }

    return res;

}


/**
 * UDP接收
 * @param fd 套接字描述符
 * @param seek 接收内容
 * @param addr 套接字地址
 * @return 接收长度
 */
int recv_by_udp(int fd, char *seek, struct sockaddr *addr){
    struct PackInfo        pack_info;
    struct RecvPack        data;
    bzero(&pack_info, sizeof(pack_info));
    bzero(&data, sizeof(data));
    socklen_t   clilen= sizeof(struct sockaddr_in);
    int res=0;//本次成功写入的字节数
    //printf("#1 Pack_id: %d\n", id);
    if (Readable_timeo(fd, 10)==0){
        printf("!! Timeout !!\n");
        return -1;
    } 
    int n = recvfrom(fd, (char *) &data, sizeof(data), 0, (struct sockaddr *) addr, &clilen);

    if (n > 0) {
        if (data.head.id == id){
            pack_info.id = data.head.id;
            pack_info.buf_size  = data.head.buf_size;
            ++id;
            /** 发送数据包确认信息 **/
            //printf("#2 Send Ack..\n");
            if (sendto(fd, (char *) &pack_info, sizeof(pack_info), 0, (struct sockaddr *) addr, clilen) < 0)
                printf("Send confirm information failed!");
    
            /* 写入文件 */
            memcpy(seek, data.buf, data.head.buf_size); 
            res = data.head.buf_size;
        }
        else if(data.head.id < id){
            /** 如果是重发的包 **/
            pack_info.id        = data.head.id;
            pack_info.buf_size  = data.head.buf_size;
            /* 重发数据包确认信息 */
            printf("!!!!#3 Resend confirm information..\n");
            if (sendto(fd, (char *) &pack_info, sizeof(pack_info), 0, (struct sockaddr *) addr, clilen) < 0)
                printf("Send confirm information failed!");
        }

    }
    return res;
}


/**
 * UDP全局变量复位
 */
void reset_udp_id(){
    send_id = 0;
    receive_id = 0;
    id = 1;
}


/** > 0 if descriptor is readable **/
/**
 * 套接字超时返回函数
 * @param fd 套接字描述符
 * @param sec 超时时间
 * @return 0
 */
int readable_timeo(int fd, int sec){
    fd_set         rset;
    struct timeval tv;

    FD_ZERO(&rset);
    FD_SET(fd, &rset);

    tv.tv_sec = sec;
    tv.tv_usec = 0;

    return(select(fd+1, &rset, NULL, NULL, &tv));
}

/**
 * 套接字超时返回函数
 * @param fd 套接字描述符
 * @param sec 超时时间
 * @return 0
 */
int Readable_timeo(int fd, int sec){
    int    n;

    if ( (n = readable_timeo(fd, sec)) < 0)
        perror("readable_timeo error");
    return(n);
}

/**
 * 进度条
 * @param rate 进度
 */
void progress_bar(int rate){
    /** 输入有效性校正 **/
    if (rate<0 || rate>100)printf("!! Out of range [0-100] !!\n");
    if (rate < 0)rate=0;
    if (rate > 100)rate=100;

    int old=rate;
    //获取控制台长宽，使进度条长度自适应控制台宽度
    //BUG:最大化会多行#
    struct winsize console;
    int len = 50;
    if (ioctl(STDIN_FILENO, TIOCGWINSZ, (char *)&console)!=-1){
        //将 len 的值设置为控制台的列数减 16
        len = console.ws_col-16;
    }
    rate=len*rate/100;
    char bar[len];
    bzero(bar, len);
    const char *state = "|/-\\";

    printf("[");
    for(int i=0; i<len; i++){
        bar[i]=i<rate? '#':'.';
        printf("%c",bar[i]);
    }
    printf("]");
    printf(" [%d%%] [%c]\r",old, state[old%4]);
    //函数使用 fflush 函数刷新输出，
    //使用 ANSI 转义序列 \33[2K\r 清除当前行的内容。
    fflush(stdout);
    printf("\33[2K\r");
}
