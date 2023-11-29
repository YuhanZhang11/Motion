import random
import socket
import time
from datetime import datetime

def generate_data():
    data = [random.uniform(-10, 10) for _ in range(6)]
    return data

def transmit_data(data, client_socket, server_address, log_file):
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")
    data_str = f"{timestamp}, {' '.join(map(str, data))}\n"
    log_file.write(data_str)
    client_socket.sendto(data_str.encode(), server_address)

def main():
    PORT_B = 55000
    address_b = ('127.0.0.1', PORT_B)

    client_socket_b = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    expected_message = "this is motion server"
    client_socket_b.sendto(expected_message.encode(), address_b)
    print(234)

    received_message, server_address = client_socket_b.recvfrom(1024)
    print(345)
    
    print(f"[LOG] : Received message from machineb: {received_message.decode()}")

    if received_message == b"hi motion server":
        print("[LOG] : Waiting for machineb to handshake with machinec...")

        while True:
            received_message_b, server_address = client_socket_b.recvfrom(1024)

            if received_message_b == b"handshaked with machinec":
                print("[LOG] : Machineb has successfully handshake with machinec. Starting data transmission...")
                with open("machinea_log.txt", "a") as log_file:
                    for _ in range(20):
                        data = generate_data()
                        transmit_data(data, client_socket_b, address_b, log_file)
                        time.sleep(0.5)

                    # Send finishing message
                    finishing_message = "End of Transmission"
                    print("End of Transmission Message Sent to Machineb \n")
                    client_socket_b.sendto(finishing_message.encode(), server_address)
                break

    client_socket_b.close()

if __name__ == "__main__":
    main()