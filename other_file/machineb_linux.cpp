#include <iostream>
#include <fstream>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>

using namespace std;

int main() {
    int PORT_B = 49762;  // Port for machinea to connect to
    int PORT_C = 50000;  // Port for machinec

    // Create a socket to listen for connections from machinea
    int server_socket_descriptor, new_socket;
    struct sockaddr_in server_address;
    int server_address_length = sizeof(server_address);

    if ((server_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("[ERROR] : Socket failed.\n");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT_B);

    if (bind(server_socket_descriptor, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("[ERROR] : Bind failed.\n");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket_descriptor, 3) < 0) {
        perror("[ERROR] : Listen failed.\n");
        exit(EXIT_FAILURE);
    }

    cout << "[LOG] : Listening for incoming data from machinea on PORT_B...\n";

    if ((new_socket = accept(server_socket_descriptor, (struct sockaddr *)&server_address, (socklen_t *)&server_address_length)) < 0) {
        perror("[ERROR] : Accept failed.\n");
        exit(EXIT_FAILURE);
    }

    cout << "[LOG] : Accepted connection from machinea.\n";

    int PORT_C = 50000;  // Port for machinec

    int client_socket_descriptor;
    struct sockaddr_in client_address;

    if ((client_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("[ERROR] : Socket failed.\n");
        exit(EXIT_FAILURE);
    }

    client_address.sin_family = AF_INET;
    client_address.sin_port = htons(PORT_C);

    const char* machinec_ip = "127.0.0.1";
    if (inet_pton(AF_INET, machinec_ip, &client_address.sin_addr) <= 0) {
        cout << "[ERROR] : Invalid address\n";
        exit(EXIT_FAILURE);
    }

    if (connect(client_socket_descriptor, (struct sockaddr *)&client_address, sizeof(client_address)) < 0) {
        perror("[ERROR] : Connection attempt to machinec failed.\n");
        exit(EXIT_FAILURE);
    }

    cout << "[LOG] : Connected to machinec.\n";

    // Logging: Open a log file for writing
    ofstream logFile;
    logFile.open("machineb_log.txt", ios::app);

    if (!logFile.is_open()) {
        cout << "[ERROR] : Failed to open log file for writing.\n";
        exit(EXIT_FAILURE);
    }

    char buffer[1024] = {};

    while (true) {
        int valread = read(new_socket, buffer, 1024);

        if (valread == 0) {
            cout << "[LOG] : No more data received from machinea. Exiting.\n";
            break;
        }

        cout << "[LOG] : Data received from machinea: " << buffer << endl;

        // Logging: Write received data to the log file
        logFile << buffer << endl;
        logFile.flush();

        // Send the received data to machinec
        send(client_socket_descriptor, buffer, valread, 0);
        cout << "[LOG] : Data sent to machinec.\n";

        memset(buffer, 0, sizeof(buffer));
    }

    // Close the sockets
    close(server_socket_descriptor);
    close(new_socket);
    close(client_socket_descriptor);

    // Logging: Close the log file
    logFile.close();

    return 0;
}
