import socket
import threading
import os

# 配置
SERVER_IP = '127.0.0.1'
SERVER_PORT = 8080
FILENAME = '002_kaoyan.mp4'
NUM_THREADS = 8  # 线程数

def send_file_part(filename, offset, size, server_ip, server_port):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((server_ip, server_port))
        with open(filename, 'rb') as f:
            f.seek(offset)
            data = f.read(size)
            s.sendall(data)
    print(f"Sent part from offset {offset} with size {size}")

def main():
    file_size = os.path.getsize(FILENAME)
    print(file_size);
    part_size = file_size // NUM_THREADS

    threads = []
    for i in range(NUM_THREADS):
        offset = i * part_size
        size = part_size if i < NUM_THREADS - 1 else file_size - offset
        thread = threading.Thread(target=send_file_part, args=(FILENAME, offset, size, SERVER_IP, SERVER_PORT))
        threads.append(thread)
        thread.start()

    for thread in threads:
        thread.join()

if __name__ == '__main__':
    main()