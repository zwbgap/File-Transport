import socket
import threading
import time

def client_thread(server_ip, duration):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((server_ip, 5001))
        start_time = time.time()
        while time.time() - start_time < duration:
            s.sendall(b'x' * 1024)
        s.shutdown(socket.SHUT_WR)

def client(server_ip, num_threads, duration):
    threads = []
    for _ in range(num_threads):
        thread = threading.Thread(target=client_thread, args=(server_ip, duration))
        threads.append(thread)
        thread.start()
    for thread in threads:
        thread.join()

if __name__ == '__main__':
    import sys
    if len(sys.argv) != 4:
        print("Usage: python client.py <server_ip> <num_threads> <duration>")
        sys.exit(1)
    server_ip = sys.argv[1]
    num_threads = int(sys.argv[2])
    duration = int(sys.argv[3])
    client(server_ip, num_threads, duration)