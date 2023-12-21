#include "mars.h"
#include <Ws2tcpip.h>
#include <string>
#include <chrono>
#include <thread>
#include <sstream>
#include <fstream>
#include <vector>

using namespace std;

// Function for setting up connection and performing handshake
SOCKET setupConnection()
{
    int PORT_C = 50000; // Port for machinec to connect to machineb

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cerr << "[ERROR] : WSAStartup failed\n";
        return INVALID_SOCKET; // Indicate connection failure
    }

    SOCKET machinec_socket;
    struct sockaddr_in machinec_address;

    machinec_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (machinec_socket == INVALID_SOCKET)
    {
        cerr << "[ERROR] : Socket failed.\n";
        WSACleanup();
        return INVALID_SOCKET; // Indicate connection failure
    }

    machinec_address.sin_family = AF_INET;
    machinec_address.sin_port = htons(PORT_C); // Port for machineb
    const char *machineb_ip = "127.0.0.1";     // Machine B's IP address
    inet_pton(AF_INET, machineb_ip, &machinec_address.sin_addr);

    if (connect(machinec_socket, (struct sockaddr *)&machinec_address, sizeof(machinec_address)) == SOCKET_ERROR)
    {
        cerr << "[ERROR] : Connection attempt to machineb failed.\n";
        closesocket(machinec_socket);
        WSACleanup();
        return INVALID_SOCKET; // Indicate connection failure
    }

    cout << "[LOG] : Connected to machineb.\n";

    // Send message to machineb for handshake
    const char *handshake_msg = "this is MARS";
    send(machinec_socket, handshake_msg, strlen(handshake_msg), 0);
    cout << "[LOG] : Sent handshake request to machineb.\n";

    char buffer[1024] = {};
    int valread = recv(machinec_socket, buffer, 1024, 0);

    if (valread > 0)
    {
        string received_message(buffer);
        cout << "[LOG] : Received handshake response from machineb: " << received_message << endl;

        if (received_message != "MARS ready to request data")
        {
            cout << "[ERROR] : Invalid response received from machineb. Closing connection.\n";
            closesocket(machinec_socket);
            WSACleanup();
            return INVALID_SOCKET; // Indicate connection failure
        }
    }
    else
    {
        cout << "[ERROR] : No response received from machineb.\n";
        closesocket(machinec_socket);
        WSACleanup();
        return INVALID_SOCKET; // Indicate connection failure
    }

    return machinec_socket; // Return the connected socket
}

// Function for continuous communication after handshake
void marsConnect(SOCKET machinec_socket)
{
    while (true)
    {
        // Send message to machineb to request data
        const char *data_request = "Request for data";
        send(machinec_socket, data_request, strlen(data_request), 0);
        cout << "[LOG] : Sent data request to machineb.\n";

        // Receive response from machineb (most recent data)
        char data_buffer[1024] = {};
        int data_valread = recv(machinec_socket, data_buffer, 1024, 0);

        if (data_valread > 0)
        {
            // get data from machineb and check if received data is an exit signal
            string received_data(data_buffer);
            cout << "[LOG] : Received most recent data from machineb: " << received_data << endl;

            // if (received_data == "close program now")
            // {
            //     cout << "[LOG] : Received exit signal from machineb. Exiting loop.\n";
            //     break; // Exit the while loop
            // }

            // Split the received_data by comma to get the numbers
            stringstream ss(received_data);
            string token;
            vector<string> numbers;

            while (getline(ss, token, ','))
            {
                numbers.push_back(token);
            }

            // Check if length is 8 (first one is datetime, last one is just empty)
            // [x, y, z, pitch(x), yaw(y), roll(z)]
            cout << "I get " << numbers.size() << " numbers" << endl;
            cout << "The first number is " << numbers[0] << endl;
            cout << "The last number is " << numbers[7] << endl;

            // Initialize variables
            double m_angles[3];
            double m_translations[3];

            m_translations[0] = std::stod(numbers[1]);
            m_translations[1] = std::stod(numbers[2]);
            m_translations[2] = std::stod(numbers[3]);
            m_angles[0] = std::stod(numbers[4]);
            m_angles[1] = std::stod(numbers[5]);
            m_angles[2] = std::stod(numbers[6]);

            cout << "x: " << m_translations[0] << endl;
            cout << "y: " << m_translations[1] << endl;
            cout << "z: " << m_angles[2] << endl;
            cout << "pitch(x): " << m_angles[0] << endl;
            cout << "yaw(y): " << m_angles[1] << endl;
            cout << "roll(z): " << m_angles[2] << endl;
            cout << endl;
        }
        else
        {
            cout << "[ERROR] : No response received from machineb.\n";
        }

        // Wait for 1 seconds before sending the next request
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // After loop exits, close the socket
    closesocket(machinec_socket);
    WSACleanup();
}