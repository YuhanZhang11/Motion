import random
import socket
import time
from datetime import datetime

def generate_data():
    data = [random.uniform(-10, 10) for _ in range(6)]
    return data

def transmit_data(data, client_socket, log_file):
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    data_str = f"{timestamp},{' '.join(map(str, data))}"
    log_file.write(data_str + "\n")
    client_socket.send(data_str.encode())

def main():
    PORT = 55000
    address = ('127.0.0.1', PORT)

    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect(address)

    expected_message = "this is motion server"
    client_socket.send(expected_message.encode())

    received_message = client_socket.recv(1024).decode()
    print(f"[LOG] : Received message from machineb: {received_message}")

    if received_message == "hi motion server":
        print("[LOG] : Starting data transmission...")
        with open("machinea_log.txt", "a") as log_file:
            for _ in range(20):  
                data = generate_data()
                transmit_data(data, client_socket, log_file)
                time.sleep(0.5)

    client_socket.close()

if __name__ == "__main__":
    main()