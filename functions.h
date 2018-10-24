#pragma once
#pragma comment(lib, "pthreadVC2.lib")
#include <pthread.h>
#include <sstream>
#include <Winsock2.h>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <Windows.h>
#include <time.h>
#include <string>
#include <map>
#include <iterator>
#include <vector>

#include "var.h"
#include "socket_com.h"
#define infinity 9999
#define MAX_NUM 3
#define MESSAGE_INTERVAL 5* 1000
#define DEATH_TIME 100  // seconds
#define UPDATE_INTERVAL 5 * 1000
using namespace std;
string IP_ADDR = "192.168.1.1";
using namespace std;

/*--------------------------functions----------------------------*/
// ·��������
void initial();			// ��ʼ����������·����Ϣ
void send_router();		// ���ڸ��£����ھӷ��������������
void* listen(void*);	// ��������
						// ���ݼ���������Ϣ��������receive_router / forward_message / receive_message
void receive_router(string);	// �����ڽӽڵ㷢���ľ�������
								// ���ֽ��յ���router��Ϣʱ���ã��������ھ����Լ�����ľ�������,���ָı�����DV-algo
void DV_algo(string src_ip, map<string, dest_cost>);				// ·���㷨,����ı���� ���ҵ���send_router
void forward_message(string, string);		// ת����Ϣ **������Ŀ��ip����Ϣ
void* count_neighbor_time(void*);			// �ж��ھӳ�ʱ  �޸ļ�¼��distance_matrix������DV_algo
void* count_upgrade_time(void*);			// ����ÿ��30s����һ��send router

											// �ͻ��˹���
void receive_message(string);		// ������Ϣ
void* send_message(void*);			// ������Ϣ
void print_router();				// ��ӡ·�ɱ�

									// auxiliary functions
void* distribute(void* signal = NULL)
{
	while (1)
	{
		if (server_receive_changed)
		{
			if (server_receive[0] == 'r')
				// router info
			{
				receive_router(server_receive);
			}
			else if (server_receive[0] == 'm')
				// message
			{
				string dest_ip = "";
				int i = 0;
				for (i = 2; server_receive[i] != '\n'; i++)
				{
					dest_ip += server_receive[i];
				}
				server_receive.erase(0, i + 1);

				if (dest_ip == myself.ip)
					// receive
				{
					receive_message(server_receive);
				}
				else
					// forward
				{
					forward_message(dest_ip, server_receive);
				}
			}

			server_receive_changed = false;
		}
	}
	return NULL;
}
void print_all_neighbors() {
	for (int i = 0; i < neighbors.size(); i++)
	{
		cout << "Neighbor " << i << endl
			<< "---------------------------------------" << endl
			<< "IP: " << setw(15) << neighbors[i] << endl
			<< "Name: " << setw(13) << neighbor_nodes[neighbors[i]]->host_name << endl
			<< "Up: " << setw(15) << neighbor_nodes[neighbors[i]]->up << endl;
		cout << "Last Modified Time: " << endl << ctime(&neighbors_last_modified_time[neighbors[i]]) << endl;
		cout << endl;
	}
}
void print_all_nodes_ip() {
	for (int i = 0; i < all_nodes_ip.size(); i++)
	{
		cout << "Node " << i << " IP: " << all_nodes_ip[i] << endl;
	}
}
void no_more_than_infinity()
{
	for (int i = 0; i < all_nodes_ip.size(); i++)
	{
		for (int s = 0; s < all_nodes_ip.size(); s++)
		{
			if (distance_matrix[all_nodes_ip[i]][all_nodes_ip[s]] > infinity)
				distance_matrix[all_nodes_ip[i]][all_nodes_ip[s]] = infinity;
		}
	}
}

// definations of the functions

void DV_algo(string src_ip, map<string, dest_cost> distance_matrix_temp)
{
	bool changed = false;

	for (int i = 0; i < all_nodes_ip.size(); i++)
		// tranverse the distance matrix to update the cost and next hop when the change on original route
	{
		string target_ip = all_nodes_ip[i];
		if (dest_nexthop[target_ip] == src_ip)
			// if the src_ip was on one origin route
		{
			// upgrade the cost caused by the src_ip's cost change
			int origin_cost = distance_matrix_temp[myself.ip][target_ip];
			distance_matrix_temp[myself.ip][target_ip] = distance_matrix_temp[myself.ip][src_ip] + distance_matrix_temp[src_ip][target_ip];

			// avoid cross boundary
			if (distance_matrix_temp[myself.ip][target_ip] > infinity)
				distance_matrix_temp[myself.ip][target_ip] = infinity;

			if (origin_cost != distance_matrix_temp[myself.ip][target_ip])
			{
				// mark the change
				changed = true;
			}

			// when the new cost is higher 
			if (origin_cost < distance_matrix_temp[myself.ip][target_ip])
			{
				// ** poisoned reverse ** (not contradict with disapear neighor node)
				distance_matrix_temp[myself.ip][target_ip] = infinity;
				// then find instead next hop
				for (int mid_node = 0; mid_node < neighbors.size(); i++)
				{
					string mid_ip = neighbors[mid_node];
					if (mid_ip == src_ip) continue;
					if (distance_matrix_temp[mid_ip][target_ip] + distance_matrix_temp[myself.ip][mid_ip] < infinity)
					{
						dest_nexthop[target_ip] = mid_ip;
						distance_matrix_temp[myself.ip][target_ip] = distance_matrix_temp[mid_ip][target_ip] + distance_matrix_temp[myself.ip][mid_ip];

					}
				}
			}
		}
	}
	// if the src_ip is not original next hop
	for (int i = 0; i < all_nodes_ip.size(); i++)
		// traverse the distance matrix to update the lower cost and next hop
	{
		string target_ip = all_nodes_ip[i];
		if (distance_matrix_temp[myself.ip][src_ip] + distance_matrix_temp[src_ip][target_ip] < distance_matrix_temp[myself.ip][target_ip])
			// when road with lower cost found
		{
			dest_nexthop[target_ip] = src_ip;	// upgrade the next hop to the destination
												// upgrade the cost to the destination
			distance_matrix_temp[myself.ip][target_ip] = distance_matrix_temp[myself.ip][src_ip] + distance_matrix_temp[src_ip][target_ip];

			// mark the change
			changed = true;
		}
	}
	distance_matrix = distance_matrix_temp;
	if (changed)
		// send changed router to neighbors
	{
		send_router();
	}
	print_router();

}

void* count_upgrade_time(void*)
{
	while (true)
		// periodically send routers with UPDATE interval
	{
		Sleep(UPDATE_INTERVAL);
		send_router();
	}
}

void* count_neighbor_time(void*)
{
	while (true)
		// confirm neighbors' death after about DEATH_TIME
	{
		for (int i = 0; i < neighbors.size(); i++)
		{
			if (time(0) - neighbors_last_modified_time[neighbors[i]] > DEATH_TIME)
			{
				time_t current_time = time(0);
				cout << "��ʱ��\n" << ctime(&current_time) << endl;
				cout << "Last Modified Time:\n" << ctime(&neighbors_last_modified_time[neighbors[i]]) << endl;
				// confirm dead of neighbors[i] and change the distance matrix
				for (int s = 0; s < all_nodes_ip.size(); s++)
					// cut all the link from / to neighbor[i]
				{
					distance_matrix[neighbors[i]][all_nodes_ip[s]] = infinity;
				}
				distance_matrix[myself.ip][neighbors[i]] = infinity;

				DV_algo(neighbors[i], distance_matrix);
			}
			Sleep(20);
		}
	}
}

void print_router()
{

	outfile.open(".\\myfile.txt", ios::app);
	if (!outfile) //����ļ��Ƿ�������//�������ڼ���ļ��Ƿ����
	{
		cout << "abc.txt can't open" << endl;
		abort(); //��ʧ�ܣ���������
	}
	else
	{
		outfile << "\n####################\n";
		outfile << "Router of " << myself.host_name << "(" << myself.ip << ")" << endl;
		outfile << setw(30) << "Destination" << setw(30) << "Next Hop" << endl;
		outfile << "<----------------------------------------------------------------------->" << endl;
		for (map<string, string>::iterator iter = dest_nexthop.begin(); iter != dest_nexthop.end(); iter++)
		{
			outfile << setw(30) << iter->first << setw(30) << iter->second << endl;
		}
		outfile << endl << "Total Num: " << dest_nexthop.size() << endl << endl;

		outfile << "Destiance info:\n";
		for (auto it = distance_matrix.begin(); it != distance_matrix.end(); ++it) {
			auto dis_map = it->second;
			for (auto iter = dis_map.begin(); iter != dis_map.end(); ++iter) {
				outfile << it->first << "  to " << iter->first << " : " << iter->second << endl;
			}
		}
		outfile << "\n####################\n";


		outfile.close();
	}

	//system("cls");
	cout << "Router of " << myself.host_name << "(" << myself.ip << ")" << endl;
	cout << setw(30) << "Destination" << setw(30) << "Next Hop" << endl;
	cout << "<----------------------------------------------------------------------->" << endl;
	for (map<string, string>::iterator iter = dest_nexthop.begin(); iter != dest_nexthop.end(); iter++)
	{
		cout << setw(30) << iter->first << setw(30) << iter->second << endl;
	}
	cout << endl << "Total Num: " << dest_nexthop.size() << endl;
	return;
}

void initial()
{
	/* ����ڵ��� */
	system("cls");
	int num_net_nodes = 0;
	cout << "����������ڵ�����" << endl;
	cin >> num_net_nodes;

	/* ����ڵ�ip */
	cout << "��������������ڵ�ip(��һ��Ϊ���ڵ�)��" << endl;
	for (int i = 0; i < num_net_nodes; i++)
	{
		string ip_addr = "";
		cin >> ip_addr;
		all_nodes_ip.push_back(ip_addr);
		// ��ʼ���������
		distance_matrix[all_nodes_ip[0]][all_nodes_ip[i]] = infinity;
		distance_matrix[all_nodes_ip[0]][all_nodes_ip[0]] = 0;
	}

	print_all_nodes_ip();
	//system("pause");
	//system("cls");
	/* ���ڵ���Ϣ */
	string myname;
	cout << "�����뱾�ڵ�Hostname; " << endl;
	cin >> myname;
	myself = *(new node(all_nodes_ip[0], myname));

	cout << "������Ĭ������:\n";
	cin >> IP_ADDR;

	/* ��ʼ��next hop */
	for (int i = 0; i < num_net_nodes; i++)
	{
		dest_nexthop[all_nodes_ip[i]] = IP_ADDR;
	}

	/* ���ڵ��ھ� */
	int num_neighbors = 0;
	cout << "�������ھ�������" << endl;
	cin >> num_neighbors;
	cout << "�����������ڽӽڵ�IP��Hostname(4 characters): " << endl;
	for (int i = 0; i < num_neighbors; i++)
	{
		// ip
		string ip_addr = "";
		cin >> ip_addr;
		neighbors.push_back(ip_addr);
		// nodes
		string host_name = "";
		cin >> host_name;
		node* new_nodes = new node(ip_addr, host_name);
		neighbor_nodes[ip_addr] = new_nodes;
		// last modified time
		neighbors_last_modified_time[ip_addr] = time(0);
		// other info
		all_nodes_information_byhostname[host_name] = new_nodes;
		all_nodes_information_byip[new_nodes->ip] = new_nodes;
		// next hop
		dest_nexthop[ip_addr] = ip_addr;
	}
	print_all_neighbors();

	/* ���ڽӵ���� */
	for (int i = 0; i < num_neighbors; i++)
	{
		int cost = 1;
		cout << "�����뱾�ڵ㵽 " << neighbors[i] << " ��cost: " << endl;
		cin >> cost;
		distance_matrix[all_nodes_ip[0]][neighbors[i]] = cost;
	}

}

void send_router() {
	for (int i = 0; i < neighbors.size(); i++)
	{
		string content = "";
		content += "r\n";		// mark of router
		content += myself.ip;	// record the sender
		content += '\n';
		for (dest_cost::iterator str_p = distance_matrix[myself.ip].begin(); str_p != distance_matrix[myself.ip].end(); str_p++)
		{
			content += str_p->first;
			content += " ";
			content += to_string(str_p->second);
			content += "\n";
		}
		transmission_obj trans_obj(neighbors[i].c_str(), content.c_str());
		client((void*)&trans_obj);
	}

}

void* listen(void* signal = NULL)
{
	pthread_t threads[2];
	pthread_create(&threads[0], NULL, server, NULL);
	pthread_create(&threads[1], NULL, distribute, NULL);
	pthread_join(threads[0], NULL);
	pthread_join(threads[1], NULL);
	return signal;
}

void receive_router(string receivemessage) {
	string src_ip = "";
	int i = 0; // mark the position in the received message
	for (i = 2; receivemessage[i] != '\n'; i++)
	{
		src_ip += receivemessage[i];
	}
	cout << "�յ�����" << src_ip << "��·�ɸ��£�" << endl;
	while (i != receivemessage.size() - 1)
	{
		string dest_ip = "";
		for (i = i + 1; receivemessage[i] != ' '; i++)
		{
			dest_ip += receivemessage[i];
		}
		string str_cost = "";
		for (i = i + 1; receivemessage[i] != '\n'; i++)
		{
			str_cost += receivemessage[i];
		}
		// update the distance_matrix
		distance_matrix[src_ip][dest_ip] = stoi(str_cost);
	}
	DV_algo(src_ip, distance_matrix);
}

void receive_message(string content) {
	//for (int i = start_pos; i < server_receive.size(); i++)
	//	// print the message from start_position
	//{
	//	cout << server_receive[i];
	//}
	cout << "�յ���Ϣ��" << endl
		<< content << endl;
}

void* send_message(void* signal = NULL)
{
	//string content = "";
	//string target_ip = "";
	//cout << "������Message��Ŀ��IP: " << endl;
	//cin >> target_ip;

	//content += "m";
	//content += dest_nexthop[target_ip];
	//content += '\n';
	//cout << "��������Ҫ�����Message: " << endl;
	//cin >> content;
	while (true)
	{
		for (int i = 0; i < all_nodes_ip.size(); i++)
		{
			if (all_nodes_ip[i] != myself.ip) {
				string content = "";
				string target_ip = "";
				target_ip = all_nodes_ip[i];

				content += "m";
				content += '\n';
				content += target_ip;
				content += '\n';
				content += "Test message from ";
				content += myself.ip;
				content += " to ";
				content += target_ip;

				// send
				transmission_obj trans_obj((dest_nexthop[target_ip]).c_str(), content.c_str());
				client((void*)&trans_obj);
			}
		}
		Sleep(MESSAGE_INTERVAL); // ÿ��һ��ʱ�䷢��һ����Ϣ
	}
	return signal;
}

void forward_message(string target_ip, string content)
{
	cout << "ת����Ϣ�� " << endl
		<< content << endl;
	content += "m";
	content += "\nTarget IP: ";
	content += target_ip;
	content += "\nContent: ";
	transmission_obj trans_obj((dest_nexthop[target_ip]).c_str(), content.c_str());
	client((void*)&trans_obj);
}