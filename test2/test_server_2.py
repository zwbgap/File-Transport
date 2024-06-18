import socket
import threading
import time

def handle_client(conn, addr):
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
    print(f"Thread {threading.current_thread().name} - Bandwidth: {bandwidth:.2f} MB/s")
    conn.close()

def server():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind(('127.0.0.1', 5001))
        s.listen()
        print("Server is listening on port 5001")
        while True:
            conn, addr = s.accept()
            threading.Thread(target=handle_client, args=(conn, addr)).start()

if __name__ == '__main__':
    server()