#This is a basic script to host a webpage at the IP specified
import time
import socket




server_address_1 =  ('127.0.0.2', 8001)
sock_1 = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock_1.bind(server_address_1)

def main():
    max_len = 65507
    while True:
        frame,_ = sock_1.recvfrom(max_len)
        print(str(frame))

if __name__ == "__main__":
    main()
