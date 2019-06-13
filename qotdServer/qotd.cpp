/**************************************************************
*	Michael Cruz
*	April 4, 2019
*	This server communicates with a client using UDP.
***************************************************************/

#include <iostream>
#include <fstream>
#include <WinSock2.h>
#include "qotd.h"
#pragma comment (lib,"ws2_32.lib")
using namespace std;

int main() {
	bool shutdown_server = false;
	char server_ip[MAX_SERVER_NAME];
	char broadcast_ip[MAX_SERVER_NAME];
	char server_port[MAX_PORT];
	strcpy_s(server_port, "17");
	char remote_ip[MAX_HOSTNAME];
	char remote_port[MAX_PORT];

	WORD wVersionRequired = 0x0202;
	WSADATA wsaData;
	int err = WSAStartup(wVersionRequired, &wsaData);
	if (err) {
		cout << "Unable to initialize Windows Socket Library." << endl;
		return;
	}

	SOCKET s = passivesock("qotd", "udp");

	if (s != INVALID_SOCKET) {
		getIPAddressInfo(server_ip, broadcast_ip);
		cout << timestamp() << " - Started qotd service on " << server_ip << ":" << server_port << endl;
		
		while (!shutdown_server) {
			int status = wait(s, 1, 0);
			
			if (status > 0 ) {
				char recv_buffer[MAX_RECV_BUFFER] = "";
				int numBytesReceived = UDP_recv(s, recv_buffer, MAX_RECV_BUFFER, remote_ip, remote_port);
				
				if (numBytesReceived > 0) {
					cout << timestamp() << " - Datagram received from: " << remote_ip << ":" << remote_port << endl;

					if (_stricmp(recv_buffer, "sendQOTD") == 0) {
						cout << timestamp() << " - Received command: ";
						recv_buffer[numBytesReceived] = '\0';
						cout << recv_buffer << endl;
						
						ifstream fin;
						fin.open("qotd.txt");
						char send_buffer[MAX_SEND_BUFFER] = "";

						if (fin.is_open()) {
							while (!fin.eof()) {
								fin.getline(send_buffer, MAX_SEND_BUFFER - 1);
								if (_stricmp(send_buffer, "") == 0) { strcpy_s(send_buffer, "\n\n"); }
								int send = UDP_send(s, send_buffer, (int)strlen(send_buffer) + 1, remote_ip, remote_port);
							}

							fin.close();
							cout << timestamp() << " - Sent file: qotd.txt to " << remote_ip << ":" << remote_port << endl;
						}
					}
					else if (_stricmp(recv_buffer, "shutdown") == 0 && _stricmp(server_ip, remote_ip) == 0) {
						cout << timestamp() << "Service qotd will be shutdown" << endl;
						shutdown_server = true;
					}
					else if (_stricmp(recv_buffer, "shutdown") == 0 && _stricmp(server_ip, remote_ip) != 0) {
						cout << timestamp() << " - SHUTDOWN request denied" << endl;
					}
					else {
						cout << timestamp() << " - Ignored command: ";
						recv_buffer[numBytesReceived] = '\0';
						cout << recv_buffer << endl;
					}
				}
			}
		}

		closesocket(s);
	}

	WSACleanup();
	return 0;
}