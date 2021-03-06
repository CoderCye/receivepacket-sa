#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <cstdlib>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

#define HOST "37.230.162.117"              // Hostname
#define PORT 7777                          // Port
#define RESPONCE_HEADER_SIZE 11
#define RESPONCE_STR_MAXLEN 32


#pragma pack(push,1)

typedef struct samp_query_s
{
	char header[4];
	unsigned int ip;
	short port;
	char code;
} samp_query_t;

typedef struct iresponce_s
{
	char has_password;
	short players;
	short maxplayers;

	char hostname[RESPONCE_STR_MAXLEN];
	char gamemode[RESPONCE_STR_MAXLEN];
	char mapname[RESPONCE_STR_MAXLEN];
} iresponce_t;

#pragma pack(pop)

int main() {
	WSAData wsa;
	SOCKADDR_IN sAddr;
	SOCKET sock;
	samp_query_t query;
	char buf[1024];
	int received;
	unsigned int len;
	char* pos, * end;
	iresponce_t responce;

	if (FAILED(WSAStartup(MAKEWORD(1, 1), &wsa))) return 1;

	inet_pton(AF_INET, HOST, &sAddr.sin_addr);
	sAddr.sin_port = htons(PORT);
	sAddr.sin_family = AF_INET;
	
	if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR) return 1;

	memcpy(query.header, "SAMP", 4);
	query.ip = sAddr.sin_addr.S_un.S_addr;
	query.port = sAddr.sin_port;
	query.code = 'i';

	sendto(sock, (const char*)& query, sizeof(query), 0, (sockaddr*)& sAddr, sizeof(sAddr)); // Send Full Packet
	received = recv(sock, buf, sizeof(buf), 0); // Read Responce from Server
	closesocket(sock);
	WSACleanup();
	
	if (received < (RESPONCE_HEADER_SIZE + 1 + 2 + 2 + 4))
		return 1;

	end = buf + received;
	pos = buf + RESPONCE_HEADER_SIZE;
	// Players and Pass
	memcpy(&responce, pos, 5);
	pos += 5;
	
	// HostName
	len = *(unsigned int*)pos;
	pos += 4;
	if (pos + len + 4 >= end)
		return 1;
	memcpy(&responce.hostname, pos, len = min(len, RESPONCE_STR_MAXLEN - 1));
	responce.hostname[len] = '\0';
	pos += len;
	
	// GameMode
	pos += 4;
	if (pos + len + 4 >= end)
		return 1;
	len = *(unsigned int*)pos;
	memcpy(&responce.gamemode, pos, len = min(len, RESPONCE_STR_MAXLEN - 1));
	responce.gamemode[len] = '\0';
	pos += len;
	
	// MapName
	pos += 4;
	/*
	if (pos + len > end)
		return 1;
	*/
	len = *(unsigned int*)pos;
	memcpy(&responce.mapname, pos, len = min(len, RESPONCE_STR_MAXLEN - 1));
	responce.mapname[len] = '\0';

	// Print Information
	std::cout << responce.hostname << std::endl;
	std::cout << "Players: " << responce.players << " / " << responce.maxplayers << "\x0a";
	std::cout << "Gamemode: " << responce.gamemode << std::endl;
	if (responce.has_password == 0)
		std::cout << "Password: None" << std::endl;

	return EXIT_SUCCESS;
}