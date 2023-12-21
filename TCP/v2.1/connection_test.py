#####################
# Here we test the connection limit and behaviours

####################

import socket
import time

def connect_to_server(port):
    # Create a socket object
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    # Define the server address and port
    server_address = ('localhost', port)

    try:
        # Connect to the server
        client_socket.connect(server_address)
        print(f"Connected to the server on port {port}")

        # Optionally, you can send data or perform other operations here

    except ConnectionRefusedError:
        print(f"Connection was refused, ensure the server is running on port {port}")

    finally:
        # Close the socket
        client_socket.close()

# Make two connections sequentially
connect_to_server(55000)  # Connect to port 55000
time.sleep(1)  # Wait for 1 second between connections
connect_to_server(55000)  # Connect to port 55000 again
