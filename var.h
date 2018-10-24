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
message:首字母m
router info: 首字母r
*/

/*------------------*/

struct node {
	node(string new_ip = "", string new_host_name = "", bool status = true) 
		:ip(new_ip), host_name(new_host_name), up(status) {}
	string ip;
	string host_name;
	bool up;
};

map<string, node *> neighbor_nodes; // 通过ip索引邻居节点
map<string, time_t> neighbors_last_modified_time;
// 通过ip索引邻接节点的上次修改时间，用于判断超时

vector<string> neighbors;    // 记录邻居的ip
vector<string> all_nodes_ip;    // 记录所有已知节点的ip
								//  当出现新的ip时，使用find操作返回false，需要push

map<string, node *> all_nodes_information_byip;
// 记录所有已知ip的 节点的其他信息
map<string, node *> all_nodes_information_byhostname;
// 记录所有已知hostname的节点的其他信息

typedef map<string, int> dest_cost;
map<string, dest_cost> distance_matrix;
// 索引方式：例如从ip1到ip2的距离就是
// distance_matrix[ip1][ip2] = cost

map<string, string> dest_nexthop;
// 索引方式：例如到ip2的下一跳就是ip3
// origin_dest_nexthop[ip2（终点）] = ip3(下一跳)

// 节点信息
node myself;

// 日志读写头
ofstream outfile;