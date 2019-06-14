#include "ProcResManager.h"

ProcResManager::ProcResManager()
{
	for (int i = 0; i < MAX_PROC_NUM; i++)
	{
		pcb[i].id = i;
	}

	pcb[0].pid = "init";
	pcb[0].priority = 0;
	ProcList[0].push_back(0);
	currRunning = 0;

	//rid,initial,remaining
	R1 = { "R1",1,1 };
	R2 = { "R2",2,2 };
	R3 = { "R3",3,3 };
	R4 = { "R4",4,4 };

	rcb[0] = R1;
	rcb[1] = R2;
	rcb[2] = R3;
	rcb[3] = R4;
}


ProcResManager::~ProcResManager() = default;


void ProcResManager::initialize()
{
	for (int i = 1; i < MAX_PROC_NUM; i++)
	{
		pcb[i].pid = " ";
		pcb[i].type = "ready";
		pcb[i].parent = -1;
		pcb[i].children = -1;
		pcb[i].older = -1;
		pcb[i].younger = -1;
		pcb[i].priority = -1;

		for (auto& j : pcb[i].other_resource)
		{
			j.rid = -1;
			j.used = 0;
			j.waitReq = 0;
		}
	}

	currRunning = 0;

	for (int i = 0; i < MAX_RES_CATEGORY; i++)
	{
		rcb[i].initial = i + 1;
		rcb[i].remaining = i + 1;
		while (!rcb[i].wait_list.empty())
		{
			rcb[i].wait_list.pop_front();
		}
	}

	for (auto& i : ProcList)
	{
		while (!i.empty())
		{
			i.pop_back();
		}
	}
}

void ProcResManager::create(string name, int pri)
{
	for (int i = 1; i < MAX_PROC_NUM; i++)
	{
		if (pcb[i].pid == " ")
		{
			pcb[i].pid = name;
			pcb[i].priority = pri;
			ProcList[pri].push_back(pcb[i].id);
			pcb[i].parent = currRunning;

			for (int j = 1; j < MAX_PROC_NUM; j++)
			{
				if (j < i && pcb[j].parent == pcb[i].parent)
				{
					pcb[j].younger = i;
					pcb[i].older = j;
				}
			}
			break;
		}
	}
	scheduler();
}

int ProcResManager::scheduler()
{
	if (!ProcList[2].empty()) {
		currRunning = ProcList[2].front();
		pcb[currRunning].type = "running";
		return ProcList[2].front();
	}
	else if (!ProcList[1].empty()) {
		currRunning = ProcList[1].front();
		pcb[currRunning].type = "running";
		return ProcList[1].front();
	}
	else {
		currRunning = 0;
		pcb[currRunning].type = "running";
		return 0;
	}
}

void ProcResManager::destroy(int n)
{
	for (int i = 0; i < MAX_RES_CATEGORY; i++)
	{
		if (pcb[n].other_resource[i].used != 0)
		{
			release2(i, pcb[n].other_resource[i].used);

			if (rcb[i].remaining > rcb[i].initial)
			{
				cout << "error in destroy: delete resources exit initial units" << endl;
			}

			pcb[n].other_resource[i].rid = -1;
			pcb[n].other_resource[i].used = 0;
		}
	}

	if (pcb[n].type == "ready" || pcb[n].type == "running")
	{
		const int p = pcb[n].priority;
		ProcList[p].remove(n);
	}
	else if ((pcb[n].type) == "blocked")
	{
		for (int i = 0; i < MAX_RES_CATEGORY; i++)
		{
			rcb[i].wait_list.remove(n);
		}
	}

	for (int i = 0; i < MAX_PROC_NUM; i++)
	{
		if (pcb[i].parent == n)
		{
			destroy(pcb[i].id);
		}

		if (pcb[i].id == n)
		{
			pcb[i].pid = " ";
			pcb[i].type = "ready";
			pcb[i].parent = -1;
			pcb[i].children = -1;
			pcb[i].older = -1;
			pcb[i].younger = -1;
			pcb[i].priority = -1;

			for (int j = 0; j < MAX_RES_CATEGORY; j++)
			{
				pcb[i].other_resource[j].rid = -1;
				pcb[i].other_resource[j].used = 0;
				pcb[i].other_resource[j].waitReq = 0;
			}
		}

		if (pcb[i].older == n)
		{
			pcb[i].older = -1;
		}

		if (pcb[i].younger == n)
		{
			pcb[i].younger = -1;
		}
	}
	scheduler();
}

int ProcResManager::contain(string name)
{
	for (int i = 0; i < MAX_PROC_NUM; i++)
	{
		if (name == pcb[i].pid)
		{
			return i;
		}
	}
	return -1;
}


void ProcResManager::request(int n, int unit)
{
	if (rcb[n].remaining >= unit)
	{
		rcb[n].remaining -= unit;
		pcb[currRunning].other_resource[n].rid = n;
		pcb[currRunning].other_resource[n].used += unit;
	}
	else {
		pcb[currRunning].type = "blocked";
		pcb[currRunning].other_resource[n].waitReq += unit;
		rcb[n].wait_list.push_back(currRunning);
		ProcList[pcb[currRunning].priority].remove(currRunning);
	}
	scheduler();
}

void ProcResManager::release(int n, int unit)
{
	release2(n, unit);
	scheduler();
}

void ProcResManager::release2(int n, int unit)
{
	pcb[currRunning].other_resource[n].used -= unit;
	rcb[n].remaining += unit;
	if (!rcb[n].wait_list.empty())
	{
		//当list为空时，list的front函数会报错。
		int wpcb = rcb[n].wait_list.front();
		while (wpcb != 0 &&
			pcb[wpcb].other_resource[n].waitReq <= rcb[n].remaining)
		{
			rcb[n].remaining -= pcb[wpcb].other_resource[n].waitReq;
			rcb[n].wait_list.remove(wpcb);
			pcb[wpcb].type = "ready";
			pcb[wpcb].other_resource[n].used += pcb[wpcb].other_resource[n].waitReq;
			ProcList[pcb[wpcb].priority].push_back(wpcb);
			if (!rcb[n].wait_list.empty())
			{
				wpcb = rcb[n].wait_list.front();
			}
			else {
				break;
			}
		}
	}
	else {
		cout << "There is no  process that is waiting resources" << endl;
	}
}

void ProcResManager::timeout()
{
	const int priority = pcb[currRunning].priority;
	ProcList[priority].remove(currRunning);

	pcb[currRunning].type = "ready";

	ProcList[priority].push_back(currRunning);
	scheduler();
}
