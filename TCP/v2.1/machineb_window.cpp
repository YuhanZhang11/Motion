#include <iostream>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <string>

using namespace std;

int main()
{
    int PORT_B = 55000; // Port for machinea to connect to
    int PORT_C = 50000; // Port for machinec
    bool a_connected = false;
    bool c_connected = false;

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cerr << "[ERROR] : WSAStartup failed\n";
        return 1;
    }

    SOCKET server_socket_descriptor, machinea_socket, machinec_socket, temp_socket;
    struct sockaddr_in server_address, machinea_address, machinec_address, temp_address;
    int server_address_length = sizeof(server_address);

    server_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_descriptor == INVALID_SOCKET)
    {
        cerr << "[ERROR] : Socket failed.\n";
        WSACleanup();
        return 1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT_B);

    if (bind(server_socket_descriptor, (struct sockaddr *)&server_address, sizeof(server_address)) == SOCKET_ERROR)
    {
        cerr << "[ERROR] : Bind failed: " << WSAGetLastError() << "\n";
        closesocket(server_socket_descriptor);
        WSACleanup();
        return 1;
    }

    if (listen(server_socket_descriptor, 3) == SOCKET_ERROR)
    {
        cerr << "[ERROR] : Listen failed.\n";
        closesocket(server_socket_descriptor);
        WSACleanup();
        return 1;
    }

    // Create a separate socket for machinec
    SOCKET machinec_server_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (machinec_server_socket_descriptor == INVALID_SOCKET)
    {
        cerr << "[ERROR] : Machinec socket creation failed.\n";
        closesocket(machinea_socket);
        closesocket(server_socket_descriptor);
        WSACleanup();
        return 1;
    }

    server_address.sin_port = htons(PORT_C);

    if (bind(machinec_server_socket_descriptor, (struct sockaddr *)&server_address, sizeof(server_address)) == SOCKET_ERROR)
    {
        cerr << "[ERROR] : Bind for machinec failed: " << WSAGetLastError() << "\n";
        closesocket(machinec_server_socket_descriptor);
        closesocket(server_socket_descriptor);
        WSACleanup();
        return 1;
    }

    if (listen(machinec_server_socket_descriptor, 3) == SOCKET_ERROR)
    {
        cerr << "[ERROR] : Listen for machinec failed.\n";
        closesocket(machinec_server_socket_descriptor);
        closesocket(server_socket_descriptor);
        WSACleanup();
        return 1;
    }

    cout << "[LOG] : Waiting for incoming connections on PORT_C for machinec...\n";

    while (true)
    {
        cout << "[LOG] : Waiting for incoming connections from machinea on PORT_B...\n";

        temp_socket = accept(server_socket_descriptor, (struct sockaddr *)&temp_address, &server_address_length);
        if (temp_socket == INVALID_SOCKET)
        {
            cerr << "[ERROR] : Accept from random socket failed.\n";
            closesocket(server_socket_descriptor);
            WSACleanup();
            return 1;
        }
        cout << "[LOG] : Connection accepted from some machine.\n";

        // Now need to verify the identity of the machine
        char buffer[1024] = {};
        int valread = recv(temp_socket, buffer, 1024, 0);

        if (valread > 0)
        {
            string received_message(buffer);
            cout << "[LOG] : Received message from machinea: " << received_message << endl;

            if (received_message == "this is motion server")
            {
                // This is machinea(motion server)
                cout << "Set machinea. \n";
                machinea_socket = temp_socket;
                machinea_address = temp_address;

                cout << "[LOG] : Sending response to machinea.\n";
                send(machinea_socket, "hi motion server", strlen("hi motion server"), 0);
            }
            else if (received_message == "this is the host")
            {
                cout << "Set machinec. \n";
                machinec_socket = temp_socket;
                machinec_address = temp_address;
                cout << "[LOG] : Sending response to machinec.\n";
                send(machinec_socket, "hello host!", strlen("hello host!"), 0);

            } else if (received_message == "this is the host") {

            }



            // Receive message from machinec
            char buffer_c[1024] = {};
            int valread_c = recv(machinec_socket, buffer_c, 1024, 0);

            if (valread_c > 0)
            {
                string received_message_c(buffer_c);
                cout << "[LOG] : Received message from machinec: " << received_message_c << endl;

                if (received_message_c == "this is the host")
                {
                    // Send response to machinec
                    send(machinec_socket, "hello host!", strlen("hello host!"), 0);

                }
 
    