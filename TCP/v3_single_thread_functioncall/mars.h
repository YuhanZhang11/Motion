#ifndef MARS_H
#define MARS_H

#include <winsock2.h>
#include <iostream>

SOCKET setupConnection();
void marsConnect(SOCKET machinec_socket);

#endif // MARS_H