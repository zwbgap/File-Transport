#include "server_work.h"

int listenfd;  // 服务器监听套接字

int main() {
    /** 创建广播线程 **/
    pthread_t broadthrd;  // 广播线程
    int err = -1;
    if ((err = pthread_create(&broadthrd, NULL, broad, NULL)) != 0) {
        perror("can't create broad thread");  // 创建线程失败时打印错误信息
        exit(-1);  // 退出程序
    }

    // 初始化服务器，使用TCP协议
    listenfd = Server_init(TCP);

    /** 接收客户端命令行参数 **/
    struct command cmd;  // 用于存储客户端命令的结构体
    while (1) {
        printf("\nWait for task...\n");
        bzero(&cmd, sizeof(struct command));  // 将cmd结构体置零
        int clientfd = recv_cmd(&cmd);  // 接收客户端命令
        printf("cmd: %s -%s -%s\n", cmd.filename, cmd.cmd, cmd.mode); 
        recv_fileinfo(&cmd);  // 接收文件信息
        if(get_cmd(cmd.filename) == LS){
            system("ls");
            send_file_list(clientfd);
            continue;
        }
        // 根据客户端命令执行相应的操作
        switch (get_cmd(cmd.cmd)) {
            case GET:
                /** 客户端下载请求 **/
                send_file(&cmd);  // 发送文件给客户端
                break;
            case PUT:
                /** 客户端上传请求 **/
                recv_block(&cmd);  // 接收客户端上传的文件块
                break;
            case UNKNOWN:
            default:
                printf("cmd error\n");  // 命令错误
                break;
        }
    }

    return 0;
}