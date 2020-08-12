# python36
import socket
import sys

address = sys.argv[1] if len(sys.argv) > 1 else "0.0.0.0:7788"
ip, port = address.split(":")
port = int(port)
print(f"Binding to IP: {ip}  PORT: {port}")


server = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
server.bind((ip, port))

while True:
    data, addr = server.recvfrom(1024)
    print(f"Got message {data} from {addr}")

    server.sendto(b"Thank you!", addr)
    print(f"Replied with thank you.")

