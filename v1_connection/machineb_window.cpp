#include <iostream>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <string>

using namespace std;

int main()
{
    int PORT_B = 55000; // Port for machinea to connect to
    int PORT_C = 50000; // Port for machinec

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cerr << "[ERROR] : WSAStartup failed\n";
        return 1;
    }

    SOCKET server_socket_descriptor, machinea_socket, machinec_socket;
    struct sockaddr_in server_address, machinea_address, machinec_address;
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

    cout << "[LOG] : Waiting for incoming connections from machinea on PORT_B...\n";

    machinea_socket = accept(server_socket_descriptor, (struct sockaddr *)&machinea_address, &server_address_length);
    if (machinea_socket == INVALID_SOCKET)
    {
        cerr << "[ERROR] : Accept from machinea failed.\n";
        closesocket(server_socket_descriptor);
        WSACleanup();
        return 1;
    }

    cout << "[LOG] : Connection accepted from machinea.\n";

    char buffer_a[1024] = {};
    int valread_a = recv(machinea_socket, buffer_a, 1024, 0);

    if (valread_a > 0)
    {
        string received_message_a(buffer_a);
        cout << "[LOG] : Received message from machinea: " << received_message_a << endl;

        if (received_message_a == "this is motion server")
        {
            cout << "[LOG] : Sending response to machinea.\n";
            send(machinea_socket, "hi motion server", strlen("hi motion server"), 0);

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

            // Accept connection from machinec
            machinec_socket = accept(machinec_server_socket_descriptor, (struct sockaddr *)&machinec_address, &server_address_length);
            if (machinec_socket == INVALID_SOCKET)
            {
                cerr << "[ERROR] : Accept from machinec failed.\n";
                closesocket(machinec_server_socket_descriptor);
                closesocket(machinea_socket);
                closesocket(server_socket_descriptor);
                WSACleanup();
                return 1;
            }

            cout << "[LOG] : Connection accepted from machinec.\n";

            while (true)
            {
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

                        // Continue communication with machinec for data requests
                        while (true)
                        {
                            // Simulating data reception from machinea
                            char data_buffer[1024] = {};
                            int data_valread = recv(machinea_socket, data_buffer, 1024, 0);

                            if (data_valread > 0)
                            {
                                string received_data(data_buffer);
                                cout << "[LOG] : Received data from machinea: " << received_data << endl;

                                // Store the received data as the most recent data
                                most_recent_data = received_data;
                            }
                            else
                            {
                                cout << "[ERROR] : No data received from machinea.\n";
                                break;
                            }
                        }

                        // Receive message from machinec requesting data
                        char request_buffer[1024] = {};
                        int request_valread = recv(machinec_socket, request_buffer, 1024, 0);

                        if (request_valread > 0)
                        {
                            string request_message(request_buffer);
                            cout << "[LOG] : Received request for data from machinec: " << request_message << endl;

                            if (request_message == "Request for data")
                            {
                                // Send the most recent data to machinec
                                send(machinec_socket, most_recent_data.c_str(), most_recent_data.length(), 0);
                                cout << "[LOG] : Sent most recent data to machinec.\n";
                            }
                            else
                            {
                                cout << "[ERROR] : Invalid message received from machinec. Closing connection.\n";
                                break;
                            }
                        }
                        else
                        {
                            cout << "[ERROR] : No message received from machinec.\n";
                            break;
                        }
                    }
                }
                else
                {
                    cout << "[ERROR] : Invalid message received from machinec. Closing connection.\n";
                    break;
                }
            }
            else
            {
                cout << "[ERROR] : No message received from machinec.\n";
                break;
            }
        }
    }
    else
    {
        cout << "[ERROR] : Invalid message received from machinea. Closing connection.\n";
    }
}
else
{
    cout << "[ERROR] : No message received from machinea.\n";
}

closesocket(server_socket_descriptor);
closesocket(machinea_socket);
closesocket(machinec_socket);
WSACleanup();
return 0;
}