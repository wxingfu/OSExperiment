#include "prm.h"

void PRM::TestShell(const string &cmds) {
	//打开命令文件
	fstream infile(cmds);
	vector<string> item;
	string cmd;
	//启动时初始化进程与资源管理器
	Initialize();
	PrintRunProc();
	//逐行读取命令
	while (getline(infile, cmd)) {
		if (cmd != "") {
			//命令解析并选择对应的函数
			item = splitCmd(cmd, " ");
			if (cmd == "to") {
				TimeOut();
				PrintRunProc();
			}
			else if (item[0] == "cr") {
				//item[1] = name, item[2] = priority
				CreateProcess(item[1], (Priority)stoi(item[2]));
				PrintRunProc();
			}
			else if (item[0] == "req") {
				//name = item[1], unit = item[2]
				RequestRes(item[1], stoi(item[2]));
				PrintRunProc();
			}
			else if (item[0] == "rel") {
				ReleaseRes(item[1], stoi(item[2]));
				PrintRunProc();
			}
			else if (item[0] == "de") {
				//item[1] = name
				DeleteProcess(item[1]);
				PrintRunProc();
			}
			else if (item[0] == "list") {
				if (item[1] == "ready") {
					ListReady();
				}
				else if (item[1] == "res") {
					ListRes();
				}
				else if (item[1] == "block") {
					ListBlock();
				}
			}
			else if (item[0] == "pr") {
				//name = item[1]
				PrintPcbInfo(item[1]);
			}
			else {
				cerr << "no this command!" << endl;
			}
		}
	}
	infile.close();
}

void PRM::Initialize() {
	InitProcess();
	//初始化四类资源
	for (int i = 0; i < MAX_RTYPE_NUM; i++) {
		RCB *rcb = new RCB();
		rcb->rid = "R" + to_string(i + 1);
		rcb->total = i + 1;
		rcb->left = i + 1;
		ResList[i].push_back(rcb);
	}
}

void PRM::InitProcess() {
	//初始化init进程
	PCB *pcb = new PCB();
	pcb->id = m_id;
	pcb->pid = "init";
	pcb->priority = Init;
	pcb->state = Ready;
	pcb->parent = nullptr;
	//初始化进程拥有的资源
	for (auto &re : pcb->res) {
		re.rid = "Ri";
		re.used = 0;
		pcb->res->waitReqNum = 0;
	}
	ProcReadyList[Init].push_back(pcb);
	//将当前指针指向init进程,同时也是初始化
	pCurrPCB = pcb;
}

void PRM::CreateProcess(const string &pname, Priority priority) {
	//为进程开辟内存空间，对其属性赋值，然后加入就绪队列
	PCB *pcb = new PCB();
	pcb->id = ++m_id;
	pcb->pid = pname;
	pcb->priority = priority;
	pcb->state = Ready;
	pcb->parent = pCurrPCB;
	pCurrPCB->child.push_back(pcb);
	for (auto &re : pcb->res) {
		re.rid = "Ri";
		re.used = 0;
		pcb->res->waitReqNum = 0;
	}
	ProcReadyList[priority].push_back(pcb);
	Scheduler();
}

PCB *PRM::FindPcbByPid(const string &pname) {
	//先查找就绪队列，在查找阻塞队列
	PCB *fPcb = nullptr;
	for (Priority pri = Sys; pri >= Init; pri = (Priority)(pri - 1)) {
		if (!ProcReadyList[pri].empty()) {
			for (auto itPcb : ProcReadyList[pri]) {
				if (itPcb->pid == pname) {
					fPcb = itPcb;
					break;
				}
			}
		}
	}
	if (!BlockList.empty()) {
		for (auto pcbIt : BlockList) {
			if (pcbIt->pid == pname) {
				fPcb = pcbIt;
				break;
			}
		}
	}
	return fPcb;
}

void PRM::DeleteProcess(const string &pname) {
	//查找到要删除的进程，释放其拥有的资源，从就绪队列或阻塞队列中删除
	//有子进程则删除其子进程
	PCB *toDePcb = FindPcbByPid(pname);
	if (toDePcb != nullptr) {
		RelResource(toDePcb);
		if (toDePcb->state == Ready) {
			ProcReadyList[toDePcb->priority].remove(toDePcb);
		}
		if (toDePcb->state == Block) {
			BlockList.remove(toDePcb);
		}
		while (!toDePcb->child.empty()) {
			DeleteProcess(toDePcb->child.front()->pid);
		}
		Scheduler();
	}
}

void PRM::Scheduler() {
	//从优先级最高寻找进程，找到则调度其运行
	for (Priority pri = Sys; pri >= Init; pri = (Priority)(pri - 1)) {
		if (!ProcReadyList[pri].empty()) {
			pCurrPCB = ProcReadyList[pri].front();
			pCurrPCB->state = Running;
			break;
		}
	}
}

void PRM::RequestRes(const string &rname, int uint) {
	for (int i = 0; i < MAX_RTYPE_NUM; i++) {
		for (auto itRes : ResList[i]) {
			//找到当前运行进程需要申请的资源类别
			if (itRes->rid == rname) {
				pCurrPCB->res[i].rid = rname;
				//剩余资源可分配则申请成功，否则进入阻塞队列
				if (itRes->left >= uint) {
					itRes->left -= uint;
					pCurrPCB->res[i].used += uint;
				}
				else {
					pCurrPCB->state = Block;
					pCurrPCB->res[i].waitReqNum += uint;
					itRes->waitList.push_back(pCurrPCB);
					BlockList.push_back(pCurrPCB);
					ProcReadyList[pCurrPCB->priority].remove(pCurrPCB);
				}
			}
		}
	}
	Scheduler();
}

void PRM::RelResource(PCB *toDePcb) {
	//释放被删除进程拥有的资源
	for (int i = 0; i < MAX_RTYPE_NUM; i++) {
		if (toDePcb->res[i].used != 0) {
			RelRes("R" + to_string(i + 1), toDePcb->res[i].used);
			for (auto itRcb : ResList[i]) {
				if (itRcb->left > itRcb->total) {
					cout << "+++ error +++" << endl;
				}
			}
			toDePcb->res[i].rid = "Ri";
			toDePcb->res[i].used = 0;
		}
	}
}

void PRM::ReleaseRes(const string &rname, int uint) {
	RelRes(rname, uint);
	Scheduler();
}

void PRM::RelRes(const string &rname, int uint) {
	PCB *pcb;
	for (int i = 0; i < MAX_RTYPE_NUM; i++) {
		for (auto itRes : ResList[i]) {
			//查找需要释放的资源
			if (itRes->rid == rname) {
				pCurrPCB->res[i].used -= uint;
				itRes->left += uint;
				//从等待队列里取一个等待进程获得资源并从等待队列中移除然后插入就绪队列
				if (!itRes->waitList.empty()) {
					pcb = itRes->waitList.front();
					while (pcb != nullptr
						&& pcb->res[i].waitReqNum <= itRes->left) {
						itRes->left -= pcb->res[i].waitReqNum;
						//先移除后改状态
						itRes->waitList.remove(pcb);
						pcb->state = Ready;
						pcb->res[i].used += pcb->res[i].waitReqNum;
						//插入就绪队列
						ProcReadyList[pcb->priority].push_back(pcb);
						if (!itRes->waitList.empty()) {
							pcb = itRes->waitList.front();
						}
						else {
							break;
						}
					}
				}
			}
		}
	}
}

void PRM::TimeOut() {
	Priority pri = pCurrPCB->priority;
	//先移除后改状态
	ProcReadyList[pri].remove(pCurrPCB);
	pCurrPCB->state = Ready;
	//然后再插入到就绪队列末尾
	ProcReadyList[pri].push_back(pCurrPCB);
	Scheduler();
}

void PRM::PrintRunProc()
{
	cout << "process" << " " << pCurrPCB->pid
		<< " " << "is running" << " and " << "res ==> ";
	for (int i = 0; i < MAX_RTYPE_NUM; i++) {
		if (pCurrPCB->res[i].rid != "Ri") {
			cout << pCurrPCB->res[i].rid << ":" << pCurrPCB->res[i].used << " ";
		}
	}
	cout << endl;
}

vector<string> PRM::splitCmd(string str, const string &pattern) {
	string::size_type pos;
	vector<string> result;
	str += pattern;
	string::size_type size = str.size();
	for (unsigned i = 0; i < size; i++) {
		pos = str.find(pattern, i);
		if (pos < size) {
			string s = str.substr(i, pos - i);
			result.push_back(s);
			i = pos + pattern.size() - 1;
		}
	}
	return result;
}

void PRM::ListReady()
{
	for (Priority pri = Sys; pri >= Init; pri = (Priority)(pri - 1)) {
		int count = ProcReadyList[pri].size();
		cout << pri << " : ";
		if (!ProcReadyList[pri].empty()) {
			for (auto p : ProcReadyList[pri]) {
				if (p->state == Ready) {
					cout << p->pid;
					if (count > 1)
					{
						cout << "-";
						count--;
					}
				}
				else {
					count--;
				}
			}
		}
		cout << endl;
	}
}

void PRM::ListBlock()
{
	for (int i = 0; i < MAX_RTYPE_NUM; i++) {
		for (auto res : ResList[i]) {
			cout << res->rid << " ";
			int count = res->waitList.size();
			for (auto wp : res->waitList) {
				cout << wp->pid;
				if (count > 1)
				{
					cout << "-";
					count--;
				}
			}
			cout << endl;
		}
	}
}

void PRM::ListRes()
{
	for (int i = 0; i < MAX_RTYPE_NUM; i++) {
		for (auto res : ResList[i]) {
			cout << res->rid << "---->"
				<< "total : " << res->total << " <> "
				<< "left : " << res->left;
			cout << endl;
		}
	}
}

void PRM::PrintPcbInfo(const string& pname)
{
	PCB*pcb = FindPcbByPid(pname);
	cout << "***** " << "info" << " of " << pname << " *****" << endl;
	if (pcb != nullptr) {
		cout << "id : " << pcb->id << "\n";
		cout << "pid : " << pcb->pid << "\n";
		cout << "state : ";
		switch (pcb->state) {
			//Ready = 0, Block, Running
		case 0:
			cout << "Ready" << "\n";
			break;
		case 1:
			cout << "Block" << "\n";
			break;
		case 2:
			cout << "Running" << "\n";
			break;
		}
		cout << "priority : " << pcb->priority << "\n";
		cout << "parent : " << pcb->parent->pid << "\n";
		cout << "res-->";
		for (int i = 0; i < MAX_RTYPE_NUM; i++) {
			if (pcb->res[i].used > 0)
			{
				cout << pcb->res[i].rid << ":" << pcb->res[i].used << " ";
			}
		}
		cout << endl;
	}
	else {
		cout << "the process is exited" << endl;
	}
}
