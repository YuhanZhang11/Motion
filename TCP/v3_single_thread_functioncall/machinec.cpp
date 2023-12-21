#include "mars.h"

int main() {
    // Establish the connection
    SOCKET machinec_socket = setupConnection();

    // Get the data
    if (machinec_socket != INVALID_SOCKET) {
        // If connection is successful, proceed with continuous communication
        marsConnect(machinec_socket);
    } else {
        std::cerr << "[ERROR] : Connection failed. Exiting program.\n";
    }

    return 0;
}