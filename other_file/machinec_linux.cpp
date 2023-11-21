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
    int PORT = 49762;

    // Create a socket for listening to incoming data
    int server_socket_descriptor, new_socket;
    struct sockaddr_in address;
    int address_length = sizeof(address);

    if ((server_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("[ERROR] : Socket failed.\n");
        exit(EXIT_FAILURE);
    }
    cout << "[LOG] : Server Socket Created Successfully.\n";

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_socket_descriptor, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("[ERROR] : Bind failed.\n");
        exit(EXIT_FAILURE);
    }
    cout << "[LOG] : Binding Successful.\n";

    if (listen(server_socket_descriptor, 3) < 0) {
        perror("[ERROR] : Listen failed.\n");
        exit(EXIT_FAILURE);
    }
    cout << "[LOG] : Listening for incoming data...\n";

    if ((new_socket = accept(server_socket_descriptor, (struct sockaddr *)&address, (socklen_t *)&address_length)) < 0) {
        perror("[ERROR] : Accept failed.\n");
        exit(EXIT_FAILURE);
    }

    cout << "[LOG] : Accepted connection from Machine A.\n";

    // Receive and log data
    char buffer[1024] = {};
    while (true) {
        int valread = read(new_socket, buffer, 1024);

        if (valread == 0) {
            cout << "[LOG] : No more data received. Exiting.\n";
            break;
        }

        cout << "[LOG] : Data received from Machine A: " << buffer << endl;

        // Log the data to a log file (e.g., machinec_log.txt)
        ofstream logfile;
        logfile.open("./machinec_log.txt", ios::app);

        if (logfile.is_open()) {
            logfile << buffer << endl;
            logfile.close();
            cout << "[LOG] : Data appended to machinec_log.txt.\n";
        } else {
            cout << "[ERROR] : Machine C log file creation failed, Exiting.\n";
            exit(EXIT_FAILURE);
        }

        memset(buffer, 0, sizeof(buffer);  // Clear the buffer
    }

    return 0;
}
