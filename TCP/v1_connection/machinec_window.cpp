#include <iostream>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <string>


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

    // Send message to machineb
    const char *msg = "this is the host";
    send(machinec_socket, msg, strlen(msg), 0);
    cout << "[LOG] : Sent message to machineb: " << msg << endl;

    // Receive response from machineb
    char buffer[1024] = {};
    int valread = recv(machinec_socket, buffer, 1024, 0);

    if (valread > 0) {
        string received_message(buffer);
        cout << "[LOG] : Received response from machineb: " << received_message << endl;

        if (received_message == "hello host!") {
            // Handshake successful, continue communication with machineb...
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
