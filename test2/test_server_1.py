import socket
import time

def server():
    # 创建一个TCP/IP套接字
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        # 绑定服务器地址和端口
        s.bind(('127.0.0.1', 5001))
        # 开始监听连接
        s.listen()
        print("Server is listening on port 5001")
        # 接受连接
        conn, addr = s.accept()
        with conn:
            print('Connected by', addr)
            start_time = time.time()
            total_data = 0
            while True:
                data = conn.recv(1024)
                if not data:
                    break
                total_data += len(data)
            end_time = time.time()
            duration = end_time - start_time
            bandwidth = total_data / duration / 1024 / 1024  # 转换为MB/s
            print(f"Bandwidth: {bandwidth:.2f} MB/s")

if __name__ == '__main__':
    server()