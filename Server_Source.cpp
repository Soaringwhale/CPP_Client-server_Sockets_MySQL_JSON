#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream> 
#include <cstdio> 
#include <cstring> 
#include <winsock2.h> 

#include <conio.h>
#include <stdlib.h>
#include <windows.h>        //  windows.h подключать до mysql.h
#include <mysql.h>
#include <stdio.h>
#include <nlohmann\json.hpp>      // for using json

#include<thread>
#include<chrono>

#pragma comment(lib, "WS2_32.lib")       

using json = nlohmann::json;

 MYSQL_RES* makeRequestToDB(std::string  sqlrequest)     // запрос к БД от сервера, принимает сам запрос
{
	const char* request = sqlrequest.c_str();      
	MYSQL* conn;                         // создаем дескриптор соединения
	conn = mysql_init(NULL);             //  инициализируем соединение
	if (conn == NULL) {     
		fprintf(stderr, "Error: can't create MySQL-descriptor\n");      
	}
	if (!mysql_real_connect(conn, "localhost", "root", "hhhesoyam1", "mytest_db2", NULL, NULL, 0))     // пытаемся подключиться к бд
	{
		fprintf(stderr, "Error: can't connect to database %s\n", mysql_error(conn));
	}
	else  fprintf(stdout, "Connected to database..\n");                    

	mysql_set_character_set(conn, "utf8");                                              //  устанавливаем кодировку  utf8
	std::cout << "characterset: " << mysql_character_set_name(conn) << std::endl;

	MYSQL_RES* result;                       // для результата выполнения запроса 
	
	if (mysql_query(conn, request) > 0) {         // выполнятся SQL запрос  
		printf("%s", mysql_error(conn));
	}
	if (result = mysql_store_result(conn)) {       //  массив всех строк результирующего запроса (результирующая таблица) возвращается серверу
		mysql_close(conn);                       
		return result;
	}
	else {
		fprintf(stderr, "%s\n", mysql_error(conn));
		mysql_close(conn);                               
		return 0;       
	}
	//mysql_free_result(result);   //  очистка результата   
}

DWORD WINAPI serverReceive(LPVOID lpParam)   //Получение данных от клиента, принимает указатель на сокет клиента (LPVOID - указатель типа void, позже будет приведет к SOCKET*)
{
	char buffer[1024] = { 0 };                
	SOCKET client = *(SOCKET*)lpParam;        

	while (true)  
	{
		if (recv(client, buffer, sizeof(buffer), 0) == SOCKET_ERROR)      // получаем данные от клиента 
		{
			std::cout << "recv function failed with error " << WSAGetLastError() << std::endl;
			return -1;
		}
		std::string str(buffer);
		json j2 = json::parse(str);
		std::cout << "Client sent command: " << j2["command"] << std::endl;
		memset(buffer, 0, sizeof(buffer));          
		if (j2["command_type"] == 1 && j2["command"] == "quit") {           //если тип команды 1, это команда серверу. quit - закрыть соединение
			std::cout << "Client Disconnected.\n";
			break;                                    
		}
		MYSQL_RES* result = nullptr;
		if (j2["command_type"] == 0) {                      // если тип команды 0 - сервер делает запрос к бд
			result = makeRequestToDB(j2["command"]);
		}
		if (result == 0) {
			std::string res = "incorrect SQL-request. Enter the type of the new command (0 or 1)";
			if (send(client, res.c_str(), res.length(), 0) == SOCKET_ERROR) {                        // если makeRequestToDB вернул 0, сообщаем клиенту, что SQL-запрос составлен не верно
				continue;
			}
			else continue;
		} 
		MYSQL_ROW row;                    //  массив значений текущей строки
		int i = 0;
		std::string res = "";
		while (row = mysql_fetch_row(result)) {                     //  извлечение следующей строки из результирующего набора
			for (i = 0; i < mysql_num_fields(result); i++) {    
				res = res + row[i] + ",";                       
			}  
			res.back() = '\n';
			if (send(client, res.c_str(), res.length(), 0) == SOCKET_ERROR) {                                  // отправляем получившуюся строку клиенту
				std::cout << "answer from server failed with error " << WSAGetLastError() << std::endl;
				continue;
			}
			else res = "";
			std::this_thread::sleep_for(std::chrono::milliseconds(300));
		}
	}
	return 1;
}

DWORD WINAPI serverSend(LPVOID lpParam)       //  отправка сообщений клиенту вручную
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

	WSADATA WSAData;               // структура с информацией о версии сокетов и пр.
	SOCKET server, client;                //Сокеты сервера и клиента
	SOCKADDR_IN serverAddr, clientAddr;        // Адреса  (используется для привязки через bind(), содержит семейство, адрес и порт)

	WSAStartup(MAKEWORD(2, 0), &WSAData);         // функция запуска сокетов, принимает версию сокетов и WSADATA
	server = socket(AF_INET, SOCK_STREAM, 0);       // инициализируем сокет сервера функцией socket(), при успехе возвращается дескриптор сокета
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

	if (listen(server, 0) == SOCKET_ERROR) {           //Если не удалось получить запрос
		std::cout << "Listen function failed with error:" << WSAGetLastError() << std::endl;
		return -1;
	}
	std::cout << "Listening for incoming connections....\n";
    
	int clientAddrSize = sizeof(clientAddr);        
	if ((client = accept(server, (SOCKADDR*)&clientAddr, &clientAddrSize)) != INVALID_SOCKET) {        		//Если соединение установлено
		std::cout << "Client connected to server\n";
		std::cout << "You can type a message for client here, or type \"quit\" for disconnect\n";

		DWORD tid;       // Идентификатор потока 

		HANDLE t1 = CreateThread(NULL, 0, serverReceive, &client, 0, &tid);    //Создание потока для фнк принятия (вызов serverReceive), вернет дескриптор потока в t1
		if (t1 == NULL) {
			std::cout << "Thread Creation Error: " << WSAGetLastError() << std::endl;
		}
		HANDLE t2 = CreateThread(NULL, 0, serverSend, &client, 0, &tid);         //Создание потока для отправки данных (вызов serverSend), вернет дескриптор потока в t2
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