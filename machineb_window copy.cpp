#include <iostream>
#include <fstream>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

int main() {
    int PORT_B = 55000; // Port for machinea to connect to
    int PORT_C = 50000; // Port for machinec

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "[ERROR] : WSAStartup failed\n";
        return 1;
    }

    SOCKET server_socket_descriptor, machinea_socket, machinec_socket;
    struct sockaddr_in server_address, machinea_address, machinec_address;
    int server_address_length = sizeof(server_address);

    server_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_descriptor == INVALID_SOCKET) {
        cerr << "[ERROR] : Socket failed.\n";
        WSACleanup();
        return 1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT_B);

    if (bind(server_socket_descriptor, (struct sockaddr *)&server_address, sizeof(server_address)) == SOCKET_ERROR) {
        cerr << "[ERROR] : Bind failed: " << WSAGetLastError() << "\n";
        closesocket(server_socket_descriptor);
        WSACleanup();
        return 1;
    }

    if (listen(server_socket_descriptor, 3) == SOCKET_ERROR) {
        cerr << "[ERROR] : Listen failed.\n";
        closesocket(server_socket_descriptor);
        WSACleanup();
        return 1;
    }

    cout << "[LOG] : Waiting for incoming connections from machinea on PORT_B...\n";

    machinea_socket = accept(server_socket_descriptor, (struct sockaddr *)&machinea_address, &server_address_length);
    if (machinea_socket == INVALID_SOCKET) {
        cerr << "[ERROR] : Accept from machinea failed.\n";
        closesocket(server_socket_descriptor);
        WSACleanup();
        return 1;
    }

    cout << "[LOG] : Connection accepted from machinea.\n";

    char buffer[1024] = {};
    int valread = recv(machinea_socket, buffer, 1024, 0);

    if (valread > 0) {
        string received_message(buffer);
        cout << "[LOG] : Received message from machinea: " << received_message << endl;

        if (received_message == "this is motion server") {
            cout << "[LOG] : Sending response to machinea.\n";
            send(machinea_socket, "hi motion server", strlen("hi motion server"), 0);

            // Now, waiting for machinec's connection and message
            machinec_socket = socket(AF_INET, SOCK_STREAM, 0);
            if (machinec_socket == INVALID_SOCKET) {
                cerr << "[ERROR] : Machinec socket creation failed.\n";
                closesocket(machinea_socket);
                closesocket(server_socket_descriptor);
                WSACleanup();
                return 1;
            }

            machinec_address.sin_family = AF_INET;
            machinec_address.sin_port = htons(PORT_C);
            const char *machinec_ip = "127.0.0.1";
            inet_pton(AF_INET, machinec_ip, &machinec_address.sin_addr);

            if (connect(machinec_socket, (struct sockaddr *)&machinec_address, sizeof(machinec_address)) == SOCKET_ERROR) {
                cerr << "[ERROR] : Connection attempt to machinec failed.\n";
                closesocket(machinec_socket);
                closesocket(machinea_socket);
                closesocket(server_socket_descriptor);
                WSACleanup();
                return 1;
            }

            cout << "[LOG] : Connected to machinec.\n";

            while (true) {
                char c_buffer[1024] = {};
                int c_valread = recv(machinec_socket, c_buffer, 1024, 0);

                if (c_valread > 0) {
                    string c_received_message(c_buffer);
                    cout << "[LOG] : Received message from machinec: " << c_received_message << endl;

                    if (c_received_message == "send data") {
                        ifstream dataFile("data.txt"); // Open data file

                        if (!dataFile.is_open()) {
                            cerr << "[ERROR] : Failed to open data file for reading.\n";
                            closesocket(machinec_socket);
                            closesocket(machinea_socket);
                            closesocket(server_socket_descriptor);
                            WSACleanup();
                            return 1;
                        }

                        string line;
                        string latestData;

                        // Read the most recent line from the file
                        while (getline(dataFile, line)) {
                            latestData = line;
                        }

                        dataFile.close();

                        // Send the most recent data to Machine C
                        send(machinec_socket, latestData.c_str(), latestData.size(), 0);
                        cout << "[LOG] : Sent data to machinec.\n";
                    } else {
                        cout << "[ERROR] : Invalid message received from machinec. Closing connection.\n";
                        closesocket(machinec_socket);
                        closesocket(machinea_socket);
                        closesocket(server_socket_descriptor);
                        WSACleanup();
                        return 1;
                    }
                }
            }
        } else {
            cerr << "[ERROR] : No message received from machinea.\n";
            closesocket(machinea_socket);
            closesocket(server_socket_descriptor);
            WSACleanup();
            return 1;
        }
    }

    closesocket(server_socket_descriptor);
    closesocket(machinea_socket);
    closesocket(machinec_socket);
    WSACleanup();
    return 0;
}
