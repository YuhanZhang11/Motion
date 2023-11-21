#include <iostream>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <string>
#include <chrono>
#include <thread>

using namespace std;

int main() {
    int PORT_C = 50000; // Port for machinec to connect to machineb

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "[ERROR] : WSAStartup failed\n";
        return 1;
    }

    SOCKET machinec_socket;
    struct sockaddr_in machinec_address;
    
    machinec_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (machinec_socket == INVALID_SOCKET) {
        cerr << "[ERROR] : Socket failed.\n";
        WSACleanup();
        return 1;
    }

    machinec_address.sin_family = AF_INET;
    machinec_address.sin_port = htons(PORT_C); // Port for machineb
    const char *machineb_ip = "127.0.0.1"; // Machine B's IP address
    inet_pton(AF_INET, machineb_ip, &machinec_address.sin_addr);

    if (connect(machinec_socket, (struct sockaddr *)&machinec_address, sizeof(machinec_address)) == SOCKET_ERROR) {
        cerr << "[ERROR] : Connection attempt to machineb failed.\n";
        closesocket(machinec_socket);
        WSACleanup();
        return 1;
    }

    cout << "[LOG] : Connected to machineb.\n";

    // Send message to machineb for handshake
    const char *handshake_msg = "Request for handshake";
    send(machinec_socket, handshake_msg, strlen(handshake_msg), 0);
    cout << "[LOG] : Sent handshake request to machineb.\n";

    char buffer[1024] = {};
    int valread = recv(machinec_socket, buffer, 1024, 0);

    if (valread > 0) {
        string received_message(buffer);
        cout << "[LOG] : Received handshake response from machineb: " << received_message << endl;

        if (received_message == "hello host!") {
            // Handshake successful, continue communication with machineb...
            while (true) {
                // Send message to machineb to request data
                const char *data_request = "Request for data";
                send(machinec_socket, data_request, strlen(data_request), 0);
                cout << "[LOG] : Sent data request to machineb.\n";

                // Receive response from machineb (most recent data)
                char data_buffer[1024] = {};
                int data_valread = recv(machinec_socket, data_buffer, 1024, 0);

                if (data_valread > 0) {
                    string received_data(data_buffer);
                    cout << "[LOG] : Received most recent data from machineb: " << received_data << endl;
                } else {
                    cout << "[ERROR] : No response received from machineb.\n";
                }

                // Wait for 2 seconds before sending the next request
                std::this_thread::sleep_for(std::chrono::seconds(2));
            }
        } else {
            cout << "[ERROR] : Invalid response received from machineb. Closing connection.\n";
        }
    } else {
        cout << "[ERROR] : No response received from machineb.\n";
    }

    closesocket(machinec_socket);
    WSACleanup();
    return 0;
}