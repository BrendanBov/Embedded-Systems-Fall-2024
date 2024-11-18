#!/usr/bin/env python
import time
import socket

server_address = ('127.0.0.2',8001)
client = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)

def main():
    while(1):
        data = b'hello'
        client.sendto(data,server_address)
        time.sleep(0.1)
       
if __name__ == '__main__': 
   main()