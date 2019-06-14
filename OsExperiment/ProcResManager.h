#ifndef MANAGER_H
#define MANAGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <list>
using namespace std;


//最大进程数
#define MAX_PROC_NUM		20
//四类资源
#define MAX_RES_CATEGORY	4

//资源控制块
typedef struct ResControlBlock {
	string rid = "Ri";
	int initial = 0;
	int remaining = 0;
	list<int> wait_list;
}RCB;

//资源结构体
typedef struct Resource {
	int rid = -1;
	int used = 0;
	int waitReq = 0;
}resource;

//进程控制块
typedef struct ProcControlBlock {
	string pid = " ";
	string  type = "ready";
	int id = -1;
	int parent = -1;
	int children = -1;
	int younger = -1;
	int older = -1;
	int priority = -1;
	resource other_resource[MAX_RES_CATEGORY];
}PCB;

//进程&资源管理器
class ProcResManager
{
	//创建四类资源
	RCB R1, R2, R3, R4;
	//进程列表，优先级(Init=0, User=1, System=2)
	list<int> ProcList[3];
	//int delete_number = -1;
public:
	RCB rcb[MAX_RES_CATEGORY];
	PCB pcb[MAX_PROC_NUM];
	int currRunning;

	//初始化管理器
	ProcResManager();
	//析构
	~ProcResManager();
	//初始化进程和资源以及进程列表
	void initialize();
	//创建进程
	void create(string name, int pri);
	//调度器
	int  scheduler();
	//销毁进程
	void destroy(int n);
	//判断是否存在此进程
	int  contain(string name);
	//请求资源
	void request(int n, int unit);
	//释放资源
	void release(int n, int unit);
	//释放资源
	void release2(int n, int unit);
	//超时
	void timeout();
};



#endif // !MANAGER_H
