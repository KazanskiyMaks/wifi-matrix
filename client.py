# client.py

import socket

HOST = "192.168.0.109"  # The server's hostname or IP address
PORT = 65432  # The port used by the server


def start_client(host, port):
    print("Enter message to send (or 'quit' to exit): ")
    while True:
        message = input("> ")
        str_hash = 0

        if message.lower() == 'quit':
            break

        for b in message.encode():
            str_hash += b

        str_hash %= 256

        client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client.connect((host, port))
        client.send(message.encode())
        response = client.recv(1)
        server_hash = int.from_bytes(response)
        # print(f"{str_hash} / {server_hash}")
        response = client.recv(1024)
        if str_hash != server_hash:
            print("Unsuccessfully sent")
        # print(response)
        client.close()


if __name__ == "__main__":
    start_client(HOST, PORT)
