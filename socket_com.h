/* This header is used to encapsulate the communication functions using socket in windows */
/* By Fengxue Zhang in 2017.7.7 */
#pragma once
#pragma warning(disable:4996)
#pragma comment(lib, "ws2_32.lib")
#include<sys/types.h>
#include<winsock2.h>
#include<stdio.h>
#include<stdlib.h>
#include<string>
#include<errno.h>
#include<string.h>
#include<iostream>
using namespace std;

#define CLIENT_IP_ADDR "192.168.199.144"
#define SERVER_IP_ADDR "192.168.199.144"
#define SERVER_PORT 8000
#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE 512

typedef int socklen_t;

struct transmission_obj
{
	transmission_obj(const char* new_ip, const char* new_content) :ip_addr(new_ip), content(new_content) {}
	const char* ip_addr;
	const char* content;
};

string server_receive = "";
bool server_receive_changed = false;

void* server(void* signal = NULL)
{
	/* Initialize WSA environment */
	WSADATA wsa;						// an example to receive the wsa implementation
	WSAStartup(MAKEWORD(2, 2), &wsa);	// use the version 2.2

										/* 创建UDP套接口 */
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));    // initial server_addr as 0
	server_addr.sin_family = AF_INET;            // ipv4
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(SERVER_PORT);

	/* 创建socket */
	int server_socket_fd = socket(AF_INET, SOCK_DGRAM, 0); // ipv4 & datagram
	if (server_socket_fd == -1)
	{
		perror("Create Socket Failed:");
		exit(1);
	}

	/* 绑定套接口 */
	if (-1 == (bind(server_socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr))))
	{
		perror("Server Bind Failed:");
		exit(1);
	}

	/* 数据传输 */
	while (1)
	{
		/* 定义一个地址，用于捕获客户端地址 */
		struct sockaddr_in client_addr;
		socklen_t client_addr_length = sizeof(client_addr);

		/* 接收数据 */
		char buffer[BUFFER_SIZE];
		memset(buffer, 0, BUFFER_SIZE);
		if (recvfrom(server_socket_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &client_addr_length) == -1)
		{
			perror("Receive Data Failed:");
			exit(1);
		}

		/* 从buffer中拷贝出file_name */
		char file_name[FILE_NAME_MAX_SIZE + 1];
		memset(file_name, 0, FILE_NAME_MAX_SIZE + 1);
		strncpy(file_name, buffer, strlen(buffer)>FILE_NAME_MAX_SIZE ? FILE_NAME_MAX_SIZE : strlen(buffer));
		/*printf("%s\n", file_name);*/
		cout << "Receive content: " << file_name << endl;
		server_receive = file_name;
		server_receive_changed = true;
	}

	closesocket(server_socket_fd);

	/* End the WSA Environment */
	WSACleanup();

	return signal;
}
void* client(void* trans_obj = NULL)
{
	/* Initialize WSA environment */
	WSADATA wsa;						// an example to receive the wsa implementation
	WSAStartup(MAKEWORD(2, 2), &wsa);	// use the version 2.2

	/* 服务端地址 */
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	/*server_addr.sin_addr.s_addr = inet_addr(SERVER_IP_ADDR);*/
	server_addr.sin_addr.s_addr = inet_addr(((struct transmission_obj *)trans_obj)->ip_addr);
	server_addr.sin_port = htons(SERVER_PORT);

	/* 创建socket */
	int client_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (client_socket_fd < 0)
	{
		perror("Create Socket Failed:");
		exit(1);
	}

	/* 输入文件名到缓冲区 */
	char file_name[FILE_NAME_MAX_SIZE + 1];
	memset(file_name, 0, FILE_NAME_MAX_SIZE + 1);
	/*printf("Please Input File Name On Server:\t");
	scanf("%s", file_name);*/
	cout << endl << "<---------------------------------->" << endl;
	cout << "Sending content: " << ((struct transmission_obj *)trans_obj)->content << endl;
	cout << "<---------------------------------->" << endl << endl;
	strcpy(file_name, ((struct transmission_obj *)trans_obj)->content);

	char buffer[BUFFER_SIZE];
	memset(buffer, 0, BUFFER_SIZE);
	strncpy(buffer, file_name, strlen(file_name)>BUFFER_SIZE ? BUFFER_SIZE : strlen(file_name));

	/* 发送文件名 */
	if (sendto(client_socket_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("Send File Name Failed:");
		exit(1);
	}

	closesocket(client_socket_fd);

	/* End the WSA environment */
	WSACleanup();

	return NULL;
}