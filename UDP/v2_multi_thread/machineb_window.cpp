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
int PORT_B = 55000; // Port for machine A
int PORT_C = 50000; // Port for machine C

// Helper Function to Extract Last Line
string getLastLine(const string &receivedData) {
    istringstream stream(receivedData);
    string lastLine, line;

    while (getline(stream, line)) {
        lastLine = line;
    }

    return lastLine;
}

// Function for Receiving Data from machine A
void receiveDataFromMachineA(SOCKET machinea_socket) {
    while (true) {
        char data_buffer[1024] = {};
        sockaddr_in sender_addr;  
        int sender_addr_len = sizeof(sender_addr);

        int data_valread = recvfrom(machinea_socket, data_buffer, 1024, 0, (struct sockaddr *)&sender_addr, &sender_addr_len);

        if (data_valread > 0) {
            string received_data(data_buffer);

            if (received_data == "transmission complete") {
                // No more data from machine A
                lock_guard<mutex> guard(dataMutex);
                close_machinec = true;
                break;
            } else {
                string last_received_data = getLastLine(received_data);
                lock_guard<mutex> guard(dataMutex);
                most_recent_data = last_received_data;
                cout << "Last received line from machine A: " << last_received_data << endl;
            }
        }
    }
}

// Function for Sending Data to machine C
void sendDataToMachineC(SOCKET machinec_socket) {
    while (true) {
        char data_buffer[1024] = {};
        sockaddr_in sender_addr;  
        int sender_addr_len = sizeof(sender_addr);

        int data_valread = recvfrom(machinec_socket, data_buffer, 1024, 0, (struct sockaddr *)&sender_addr, &sender_addr_len);
        string received_data(data_buffer);
        // cout << received_data <<endl;

        if (data_valread > 0 && received_data == "Request for data") {

            lock_guard<mutex> guard(dataMutex);

            if (close_machinec) {
                // Send message to machine C to exit from the loop
                sendto(machinec_socket, "close program now", strlen("close program now"), 0, (struct sockaddr *)&sender_addr, sender_addr_len);
                cout << "Closing signal sent to machine C.\n";
                break;
            }
            // Send most_recent_data to machine C
            sendto(machinec_socket, most_recent_data.c_str(), most_recent_data.length(), 0, (struct sockaddr *)&sender_addr, sender_addr_len);
            cout << "Sent most recent data to machine C.\n";
        }
    }
}

string machineAHandshake(SOCKET machinea_socket, sockaddr* machinea_addr, int machinea_addr_len) {
    char buffer_a[1024] = {};
    int valread_a = recvfrom(machinea_socket, buffer_a, 1024, 0, machinea_addr, &machinea_addr_len);

    if (valread_a > 0) {
        string received_message_a(buffer_a);
        cout << "[LOG] : Received message from machine A: " << received_message_a << endl;

        if (received_message_a == "this is motion server") {
            cout << "[LOG] : Sending response to machine A.\n";
            sendto(machinea_socket, "hi motion server", strlen("hi motion server"), 0, machinea_addr, machinea_addr_len);
            return "success";
        } else {
            cerr << "[ERROR] : Invalid message received from machine A.\n";
            return "failure";
        }
    } else {
        cerr << "[ERROR] : No message received from machine A.\n";
        return "failure";
    }
}

// Handshake with Machine C
string machineCHandshake(SOCKET machinec_socket) {
    char buffer_c[1024] = {};
    sockaddr_in sender_addr;  
    int sender_addr_len = sizeof(sender_addr);

    int valread_c = recvfrom(machinec_socket, buffer_c, 1024, 0, (struct sockaddr *)&sender_addr, &sender_addr_len);

    if (valread_c > 0) {
        string received_message_c(buffer_c);
        cout << "[LOG] : Received message from machine C: " << received_message_c << endl;

        if (received_message_c == "this is the host") {
            // Send response to Machine C
            sendto(machinec_socket, "hello host!", strlen("hello host!"), 0, (struct sockaddr *)&sender_addr, sender_addr_len);
            return "success";
        } else {
            cerr << "[ERROR] : Invalid message received from machine C.\n";
            return "failure";
        }
    } else {
        cerr << "[ERROR] : No message received from machine C.\n";
        return "failure";
    }
}

int main() {
    // Winsock initialization code
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "[ERROR] : WSAStartup failed\n";
        return 1;
    }

    SOCKET machinea_socket, machinec_socket;
    struct sockaddr_in machinea_address, machinec_address;

    // Setup socket for machine A
    machinea_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (machinea_socket == INVALID_SOCKET) {
        cerr << "[ERROR] : Machine A socket creation failed.\n";
        WSACleanup();
        return 1;
    }

    machinea_address.sin_family = AF_INET;
    machinea_address.sin_addr.s_addr = INADDR_ANY;
    machinea_address.sin_port = htons(PORT_B);

    if (bind(machinea_socket, (struct sockaddr *)&machinea_address, sizeof(machinea_address)) == SOCKET_ERROR) {
        cerr << "[ERROR] : Bind for machine A failed: " << WSAGetLastError() << "\n";
        closesocket(machinea_socket);
        WSACleanup();
        return 1;
    }

    cout << "[LOG] : Waiting for incoming messages from machine A on PORT_B...\n";

    // Setup socket for machine C
    machinec_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (machinec_socket == INVALID_SOCKET) {
        cerr << "[ERROR] : Machine C socket creation failed.\n";
        closesocket(machinea_socket);
        WSACleanup();
        return 1;
    }

    machinec_address.sin_family = AF_INET;
    machinec_address.sin_addr.s_addr = INADDR_ANY;
    machinec_address.sin_port = htons(PORT_C);

    if (bind(machinec_socket, (struct sockaddr *)&machinec_address, sizeof(machinec_address)) == SOCKET_ERROR) {
        cerr << "[ERROR] : Bind for machine C failed: " << WSAGetLastError() << "\n";
        closesocket(machinec_socket);
        closesocket(machinea_socket);
        WSACleanup();
        return 1;
    }

    cout << "[LOG] : Waiting for incoming messages on PORT_C for machine C...\n";

    // Handshake with Machine A
    string machineAHandshakeResult = machineAHandshake(machinea_socket, (sockaddr*)&machinea_address, sizeof(machinea_address));
    if (machineAHandshakeResult == "success") {
        cout << "[LOG] : Handshake with machine A successful.\n";

        // Handshake with Machine C
        string machineCHandshakeResult = machineCHandshake(machinec_socket);
        if (machineCHandshakeResult == "success") {
            cout << "[LOG] : Handshake with machine C successful.\n";

            // Send response to Machine A
            sendto(machinea_socket, "handshaked with machinec", strlen("handshaked with machinec"), 0, (struct sockaddr *)&machinec_address, sizeof(machinec_address));
        } else {
            cerr << "[ERROR] : Handshake with machine C failed.\n";
            return 1;
        }
    } else {
        cerr << "[ERROR] : Handshake with machine A failed.\n";
        return 1;
    }

    // Create threads for handling data transmission
    thread receiveThread(receiveDataFromMachineA, machinea_socket);
    thread sendThread(sendDataToMachineC, machinec_socket);

    // Join threads to ensure they complete their tasks
    receiveThread.join();
    sendThread.join();
    cout << "Transmission End" << endl;

    // Cleanup and exit
    closesocket(machinea_socket);
    closesocket(machinec_socket);
    WSACleanup();

    return 0;
}