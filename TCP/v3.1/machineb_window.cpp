#include <iostream>
#include <iostream>
#include <sstream>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <string>
#include <future>
#include <chrono>

using namespace std;

// Global Variable
string most_recent_data;
bool close_machinec = false;

// Helper Function
string getLastLine(const string &receivedData)
{
    istringstream stream(receivedData);
    string lastLine, line;

    while (getline(stream, line))
    {
        lastLine = line;
    }

    return lastLine;
}

int main()
{
    int PORT_B = 55000; // Port for machinea
    int PORT_C = 50000; // Port for machinec
    bool machinea_connect = true;
    bool machinea_ready = false;
    bool machinec_ready = false;

    SOCKET server_socket_descriptor, machinea_socket, machinec_socket;
    struct sockaddr_in server_address, machinea_address, machinec_address;
    int server_address_length = sizeof(server_address);

    while (true)
    {
        // Do the connection is machinea is not ready
        if (!machinea_ready)
        {
            WSADATA wsaData;
            if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
            {
                cerr << "[ERROR] : WSAStartup failed\n";
                machinea_connect = false;
            }

            server_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
            if (server_socket_descriptor == INVALID_SOCKET)
            {
                cerr << "[ERROR] : Machinea socket failed.\n";
                WSACleanup();
                machinea_connect = false;
            }

            server_address.sin_family = AF_INET;
            server_address.sin_addr.s_addr = INADDR_ANY;
            server_address.sin_port = htons(PORT_B);

            if (bind(server_socket_descriptor, (struct sockaddr *)&server_address, sizeof(server_address)) == SOCKET_ERROR)
            {
                cerr << "[ERROR] : Machinea bind failed: " << WSAGetLastError() << "\n";
                closesocket(server_socket_descriptor);
                WSACleanup();
                machinea_connect = false;
            }

            if (listen(server_socket_descriptor, 3) == SOCKET_ERROR)
            {
                cerr << "[ERROR] : Machinea Listen failed.\n";
                closesocket(server_socket_descriptor);
                WSACleanup();
                machinea_connect = false;
            }

            if (machinea_connect)
            {
                cout << "[LOG] : Waiting for incoming connections from machinea on PORT_B...\n";

                // Start the timer before attempting to accept
                auto start_time = std::chrono::steady_clock::now();
                bool timeout = false;

                // Check if the elapsed time exceeds 2 seconds
                while (!timeout)
                {
                    auto current_time = std::chrono::steady_clock::now();
                    auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time).count();

                    if (elapsed_time >= 2)
                    {
                        std::cerr << "[LOG] : Timeout waiting for machinea connection.\n";
                        timeout = true;
                        break;
                    }

                    // Attempt to accept the connection in a non-blocking manner
                    machinea_socket = accept(server_socket_descriptor, (struct sockaddr *)&machinea_address, &server_address_length);

                    if (machinea_socket == INVALID_SOCKET)
                    {
                        cerr << "[ERROR] : Accept from machinea failed.\n";
                        closesocket(server_socket_descriptor);
                        WSACleanup();
                    }

                    cout << "[LOG] : Connection accepted from machinea.\n";
                    machinea_ready = true;

                    char buffer_a[1024] = {};
                    int valread_a = recv(machinea_socket, buffer_a, 1024, 0);

                    if (valread_a > 0)
                    {
                        string received_message_a(buffer_a);
                        cout << "[LOG] : Received message from machinea: " << received_message_a << endl;

                        // The connection is from the motion server
                        if (received_message_a == "this is motion server")
                        {
                            // motion server connection successful; can start data sending
                            if (machinea_ready)
                            {
                                machinea_ready = true;
                                cout << "[LOG] : Machinea connection ready\n";
                                send(machinea_socket, "Machinea connection ready", strlen("Machinea connection ready"), 0);
                            }
                            // motion server connection unsuccessful;
                            else
                            {
                                cout << "[LOG] : Machinea connection not ready\n";
                            }
                        }
                        else
                        {
                            cout << "Not motion server" << endl;
                            closesocket(server_socket_descriptor);
                            closesocket(machinea_socket);
                            // WSACleanup();
                        }
                    }
                    break;
                }
            }

            // Do the connection if machinec not ready
            if (!machinec_ready)
            {
                SOCKET machinec_server_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
                if (machinec_server_socket_descriptor == INVALID_SOCKET)
                {
                    cerr << "[ERROR] : Machinec socket creation failed.\n";
                    closesocket(machinec_socket);
                    closesocket(machinec_server_socket_descriptor);
                    WSACleanup();
                    continue;
                }

                server_address.sin_port = htons(PORT_C);

                if (bind(machinec_server_socket_descriptor, (struct sockaddr *)&server_address, sizeof(server_address)) == SOCKET_ERROR)
                {
                    cerr << "[ERROR] : Bind for machinec failed: " << WSAGetLastError() << "\n";
                    closesocket(machinec_server_socket_descriptor);
                    closesocket(server_socket_descriptor);
                    WSACleanup();
                    continue;
                }

                if (listen(machinec_server_socket_descriptor, 3) == SOCKET_ERROR)
                {
                    cerr << "[ERROR] : Listen for machinec failed.\n";
                    closesocket(machinec_server_socket_descriptor);
                    closesocket(server_socket_descriptor);
                    WSACleanup();
                    continue;
                }

                cout << "[LOG] : Waiting for incoming connections on PORT_C for machinec...\n";

                machinec_socket = accept(machinec_server_socket_descriptor, (struct sockaddr *)&machinec_address, &server_address_length);
                if (machinec_socket == INVALID_SOCKET)
                {
                    cerr << "[ERROR] : Accept from machinec failed.\n";
                    closesocket(machinec_server_socket_descriptor);
                    closesocket(machinea_socket);
                    closesocket(server_socket_descriptor);
                    WSACleanup();
                    continue;
                }

                cout << "[LOG] : Connection accepted from machinec.\n";

                // Handshake with Machine C
                char buffer_c[1024] = {};
                int valread_c = recv(machinec_socket, buffer_c, 1024, 0);

                if (valread_c > 0)
                {
                    string received_message_c(buffer_c);
                    cout << "[LOG] : Received message from machinec: " << received_message_c << endl;

                    if (received_message_c == "this is MARS")
                    {
                        // change machinec_ready
                        machinec_ready = true;

                        // Send response to Machine C
                        cout << "MARS ready to request data" << received_message_c << endl;
                        send(machinec_socket, "MARS ready to request data", strlen("MARS ready to request data"), 0);
                    }
                    else
                    {
                        cout << "Not MARS" << received_message_c << endl;
                        closesocket(server_socket_descriptor);
                        closesocket(machinec_socket);
                        // WSACleanup();
                    }
                }
            }

            // Start data transmission
            while (machinea_ready && machinec_ready)
            {
                char data_buffer[1024] = {};
                int data_valread = recv(machinea_socket, data_buffer, 1024, 0);

                if (data_valread > 0)
                {
                    string received_data(data_buffer);

                    // Extracting the last line from received_data
                    string last_received_data = getLastLine(received_data);
                    cout << "Last received line from machinea: " << last_received_data << endl;
                    most_recent_data = last_received_data;
                }
                else
                {
                    // Try reconnect
                    continue;
                }

                char request_buffer[1024] = {};
                int request_valread = recv(machinec_socket, request_buffer, 1024, 0);

                if (request_valread > 0)
                {
                    string request_message(request_buffer);
                    cout << "[LOG] : Received request for data from machinec: " << request_message << endl;

                    if (request_message == "Request for data")
                    {
                        send(machinec_socket, most_recent_data.c_str(), most_recent_data.length(), 0);
                        cout << "[LOG] : Sent most recent data to machinec.\n";
                    }
                }
            }

            closesocket(server_socket_descriptor);
            closesocket(machinea_socket);
            closesocket(machinec_socket);
            WSACleanup();
            return 0;
        }
    }
}