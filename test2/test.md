### 大文件传输相关截图
![截图](/img/1.png)

![截图](/img/2.png)

![截图](/img/3.png)

![截图](/img/4.png)


#### 加入异常控制依据，增强程序的鲁棒性 (Robust)。了解如何提高套接字传输的速率，以及如何加强传输的稳定性
为了提高程序的鲁棒性，我们可以加入异常处理和控制措施，以确保在数据传输过程中能够处理各种可能出现的错误和异常情况。同时，为了提高套接字传输的速率和稳定性，可以考虑以下几点：

1. **优化缓冲区大小**：根据网络环境和数据传输的实际情况，调整套接字的发送和接收缓冲区大小。
2. **使用多线程或异步I/O**：利用多线程或异步I/O技术，可以提高数据传输的并发性和效率。
3. **流量控制和拥塞控制**：实现流量控制和拥塞控制机制，避免网络拥塞导致的数据传输速率下降。
4. **数据压缩**：在传输数据前进行压缩，可以减少传输的数据量，提高传输效率。
5. **错误检测和重传机制**：实现错误检测和重传机制，确保数据传输的可靠性。

示例代码

```python
import socket
import threading
import zlib

def handle_client(client_socket):
    try:
        # 接收数据
        data = client_socket.recv(4096)
        if not data:
            return
        
        # 解压缩数据
        decompressed_data = zlib.decompress(data)
        
        # 处理数据（示例）
        print(f"Received data: {decompressed_data}")

        # 发送响应
        response = zlib.compress(b"ACK")
        client_socket.send(response)
    except Exception as e:
        print(f"Error: {e}")
    finally:
        client_socket.close()

def server_program():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind(("0.0.0.0", 9999))
    server_socket.listen(5)
    print("Server listening on port 9999")

    while True:
        client_socket, addr = server_socket.accept()
        print(f"Accepted connection from {addr}")
        client_handler = threading.Thread(target=handle_client, args=(client_socket,))
        client_handler.start()

def client_program():
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect(("127.0.0.1", 9999))
    
    try:
        # 发送数据
        data = zlib.compress(b"Hello, server!")
        client_socket.send(data)

        # 接收响应
        response = client_socket.recv(4096)
        decompressed_response = zlib.decompress(response)
        print(f"Received response: {decompressed_response}")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        client_socket.close()

if __name__ == "__main__":
    choice = input("Run as server (s) or client (c)? ").strip().lower()
    if choice == 's':
        server_program()
    elif choice == 'c':
        client_program()
    else:
        print("Invalid choice")
```

### 解释
1. **异常处理**：在数据接收和发送过程中，加入异常处理代码，确保程序能够处理各种可能的错误和异常情况。
2. **数据压缩**：在数据传输前进行压缩，减少传输的数据量，提高传输效率。
3. **多线程处理**：服务器端使用多线程处理多个客户端连接，提高并发性和效率。
4. **缓冲区大小**：在接收和发送数据时，使用合适的缓冲区大小（例如4096字节），以提高传输效率。

通过这些措施，可以提高套接字传输的速率和稳定性，并增强程序的鲁棒性。


## 思考题解答

#### 1. 套接字有基于阻塞和非阻塞的工作方式，试问你编写的程序是基于阻塞还是非阻塞的？各有什么优缺点？

**解答**：
我编写的程序是基于阻塞的工作方式。

**阻塞模式**：
- **优点**：
  - 简单易用，代码逻辑清晰。
  - 适合处理简单的客户端-服务器通信。
- **缺点**：
  - 如果一个操作（如 `recv`）阻塞了，整个线程会等待，无法处理其他任务。
  - 在高并发场景下，可能会导致资源浪费和性能瓶颈。

**非阻塞模式**：
- **优点**：
  - 可以在等待数据的同时处理其他任务，提高资源利用率和程序响应速度。
  - 适合处理高并发和实时性要求高的应用场景。
- **缺点**：
  - 代码复杂度较高，需要处理更多的边界情况和错误。
  - 需要使用额外的机制（如 `select`、`poll` 或 `epoll`）来管理多个套接字的状态。

#### 2. 如何将上述通信改为非阻塞，避免阻塞？

**解答**：
可以通过将套接字设置为非阻塞模式来避免阻塞。以下是将套接字设置为非阻塞模式的步骤：

1. **创建套接字后，设置其为非阻塞模式**：
   ```c
   int flags = fcntl(sockfd, F_GETFL, 0);
   fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
   ```

2. **使用 `select`、`poll` 或 `epoll` 来监控套接字的状态**：
   - `select` 示例：
     ```c
     fd_set readfds;
     struct timeval tv;
     int retval;

     FD_ZERO(&readfds);
     FD_SET(sockfd, &readfds);

     // 设置超时时间
     tv.tv_sec = 5;
     tv.tv_usec = 0;

     retval = select(sockfd + 1, &readfds, NULL, NULL, &tv);
     if (retval == -1) {
         perror("select");
     } else if (retval) {
         bytes_received = recv(sockfd, buffer, BUFFER_SIZE, 0);
         // 处理接收到的数据
     } else {
         printf("No data within five seconds.\n");
     }
     ```

#### 3. 在传输前能否先将要传输的文件的相关属性性报告给对方，以便对方判断是否接受该文件的传输？

**解答**：
是的，可以在传输文件前先将文件的相关属性（如文件名、文件大小、文件类型等）报告给对方。这样对方可以根据这些信息判断是否接受文件传输。具体步骤如下：

1. **发送文件属性信息**：
   - 发送方在传输文件前，先发送一个包含文件属性信息的消息。
   - 例如，可以使用 JSON 格式来发送文件名、文件大小等信息。

2. **接收方判断是否接受文件传输**：
   - 接收方接收到文件属性信息后，可以根据这些信息做出决策（如检查文件大小是否合适，文件类型是否支持等）。
   - 如果接收方决定接受文件传输，可以发送一个确认消息给发送方。

3. **开始文件传输**：
   - 发送方在接收到确认消息后，开始传输文件数据。

**示例代码**：

**发送方**：
```c
// 发送文件属性信息
char file_info[256];
snprintf(file_info, sizeof(file_info), "{\"filename\": \"example.txt\", \"filesize\": %ld}", file_size);
send(sockfd, file_info, strlen(file_info), 0);

// 等待接收方确认
char response[64];
recv(sockfd, response, sizeof(response), 0);
if (strcmp(response, "ACCEPT") == 0) {
    // 开始文件传输
}
```

**接收方**：
```c
// 接收文件属性信息
char file_info[256];
recv(sockfd, file_info, sizeof(file_info), 0);

// 解析文件属性信息
// 例如，使用 JSON 库解析文件名和文件大小

// 判断是否接受文件传输
if (accept_file_transfer) {
    send(sockfd, "ACCEPT", strlen("ACCEPT"), 0);
    // 接收文件数据
} else {
    send(sockfd, "REJECT", strlen("REJECT"), 0);
}
```

#### 4. 了解并熟悉多线程工作原理，试编写基于多线程的网络文件传输程序。

**解答**：
可以使用多线程来实现网络文件传输程序，以提高并发处理能力。以下是一个简单的基于多线程的网络文件传输程序示例：

**发送端**：
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void *send_file(void *arg) {
    int sockfd = *(int *)arg;
    FILE *file = fopen("example.txt", "rb");
    if (file == NULL) {
        perror("fopen");
        close(sockfd);
        return NULL;
    }

    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        send(sockfd, buffer, bytes_read, 0);
    }

    fclose(file);
    close(sockfd);
    return NULL;
}

int main() {
    int sockfd;
    struct sockaddr_in servaddr;

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

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, send_file, &sockfd);
    pthread_join(thread_id, NULL);

    return 0;
}
```

**接收端**：
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void *receive_file(void *arg) {
    int connfd = *(int *)arg;
    FILE *file = fopen("received_example.txt", "wb");
    if (file == NULL) {
        perror("fopen");
        close(connfd);
        return NULL;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    while ((bytes_received = recv(connfd, buffer, BUFFER_SIZE, 0)) > 0) {
        fwrite(buffer, 1, bytes_received, file);
    }

    fclose(file);
    close(connfd);
    return NULL;
}

int main() {
    int sockfd, connfd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len;

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

    while (1) {
        len = sizeof(cliaddr);
        // 接受连接
        if ((connfd = accept(sockfd, (struct sockaddr *)&cliaddr, &len)) < 0) {
            perror("accept failed");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, receive_file, &connfd);
        pthread_detach(thread_id);
    }

    close(sockfd);
    return 0;
}
```

### 结果和分析

- **多线程发送端**：每次连接到服务器时，创建一个新线程来发送文件数据。
- **多线程接收端**：每次接受到新的连接时，创建一个新线程来接收文件数据。
- **并发处理**：通过多线程实现并发处理，提高了程序的响应能力和处理效率。
