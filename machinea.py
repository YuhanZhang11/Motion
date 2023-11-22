import random
import socket
import time
from datetime import datetime

def generate_data():
    data = [random.uniform(-10, 10) for _ in range(6)]
    return data

def transmit_data(data, client_socket, log_file):
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")
    data_str = f"{timestamp}, {' '.join(map(str, data))}, {"\n"}"
    log_file.write(data_str)
    client_socket.send(data_str.encode())

def main():
    PORT_B = 55000
    address_b = ('127.0.0.1', PORT_B)

    client_socket_b = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket_b.connect(address_b)

    expected_message = "this is motion server"
    client_socket_b.send(expected_message.encode())

    received_message = client_socket_b.recv(1024).decode()
    print(f"[LOG] : Received message from machineb: {received_message}")

    if received_message == "hi motion server":
        print("[LOG] : Waiting for machineb to handshake with machinec...")

        while True:
            received_message_b = client_socket_b.recv(1024).decode()

            if received_message_b == "handshaked with machinec":
                print("[LOG] : Machineb has successfully handshake with machinec. Starting data transmission...")
                with open("machinea_log.txt", "a") as log_file:
                    for _ in range(20):  
                        data = generate_data()
                        transmit_data(data, client_socket_b, log_file)
                        time.sleep(0.5)
                break  # Break out of the loop once data transmission is done

    client_socket_b.close()

if __name__ == "__main__":
    main()