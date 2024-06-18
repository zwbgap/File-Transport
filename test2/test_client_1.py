import socket
import time

def client(server_ip):
    # 创建一个TCP/IP套接字
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        # 连接到服务器
        s.connect((server_ip, 5001))
        start_time = time.time()
        print("Client is sending data...")
        while time.time() - start_time < 10:  # 发送10秒的数据
            s.sendall(b'x' * 1024)
        s.shutdown(socket.SHUT_WR)
        print("Client has finished sending data")

if __name__ == '__main__':
    import sys
    if len(sys.argv) != 2:
        print("Usage: python client.py <server_ip>")
        sys.exit(1)
    client(sys.argv[1])