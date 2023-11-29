#include <iostream>
#include <sstream>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <string>
#include <thread>
#include <mutex>

using namespace std;

// Global Variables
string most_recent_data;
bool close_machinec = false;
mutex dataMutex;

// Helper Function to Extract Last Line
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

// Function for Receiving Data from machine A
void receiveDataFromMachineA(SOCKET machinea_socket)
{
    while (true)
    {
        char data_buffer[1024] = {};
        int data_valread = recv(machinea_socket, data_buffer, 1024, 0);

        if (data_valread > 0)
        {
            string received_data(data_buffer);
            string last_received_data = getLastLine(received_data);

            // Update most_recent_data
            lock_guard<mutex> guard(dataMutex);
            most_recent_data = last_received_data;
            cout << "Last received line from machine A: " << last_received_data << endl;
        }
        else
        {
            // No more data from machine A
            lock_guard<mutex> guard(dataMutex);
            close_machinec = true;
            break;
        }
    }
}

// Function for Sending Data to machine C
void sendDataToMachineC(SOCKET machinec_socket)
{
    while (true)
    {
        char data_buffer[1024] = {};
        int data_valread = recv(machinec_socket, data_buffer, 1024, 0);
        string received_data(data_buffer);
        // cout << received_data <<endl;

        if (data_valread > 0 && received_data == "Request for data")
        {

            lock_guard<mutex> guard(dataMutex);

            if (close_machinec)
            {
                // Send message to machine C to exit from the loop
                send(machinec_socket, "close program now", strlen("close program now"), 0);
                cout << "Closing signal sent to machine C.\n";
                break;
            }
            // Send most_recent_data to machine C
            send(machinec_socket, most_recent_data.c_str(), most_recent_data.length(), 0);
            cout << "Sent most recent data to machine C.\n";
        }
    }
}

// Handshake with Machine A
string machineAHandshake(SOCKET machinea_socket)
{
    char buffer_a[1024] = {};
    int valread_a = recv(machinea_socket, buffer_a, 1024, 0);

    if (valread_a > 0)
    {
        string received_message_a(buffer_a);
        cout << "[LOG] : Received message from machine A: " << received_message_a << endl;

        if (received_message_a == "this is motion server")
        {
            cout << "[LOG] : Sending response to machine A.\n";
            send(machinea_socket, "hi motion server", strlen("hi motion server"), 0);
            return "success";
        }
        else
        {
            cerr << "[ERROR] : Invalid message received from machine A.\n";
            return "failure";
        }
    }
    else
    {
        cerr << "[ERROR] : No message received from machine A.\n";
        return "failure";
    }
}

// Handshake with Machine C
string machineCHandshake(SOCKET machinec_socket)
{
    char buffer_c[1024] = {};
    int valread_c = recv(machinec_socket, buffer_c, 1024, 0);

    if (valread_c > 0)
    {
        string received_message_c(buffer_c);
        cout << "[LOG] : Received message from machine C: " << received_message_c << endl;

        if (received_message_c == "this is the host")
        {
            // Send response to Machine C
            send(machinec_socket, "hello host!", strlen("hello host!"), 0);
            return "success";
        }
        else
        {
            cerr << "[ERROR] : Invalid message received from machine C.\n";
            return "failure";
        }
    }
    else
    {
        cerr << "[ERROR] : No message received from machine C.\n";
        return "failure";
    }
}

int main()
{
    int PORT_B = 55000; // Port for machine A
    int PORT_C = 50000; // Port for machine C

    // Winsock initialization code
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cerr << "[ERROR] : WSAStartup failed\n";
        return 1;
    }

    SOCKET server_socket_descriptor, machinea_socket, machinec_socket;
    struct sockaddr_in server_address, machinea_address, machinec_address;
    int server_address_length = sizeof(server_address);

    // Setup socket for machine A
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

    cout << "[LOG] : Waiting for incoming connections from machine A on PORT_B...\n";

    // Accept connection from machine A
    machinea_socket = accept(server_socket_descriptor, (struct sockaddr *)&machinea_address, &server_address_length);
    if (machinea_socket == INVALID_SOCKET)
    {
        cerr << "[ERROR] : Accept from machine A failed.\n";
        closesocket(server_socket_descriptor);
        WSACleanup();
        return 1;
    }

    cout << "[LOG] : Connection accepted from machine A.\n";

    // Setup socket for machine C
    SOCKET machinec_server_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (machinec_server_socket_descriptor == INVALID_SOCKET)
    {
        cerr << "[ERROR] : Machine C socket creation failed.\n";
        closesocket(machinea_socket);
        closesocket(server_socket_descriptor);
        WSACleanup();
        return 1;
    }

    server_address.sin_port = htons(PORT_C);

    if (bind(machinec_server_socket_descriptor, (struct sockaddr *)&server_address, sizeof(server_address)) == SOCKET_ERROR)
    {
        cerr << "[ERROR] : Bind for machine C failed: " << WSAGetLastError() << "\n";
        closesocket(machinec_server_socket_descriptor);
        closesocket(machinea_socket);
        closesocket(server_socket_descriptor);
        WSACleanup();
        return 1;
    }

    if (listen(machinec_server_socket_descriptor, 3) == SOCKET_ERROR)
    {
        cerr << "[ERROR] : Listen for machine C failed.\n";
        closesocket(machinec_server_socket_descriptor);
        closesocket(machinea_socket);
        closesocket(server_socket_descriptor);
        WSACleanup();
        return 1;
    }

    cout << "[LOG] : Waiting for incoming connections on PORT_C for machine C...\n";

    machinec_socket = accept(machinec_server_socket_descriptor, (struct sockaddr *)&machinec_address, &server_address_length);
    if (machinec_socket == INVALID_SOCKET)
    {
        cerr << "[ERROR] : Accept from machine C failed.\n";
        closesocket(machinec_server_socket_descriptor);
        closesocket(machinea_socket);
        closesocket(server_socket_descriptor);
        WSACleanup();
        return 1;
    }

    cout << "[LOG] : Connection accepted from machine C.\n";

    // Handshake with Machine A
    string machineAHandshakeResult = machineAHandshake(machinea_socket);
    if (machineAHandshakeResult == "success")
    {
        cout << "[LOG] : Handshake with machine A successful.\n";

        // Handshake with Machine C
        string machineCHandshakeResult = machineCHandshake(machinec_socket);
        if (machineCHandshakeResult == "success")
        {
            cout << "[LOG] : Handshake with machine C successful.\n";

            // Send response to Machine A
            send(machinea_socket, "handshaked with machinec", strlen("handshaked with machinec"), 0);
        }
        else
        {
            cerr << "[ERROR] : Handshake with machine C failed.\n";
            return 1;
        }
    }
    else
    {
        cerr << "[ERROR] : Handshake with machine A failed.\n";
        return 1;
    }

    // Create threads for handling data transmission
    thread receiveThread(receiveDataFromMachineA, machinea_socket);
    thread sendThread(sendDataToMachineC, machinec_socket);

    // Join threads to ensure they complete their tasks
    receiveThread.join();
    sendThread.join();
    cout << "Transmission End" <<endl;

    // Cleanup and exit
    closesocket(server_socket_descriptor);
    closesocket(machinea_socket);
    closesocket(machinec_socket);
    closesocket(machinec_server_socket_descriptor);
    WSACleanup();

    return 0;
}