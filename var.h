#pragma once
#include <string>
#include <map>
#include <iostream>
#include <vector>
#include <fstream>
using namespace std;
#define infinity 9999
#define MAX_NUM 5

using namespace std;

/* message & router info
message:����ĸm
router info: ����ĸr
*/

/*------------------*/

struct node {
	node(string new_ip = "", string new_host_name = "", bool status = true) 
		:ip(new_ip), host_name(new_host_name), up(status) {}
	string ip;
	string host_name;
	bool up;
};

map<string, node *> neighbor_nodes; // ͨ��ip�����ھӽڵ�
map<string, time_t> neighbors_last_modified_time;
// ͨ��ip�����ڽӽڵ���ϴ��޸�ʱ�䣬�����жϳ�ʱ

vector<string> neighbors;    // ��¼�ھӵ�ip
vector<string> all_nodes_ip;    // ��¼������֪�ڵ��ip
								//  �������µ�ipʱ��ʹ��find��������false����Ҫpush

map<string, node *> all_nodes_information_byip;
// ��¼������֪ip�� �ڵ��������Ϣ
map<string, node *> all_nodes_information_byhostname;
// ��¼������֪hostname�Ľڵ��������Ϣ

typedef map<string, int> dest_cost;
map<string, dest_cost> distance_matrix;
// ������ʽ�������ip1��ip2�ľ������
// distance_matrix[ip1][ip2] = cost

map<string, string> dest_nexthop;
// ������ʽ�����絽ip2����һ������ip3
// origin_dest_nexthop[ip2���յ㣩] = ip3(��һ��)

// �ڵ���Ϣ
node myself;

// ��־��дͷ
ofstream outfile;