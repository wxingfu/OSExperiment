/*
Author : UESTC wxingfu
*/

#ifndef PRM_H
#define PRM_H


#include <iostream>
#include <fstream>
#include <list>
#include <vector>
#include <string>
using namespace std;

//最大资源类别
#define	MAX_RTYPE_NUM	4

//进程优先级
enum Priority {
	//Init = 0, Usr=1, Sys=2
	Init = 0, Usr, Sys
};

//进程状态
enum State {
	//Ready = 0, Block=1, Running=2
	Ready = 0, Block, Running
};

//进程资源
typedef struct Resource {
	string rid;			//资源ID
	int used = 0;		//拥有的资源
	int waitReqNum = 0;	//在等待的资源
} res;

//进程控制块
typedef struct PCB {
	int id;						//进程ID
	string pid;					//进程名
	Priority priority;			//进程优先级
	State state;				//进程状态
	PCB *parent = nullptr;		//父进程
	list<PCB *> child;			//子进程
	res res[MAX_RTYPE_NUM];		//拥有的资源
} PCB;

//资源控制块
typedef struct RCB {
	string rid;				//资源ID
	int total;				//总的资源
	int left;				//剩余资源
	list<PCB *> waitList;	//等待资源的进程队列
} RCB;

//进程与资源管理器
class PRM {
public:
	//交互
	void TestShell(const string &cmds);

private:
	//用于自增进程ID
	int m_id = 0;
	//全局进程指针，指向当前运行进程							
	PCB *pCurrPCB = nullptr;
	//资源队列				
	list<RCB *> ResList[MAX_RTYPE_NUM];
	//就绪队列	
	list<PCB *> ProcReadyList[3];
	//阻塞队列
	list<PCB *> BlockList;

	//初始化
	void Initialize();
	//初始化进程
	void InitProcess();
	//创建进程
	void CreateProcess(const string &pname, Priority priority);
	//通过进程Pid查找进程
	PCB *FindPcbByPid(const string &pname);
	//撤销进程
	void DeleteProcess(const string &pname);
	//调度
	void Scheduler();
	//申请资源
	void RequestRes(const string &rname, int uint);
	//释放资源
	void ReleaseRes(const string &rname, int uint);
	void RelResource(PCB *toDePcb);
	void RelRes(const string &rname, int uint);
	//模拟时间片超时
	void TimeOut();
	//打印当前运行进程
	void PrintRunProc();
	//命令分割
	vector<string> splitCmd(string str, const string &pattern);
	//显示就绪队列信息
	void ListReady();
	//显示阻塞队列信息
	void ListBlock();
	//显示当前资源信息
	void ListRes();
	//打印指定进程信息
	void PrintPcbInfo(const string& pname);
};



#endif // !PRM_H
