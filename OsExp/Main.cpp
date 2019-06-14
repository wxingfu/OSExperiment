#include "prm.h"

int main() {
	//命令文件
	const string cmds = "cmds.txt";
	//创建进程与资源管理器
	PRM prm;
	//输入命令
	prm.TestShell(cmds);

	cout << endl;
	system("pause");
	return 0;
}
