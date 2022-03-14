#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream> 
#include <cstdio> 
#include <cstring> 
#include <winsock2.h> 

#include <conio.h>
#include <stdlib.h>
#include <windows.h>        //  windows.h ���������� �� mysql.h
#include <mysql.h>
#include <stdio.h>
#include <nlohmann\json.hpp>      // for using json

#include<thread>
#include<chrono>

#pragma comment(lib, "WS2_32.lib")       

using json = nlohmann::json;

 MYSQL_RES* makeRequestToDB(std::string  sqlrequest)     // ������ � �� �� �������, ��������� ��� ������
{
	const char* request = sqlrequest.c_str();      
	MYSQL* conn;                         // ������� ���������� ����������
	conn = mysql_init(NULL);             //  �������������� ����������
	if (conn == NULL) {     
		fprintf(stderr, "Error: can't create MySQL-descriptor\n");      
	}
	if (!mysql_real_connect(conn, "localhost", "root", "hhhesoyam1", "mytest_db2", NULL, NULL, 0))     // �������� ������������ � ��
	{
		fprintf(stderr, "Error: can't connect to database %s\n", mysql_error(conn));
	}
	else  fprintf(stdout, "Connected to database..\n");                    

	mysql_set_character_set(conn, "utf8");                                              //  ������������� ���������  utf8
	std::cout << "characterset: " << mysql_character_set_name(conn) << std::endl;

	MYSQL_RES* result;                       // ��� ���������� ���������� ������� 
	
	if (mysql_query(conn, request) > 0) {         // ���������� SQL ������  
		printf("%s", mysql_error(conn));
	}
	if (result = mysql_store_result(conn)) {       //  ������ ���� ����� ��������������� ������� (�������������� �������) ������������ �������
		mysql_close(conn);                       
		return result;
	}
	else {
		fprintf(stderr, "%s\n", mysql_error(conn));
		mysql_close(conn);                               
		return 0;       
	}
	//mysql_free_result(result);   //  ������� ����������   
}

DWORD WINAPI serverReceive(LPVOID lpParam)   //��������� ������ �� �������, ��������� ��������� �� ����� ������� (LPVOID - ��������� ���� void, ����� ����� �������� � SOCKET*)
{
	char buffer[1024] = { 0 };                
	SOCKET client = *(SOCKET*)lpParam;        

	while (true)  
	{
		if (recv(client, buffer, sizeof(buffer), 0) == SOCKET_ERROR)      // �������� ������ �� ������� 
		{
			std::cout << "recv function failed with error " << WSAGetLastError() << std::endl;
			return -1;
		}
		std::string str(buffer);
		json j2 = json::parse(str);
		std::cout << "Client sent command: " << j2["command"] << std::endl;
		memset(buffer, 0, sizeof(buffer));          
		if (j2["command_type"] == 1 && j2["command"] == "quit") {           //���� ��� ������� 1, ��� ������� �������. quit - ������� ����������
			std::cout << "Client Disconnected.\n";
			break;                                    
		}
		MYSQL_RES* result = nullptr;
		if (j2["command_type"] == 0) {                      // ���� ��� ������� 0 - ������ ������ ������ � ��
			result = makeRequestToDB(j2["command"]);
		}
		if (result == 0) {
			std::string res = "incorrect SQL-request. Enter the type of the new command (0 or 1)";
			if (send(client, res.c_str(), res.length(), 0) == SOCKET_ERROR) {                        // ���� makeRequestToDB ������ 0, �������� �������, ��� SQL-������ ��������� �� �����
				continue;
			}
			else continue;
		} 
		MYSQL_ROW row;                    //  ������ �������� ������� ������
		int i = 0;
		std::string res = "";
		while (row = mysql_fetch_row(result)) {                     //  ���������� ��������� ������ �� ��������������� ������
			for (i = 0; i < mysql_num_fields(result); i++) {    
				res = res + row[i] + ",";                       
			}  
			res.back() = '\n';
			if (send(client, res.c_str(), res.length(), 0) == SOCKET_ERROR) {                                  // ���������� ������������ ������ �������
				std::cout << "answer from server failed with error " << WSAGetLastError() << std::endl;
				continue;
			}
			else res = "";
			std::this_thread::sleep_for(std::chrono::milliseconds(300));
		}
	}
	return 1;
}

DWORD WINAPI serverSend(LPVOID lpParam)       //  �������� ��������� ������� �������
{
	char buffer[1024] = { 0 };
	SOCKET client = *(SOCKET*)lpParam;
	while (true) {
		fgets(buffer, 1024, stdin);         
		if (send(client, buffer, sizeof(buffer), 0) == SOCKET_ERROR) {
			std::cout << "send failed with error " << WSAGetLastError() << std::endl;
			return -1;
		}
		if (strcmp(buffer, "quit\n") == 0) {
			std::cout << "Disconnected...bye.\n";
			break;
		}
	}
	return 1;
}

int main() {

	WSADATA WSAData;               // ��������� � ����������� � ������ ������� � ��.
	SOCKET server, client;                //������ ������� � �������
	SOCKADDR_IN serverAddr, clientAddr;        // ������  (������������ ��� �������� ����� bind(), �������� ���������, ����� � ����)

	WSAStartup(MAKEWORD(2, 0), &WSAData);         // ������� ������� �������, ��������� ������ ������� � WSADATA
	server = socket(AF_INET, SOCK_STREAM, 0);       // �������������� ����� ������� �������� socket(), ��� ������ ������������ ���������� ������
	if (server == INVALID_SOCKET) {
		std::cout << "Socket creation failed with error:" << WSAGetLastError() << std::endl;
		return -1;
	}
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(5555);

	if (bind(server, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		std::cout << "Bind function failed with error: " << WSAGetLastError() << std::endl;
		return -1;
	}

	if (listen(server, 0) == SOCKET_ERROR) {           //���� �� ������� �������� ������
		std::cout << "Listen function failed with error:" << WSAGetLastError() << std::endl;
		return -1;
	}
	std::cout << "Listening for incoming connections....\n";
    
	int clientAddrSize = sizeof(clientAddr);        
	if ((client = accept(server, (SOCKADDR*)&clientAddr, &clientAddrSize)) != INVALID_SOCKET) {        		//���� ���������� �����������
		std::cout << "Client connected to server\n";
		std::cout << "You can type a message for client here, or type \"quit\" for disconnect\n";

		DWORD tid;       // ������������� ������ 

		HANDLE t1 = CreateThread(NULL, 0, serverReceive, &client, 0, &tid);    //�������� ������ ��� ��� �������� (����� serverReceive), ������ ���������� ������ � t1
		if (t1 == NULL) {
			std::cout << "Thread Creation Error: " << WSAGetLastError() << std::endl;
		}
		HANDLE t2 = CreateThread(NULL, 0, serverSend, &client, 0, &tid);         //�������� ������ ��� �������� ������ (����� serverSend), ������ ���������� ������ � t2
		if (t2 == NULL) {
			std::cout << "Thread Creation Error: " << WSAGetLastError() << std::endl;
		}

		WaitForSingleObject(t1, INFINITE);      
		WaitForSingleObject(t2, INFINITE);

		closesocket(client);                       
		if (closesocket(server) == SOCKET_ERROR) {   
			std::cout << "Close socket failed with error: " << WSAGetLastError() << std::endl;
			return -1;
		}
		WSACleanup();
	}
}