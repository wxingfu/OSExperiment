#include "ProcResManager.h"

//进程&资源 管理器 对象
ProcResManager prm;
//命令文件和输出文件
const char * infile = "cmds.txt";
const char * outfile = "result.txt";
//命令序列
vector<string> cmds;
//正在运行的进程的pid
vector<string> pids;
//读文件
bool readInfile(const char*);
//写文件
bool writeOutfile(const char*);
//命令分割
vector<string> split(string, string);
//shell
void Test_shell();
//创建进程
void createProcess(string pname, int priority);
//请求资源
void requestResources(string pname, int unit);
//释放资源
void releaseResources(string pname, int unit);
//删除进程
void deleteProcess(string pname);



int main()
{
	Test_shell();

	cout << endl;

	writeOutfile(outfile);

	system("pause");
	return 0;
}


bool readInfile(const char * file)
{
	string row;
	ifstream infile(file);

	if (!infile.is_open())
	{
		cerr << "open " << file << " failed" << endl;
		return false;
	}

	while (getline(infile, row))
	{
		if (row != "") {
			cmds.push_back(row);
		}
	}
	infile.close();
	return true;
}


bool writeOutfile(const char * file)
{
	string row;
	ofstream outfile(file);

	if (!outfile.is_open())
	{
		cerr << "open " << file << " failed" << endl;
		return false;
	}

	for (auto& pid : pids)
	{
		outfile << pid << " ";
	}

	outfile.close();
	return true;
}


vector<string> split(string str, string pattern)
{
	string::size_type pos;
	vector<string> result;
	str += pattern;//扩展字符串以方便操作
	string::size_type size = str.size();

	for (unsigned i = 0; i < size; i++)
	{
		pos = str.find(pattern, i);
		if (pos < size)
		{
			string s = str.substr(i, pos - i);
			result.push_back(s);
			i = pos + pattern.size() - 1;
		}
	}
	return result;
}


void Test_shell()
{
	int priority, unit;
	string pname;

	readInfile(infile);

	int cmdSize = cmds.size();
	vector<string> item;

	for (int i = 0; i < cmdSize; i++)
	{
		//cout << "shell>";
		if (cmds[i] == "init")
		{
			//cout << cmds[i] << endl << endl;
			prm.initialize();
			cout << prm.pcb[prm.currRunning].pid << " ";
			pids.push_back(prm.pcb[prm.currRunning].pid);
		}
		else
		{
			item = split(cmds[i], " ");
			if (cmds[i] == "to")
			{
				//cout << cmds[i] << endl << endl;
				prm.timeout();
				cout << prm.pcb[prm.currRunning].pid << " ";
				pids.push_back(prm.pcb[prm.currRunning].pid);
			}
			else if (item[0] == "cr")
			{
				//item[0] = cmd, item[1] = name, item[2] = priority
				//cout << item[0] << " " << item[1] << " " << item[2] << endl << endl;
				pname = item[1];
				priority = stoi(item[2]);
				createProcess(pname, priority);

			}
			else if (item[0] == "req")
			{
				//cmd = item[0], name = item[1], unit = item[2]
				//cout << item[0] << " " << item[1] << " " << item[2] << endl << endl;
				pname = item[1];
				unit = stoi(item[2]);
				requestResources(pname, unit);
			}
			else if (item[0] == "rel")
			{
				//cout << item[0] << " " << item[1] << " " << item[2] << endl << endl;
				pname = item[1];
				unit = stoi(item[2]);
				releaseResources(pname, unit);

			}
			else if (item[0] == "de")
			{
				//cout << item[0] << " " << item[1] << endl << endl;
				pname = item[1];
				deleteProcess(pname);
			}
			else {
				cout << "error (else error)" << " ";
				pids.push_back("error (else error)");
			}
		}
	}
}


void createProcess(string pname, int priority)
{
	if (prm.contain(pname) != -1)
	{
		cout << "error (duplicate name)" << " ";
		pids.push_back("error (duplicate name)");
	}
	else if (priority > 2 or priority <= 0)
	{
		cout << "error priority" << " ";
		pids.push_back("error priority");
	}
	else {
		prm.create(pname, priority);
		cout << prm.pcb[prm.currRunning].pid << " ";
		pids.push_back(prm.pcb[prm.currRunning].pid);
	}
}


void requestResources(string pname, int unit)
{
	if (pname == "R1" && unit == 1)
	{
		prm.request(0, unit);
		cout << prm.pcb[prm.currRunning].pid << " ";
		pids.push_back(prm.pcb[prm.currRunning].pid);
	}
	else if (pname == "R2" && (0 < unit && unit <= 2))
	{
		prm.request(1, unit);
		cout << prm.pcb[prm.currRunning].pid << " ";
		pids.push_back(prm.pcb[prm.currRunning].pid);
	}
	else if (pname == "R3" && (0 < unit && unit <= 3))
	{
		prm.request(2, unit);
		cout << prm.pcb[prm.currRunning].pid << " ";
		pids.push_back(prm.pcb[prm.currRunning].pid);
	}
	else if (pname == "R4" && (0 < unit && unit <= 4))
	{
		prm.request(3, unit);
		cout << prm.pcb[prm.currRunning].pid << " ";
		pids.push_back(prm.pcb[prm.currRunning].pid);
	}
	else {
		cout << "error (invalid request)" << " ";
		pids.push_back("error (invalid request)");
	}
}


void releaseResources(string pname, int unit)
{
	if (pname == "R1" && unit == 1)
	{
		if (prm.pcb[prm.currRunning].other_resource[0].used >= unit)
		{
			prm.release(0, unit);
			cout << prm.pcb[prm.currRunning].pid << " ";
			pids.push_back(prm.pcb[prm.currRunning].pid);
		}
		else {
			cout << "error (release exceed actual R1 units)" << " ";
			pids.push_back("error (release exceed actual R1 units)");
		}
	}
	else if (pname == "R2" && (0 < unit && unit <= 2))
	{
		if (prm.pcb[prm.currRunning].other_resource[1].used >= unit)
		{
			prm.release(1, unit);
			cout << prm.pcb[prm.currRunning].pid << " ";
			pids.push_back(prm.pcb[prm.currRunning].pid);
		}
		else {
			cout << "error (release exceed actual R2 units)" << " ";
			pids.push_back("error (release exceed actual R2 units)");
		}
	}
	else if (pname == "R3" && (0 < unit && unit <= 3))
	{
		if (prm.pcb[prm.currRunning].other_resource[2].used >= unit)
		{
			prm.release(2, unit);
			cout << prm.pcb[prm.currRunning].pid << " ";
			pids.push_back(prm.pcb[prm.currRunning].pid);
		}
		else {
			cout << "error (release exceed actual R3 units)" << " ";
			pids.push_back("error (release exceed actual R3 units)");
		}
	}
	else if (pname == "R4" && (0 < unit && unit <= 4))
	{
		if (prm.pcb[prm.currRunning].other_resource[3].used >= unit)
		{
			prm.release(3, unit);
			cout << prm.pcb[prm.currRunning].pid << " ";
			pids.push_back(prm.pcb[prm.currRunning].pid);
		}
		else {
			cout << "error (release exceed actual R4 units)" << " ";
			pids.push_back("error (release exceed actual R4 units)");
		}
	}
	else {
		cout << "error (invalid release)" << " ";
		pids.push_back("error (invalid release)");
	}
}


void deleteProcess(string pname)
{
	const int idx = prm.contain(pname);
	if (idx == -1)
	{
		cout << "error (process not existed)" << " ";
		pids.push_back("error (process not existed)");
	}
	else {
		prm.destroy(idx);
		cout << prm.pcb[prm.currRunning].pid << " ";
		pids.push_back(prm.pcb[prm.currRunning].pid);
	}
}
