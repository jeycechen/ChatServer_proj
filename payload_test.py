import socket
import time
from scapy.all import send, IP, TCP
from multiprocessing import Process

def tcp_test(server_ip, server_port, src_ip):
    try:
        # 创建原始套接字
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        # 绑定伪造的源IP地址
        sock.bind((src_ip, 0))
        # 连接到目标服务器
        sock.connect((server_ip, server_port))
        # 保持连接一段时间，例如60秒
        time.sleep(60)
    except Exception as e:
        print(f"Error: {e}")
    finally:
        sock.close()

def generate_ip(base_ip, index):
    # 生成新的IP地址
    parts = base_ip.split('.')
    parts[-1] = str(int(parts[-1]) + index)
    return '.'.join(parts)

def main():
    server_ip = '127.0.0.1'  # 目标Nginx IP
    server_port = 8000  # 目标Nginx端口
    base_src_ip = '192.168.1.1'  # 基础源IP地址
    num_connections = 100  # 并发连接数量

    processes = []

    for i in range(num_connections):
        src_ip = generate_ip(base_src_ip, i)
        p = Process(target=tcp_test, args=(server_ip, server_port, src_ip))
        p.start()
        processes.append(p)

    for p in processes:
        p.join()

if __name__ == "__main__":
    main()
