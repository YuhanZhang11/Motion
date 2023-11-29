#include <iostream>
#include <iostream>
#include <sstream>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <string>

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

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cerr << "[ERROR] : WSAStartup failed\n";
        return 1;
    }

    SOCKET machinea_socket, machinec_socket;
    struct sockaddr_in machinea_address, machinec_address;
    int address_length = sizeof(machinea_address);

    machinea_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (machinea_socket == INVALID_SOCKET)
    {
        cerr << "[ERROR] : Machinea socket creation failed.\n";
        WSACleanup();
        return 1;
    }

    machinea_address.sin_family = AF_INET;
    machinea_address.sin_addr.s_addr = INADDR_ANY;
    machinea_address.sin_port = htons(PORT_B);

    if (bind(machinea_socket, (struct sockaddr *)&machinea_address, sizeof(machinea_address)) == SOCKET_ERROR)
    {
        cerr << "[ERROR] : Bind for machinea failed: " << WSAGetLastError() << "\n";
        closesocket(machinea_socket);
        WSACleanup();
        return 1;
    }

    cout << "[LOG] : Waiting for incoming messages from machinea on PORT_B...\n";

    char buffer_a[1024] = {};
    int valread_a = recvfrom(machinea_socket, buffer_a, 1024, 0, (struct sockaddr *)&machinea_address, &address_length);

    if (valread_a > 0)
    {
        string received_message_a(buffer_a);
        cout << "[LOG] : Received message from machinea: " << received_message_a << endl;

        if (received_message_a == "this is motion server")
        {
            cout << "[LOG] : Sending response to machinea.\n";
            sendto(machinea_socket, "hi motion server", strlen("hi motion server"), 0, (struct sockaddr *)&machinea_address, address_length);

            machinec_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if (machinec_socket == INVALID_SOCKET)
            {
                cerr << "[ERROR] : Machinec socket creation failed.\n";
                closesocket(machinea_socket);
                WSACleanup();
                return 1;
            }

            machinec_address.sin_family = AF_INET;
            machinec_address.sin_addr.s_addr = INADDR_ANY;
            machinec_address.sin_port = htons(PORT_C);

            if (bind(machinec_socket, (struct sockaddr *)&machinec_address, sizeof(machinec_address)) == SOCKET_ERROR)
            {
                cerr << "[ERROR] : Bind for machinec failed: " << WSAGetLastError() << "\n";
                closesocket(machinec_socket);
                WSACleanup();
                return 1;
            }

            cout << "[LOG] : Waiting for incoming messages on PORT_C for machinec...\n";

            char buffer_c[1024] = {};
            int valread_c = recvfrom(machinec_socket, buffer_c, 1024, 0, (struct sockaddr *)&machinec_address, &address_length);

            if (valread_c > 0)
            {
                string received_message_c(buffer_c);
                cout << "[LOG] : Received message from machinec: " << received_message_c << endl;

                if (received_message_c == "this is the host")
                {
                    // Send response to Machine C
                    sendto(machinec_socket, "hello host!", strlen("hello host!"), 0, (struct sockaddr *)&machinec_address, address_length);
                    // Send response to Machine A
                    sendto(machinea_socket, "handshaked with machinec", strlen("handshaked with machinec"), 0, (struct sockaddr *)&machinea_address, address_length);

                    // Start data transmission
                    while (true)
                    {
                        char data_buffer[1024] = {};
                        int data_valread = recvfrom(machinea_socket, data_buffer, 1024, 0, (struct sockaddr *)&machinea_address, &address_length);

                        if (data_valread > 0)
                        {
                            string received_data(data_buffer);

                            if (received_data == "End of Transmission")
                            {
                                cout << "Let's terminate machinec" << endl;
                                close_machinec = 1;
                            }
                            else
                            {
                                string last_received_data = getLastLine(received_data);
                                cout << "Last received line from machinea: " << last_received_data << endl;
                                most_recent_data = last_received_data;
                            }
                        }

                        char request_buffer[1024] = {};
                        int request_valread = recvfrom(machinec_socket, request_buffer, 1024, 0, (struct sockaddr *)&machinec_address, &address_length);

                        if (request_valread > 0)
                        {
                            string request_message(request_buffer);
                            cout << "[LOG] : Received request for data from machinec: " << request_message << endl;

                            if (request_message == "Request for data")
                            {
                                if (close_machinec == true)
                                {
                                    // Send message to machinec to exit from the while loop
                                    sendto(machinec_socket, "close program now", strlen("close program now"), 0, (struct sockaddr *)&machinec_address, address_length);
                                    cout << "Closing signal sent to machinec.\n";
                                    break;
                                }
                                sendto(machinec_socket, most_recent_data.c_str(), most_recent_data.length(), 0, (struct sockaddr *)&machinec_address, address_length);
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
                }
            }
            else
            {
                cout << "[ERROR] : No message received from machinec.\n";
            }
        }
    }
    else
    {
        cout << "[ERROR] : Invalid message received from machinea. Closing connection.\n";
    }
    closesocket(machinea_socket);
    closesocket(machinec_socket);
    WSACleanup();
    return 0;
}
