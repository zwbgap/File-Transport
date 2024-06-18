#ifndef __SERVER_WORK_H__
#define __SERVER_WORK_H__

#include "common.h"

/**
 * 广播服务器IP和端口号，每隔3秒广播一次
 * 此函数在一个单独的线程中运行，向局域网内的所有设备广播服务器的IP和端口信息。
 */
void *broad();

/**
 * 接收客户端命令参数
 * @param cmd 客户端命令结构体，包含文件名、命令类型和模式
 * 该函数负责接收并解析客户端发送的命令，存储在 cmd 结构体中。
 */
int recv_cmd(struct command *cmd);

/**
 * 接收文件信息，建立并保存在本地，并把断点信息回传给客户端
 * @param cmd 客户端命令结构体，包含文件名、命令类型和模式
 * 该函数负责接收客户端发送的文件信息，包括文件名、大小、MD5校验码等，
 * 在本地创建相应的文件并回传断点信息给客户端。
 */
void recv_fileinfo(struct command *cmd);

/**
 * 接收文件，校验成功放入下载队列，等待客户端下载
 * @param cmd 客户端命令结构体，包含文件名、命令类型和模式
 * 该函数负责接收客户端发送的文件数据块，进行数据完整性校验，
 * 如果校验成功则将文件放入下载队列中等待客户端下载。
 */
void recv_block(struct command *cmd);

/**
 * 客户端下载请求，发送文件
 * @param cmd 客户端命令结构体，包含文件名、命令类型和模式
 * 该函数处理客户端的下载请求，将服务器上的文件发送给客户端。
 */
void send_file(struct command *cmd);

/**
    * 获取文件列表
    * @param client_sock_fd 客户端套接字
    * 该函数获取服务器上的文件列表，并发送给客户端。
*/
void send_file_list(int clientfd);


#endif
