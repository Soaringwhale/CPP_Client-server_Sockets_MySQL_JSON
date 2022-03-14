#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream> 
#include <cstdio> 
#include <cstring> 
#include <winsock2.h> 

#include <nlohmann\json.hpp>             // for using json
#pragma comment(lib, "WS2_32.lib")        
using json = nlohmann::json;

DWORD WINAPI clientReceive(LPVOID lpParam) {           //Получение данных от сервера
	char buffer[1024] = { 0 };
	SOCKET server = *(SOCKET*)lpParam;
	while (true) {
		if (recv(server, buffer, sizeof(buffer), 0) == SOCKET_ERROR) {                             
			std::cout << "recv function failed, error: " << WSAGetLastError() << std::endl;
			return -1;
		}
		if (strcmp(buffer, "exit\n") == 0) {
			std::cout << "Server disconnected.\n";
			return 1;
		}
		std::cout << "Recieved: " << buffer << std::endl;
		memset(buffer, 0, sizeof(buffer));
	}
	return 1;
}

DWORD WINAPI clientSend(LPVOID lpParam) {                  //Отправка данных на сервер
	std::string s, str;
	int command_type = 0;
	json j;

	SOCKET server = *(SOCKET*)lpParam;
	while (true)
	{
		while (true) {
			std::cout << "Enter command type. 0 - SQL Request, 1 - command to server\n\n";
			std::cin >> command_type;
			if (!std::cin || command_type < 0 || command_type > 1) {
				std::cout << "Please enter only 0 or 1\n";
				std::cin.clear();                                                 // сброс флагов ошибок потока
				std::cin.ignore(std::cin.rdbuf()->in_avail());                     // очищение содержимого потока 
			}
			else break;
		}

		std::cin.clear();
		std::cin.ignore(std::cin.rdbuf()->in_avail());

		if (command_type == 0)  std::cout << "Enter SQL Request. For example: select * from registry; \n";
		else if (command_type == 1)  std::cout << "Enter command to server. Type \"quit\" to close connection\n";

		std::getline(std::cin, str);
		j["command_type"] = command_type;
		j["command"] = str;
		s = j.dump();                                                   // превращаем json в строку (сериализация)
		if (send(server, s.c_str(), s.length(), 0) == SOCKET_ERROR) {                         // отправка серверу, 3-й параметр - кол-во символов для отправки
			std::cout << "send failed with error: " << WSAGetLastError() << std::endl;
			return -1;
		}
		if (j["command_type"] == 1 && j["command"] == "quit") {
			std::cout << "Disconnected...\n";        
			break;
		}
	}
	return 1;
}

int main() {
	WSADATA WSAData;
	SOCKET server;
	SOCKADDR_IN addr;
	WSAStartup(MAKEWORD(2, 0), &WSAData);
	if ((server = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		std::cout << "Socket creation failed with error: " << WSAGetLastError() << std::endl;
		return -1;
	}

	addr.sin_addr.s_addr = inet_addr("127.0.0.1");        //коннект к серверу
	addr.sin_family = AF_INET;
	addr.sin_port = htons(5555);                          //порт
	if (connect(server, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		std::cout << "Server connection failed with error: " << WSAGetLastError() << std::endl;
		return -1;
	}
	std::cout << "Connected to server\n";

	DWORD tid;
	HANDLE t1 = CreateThread(NULL, 0, clientReceive, &server, 0, &tid);
	if (t1 == NULL) std::cout << "Thread creation error: " << GetLastError();
	HANDLE t2 = CreateThread(NULL, 0, clientSend, &server, 0, &tid);
	if (t2 == NULL) std::cout << "Thread creation error: " << GetLastError();

	WaitForSingleObject(t1, INFINITE);
	WaitForSingleObject(t2, INFINITE);
	closesocket(server);
	WSACleanup();
}