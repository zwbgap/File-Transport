import socket
import threading
import os

# 配置
SERVER_IP = '127.0.0.1'
SERVER_PORT = 8080
FILENAME = 'received_002_kaoyan.mp4'
NUM_THREADS = 8  # 线程数

def receive_file_part(filename, offset, size, conn):
    with open(filename, 'r+b') as f:
        f.seek(offset)
        remaining = size
        while remaining > 0:
            data = conn.recv(min(4096, remaining))
            if not data:
                break
            f.write(data)
            remaining -= len(data)
    print(f"Received part at offset {offset} with size {size}")

def handle_client(conn, addr, file_size, part_size, thread_id):
    offset = thread_id * part_size
    size = part_size if thread_id < NUM_THREADS - 1 else file_size - offset
    receive_file_part(FILENAME, offset, size, conn)
    conn.close()

def main():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((SERVER_IP, SERVER_PORT))
        s.listen()

        file_size = int(input("Enter the size of the file to receive (in bytes): "))
        part_size = file_size // NUM_THREADS

        # 创建一个空文件
        with open(FILENAME, 'wb') as f:
            f.truncate(file_size)

        threads = []
        for i in range(NUM_THREADS):
            conn, addr = s.accept()
            thread = threading.Thread(target=handle_client, args=(conn, addr, file_size, part_size, i))
            threads.append(thread)
            thread.start()

        for thread in threads:
            thread.join()

if __name__ == '__main__':
    main()