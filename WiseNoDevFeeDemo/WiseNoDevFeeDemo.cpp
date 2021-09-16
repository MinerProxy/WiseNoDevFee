// WiseMinerDllTest.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <Windows.h>
#include <iostream>
#include <signal.h>
/*
* WiseMiner.dll 相关接口函数
*/

//开始运行反抽水程序
BOOL(__cdecl* Start)(LPCTSTR worker) = 0;

//停止反抽水程序
BOOL(__cdecl* Stop)() = 0;

//反抽水程序是否正在运行
BOOL(__cdecl* IsRunning)() = 0;

//反抽水本次共拦截多少提交共享
INT(__cdecl* GetSubmitWorkNum)() = 0;

//设置是否控制台打印信息，确实为 TRUE
VOID(__cdecl* SetShowMsg)(BOOL status) = 0;

//设置是否记录日志，缺省为TURE, 记录到 log.txt 文件
VOID(__cdecl* SetLogStatus)(BOOL status) = 0;

//输出信息
VOID(__cdecl* LogMsg)(LPCTSTR msg) = 0;

VOID(__cdecl* SetLogPathFile)(LPCTSTR filePath) = 0;

HMODULE hModule = NULL;

//加载设置Dll函数
BOOL MakeInitLib() {
	//加载dll库文件
	hModule = LoadLibrary("DevModule.dll");
	if (hModule != NULL)
	{
		Start = (BOOL(__cdecl*)(LPCTSTR)) GetProcAddress(hModule, "Start");
		Stop = (BOOL(__cdecl*)()) GetProcAddress(hModule, "Stop");
		IsRunning = (BOOL(__cdecl*)()) GetProcAddress(hModule, "IsRunning");
		GetSubmitWorkNum = (int(__cdecl*)()) GetProcAddress(hModule, "GetSubmitWorkNum");
		SetShowMsg = (VOID(__cdecl*)(BOOL)) GetProcAddress(hModule, "SetShowMsg");
		SetLogStatus = (VOID(__cdecl*)(BOOL)) GetProcAddress(hModule, "SetLogStatus");
		LogMsg = (VOID(__cdecl*)(LPCTSTR)) GetProcAddress(hModule, "LogMsg");
		SetLogPathFile = (VOID(__cdecl*)(LPCTSTR)) GetProcAddress(hModule, "SetLogPathFile");
		return TRUE;
	}
	return FALSE;
}

//退出处理程序，停止拦截程序，卸载dll库
int OnExit() {
	//如果正在运行反抽水，尝试停止
	if (IsRunning()) {
		LogMsg("Ready to stop NoDevFee demo...");
		Stop();
		LogMsg("NoDevFee stopped successlly.");
		//等待200毫秒，待日志输出
		Sleep(200);
	}
	if (hModule != NULL) {
		FreeLibrary(hModule);
	}
	//DLL 已经卸载，不能再使用LogMsg
	printf(">>Demo NoDevFee exit success.\n");
	return 0;
}
bool app_stopped = false;
void sigint_handler(int sig) {
	if (sig == SIGINT) {
		// ctrl+c退出时执行的代码
		//std::cout << "ctrl+c pressed!" << std::endl;
		app_stopped = true;
	}
}
int main()
{
	//Register exit function
	_onexit(OnExit);

	if (MakeInitLib()) {
		//设置显示控制台信息
		SetShowMsg(TRUE);

		//设置停止日志文件记录
		SetLogStatus(FALSE);

		//设置显示控制台打印消息
		SetShowMsg(TRUE);

		//开始反抽水拦截，参数为矿机名，也可以为以下几种设置：
		// 1. 矿机名：可以为空，将使用内核矿机名
		// 2. 和内核相同矿池，但不同账号或钱包： {wallet}.{workername}， 比如设置: mywallet1234.MyNoDevFee
		// 3. 矿池地址带钱包格式，讲反抽水拦截到其他矿池和指定钱包：{tcp|ssl}://{poolhost}:{poolport}/{wallet}/{workername}
		//    比如拦截到：  tcp://eth-pool.beepool.org:9530/mywallet123/MyNoDevFee
		if (Start("wisetek.DemoWorker")) {
			LogMsg("Start Demo WiseNoDevFee success.");
		}
		else {
			LogMsg("Demo WiseNoDevFee start error!");
			return -1;
		}
	}

	//循环等待反抽水运行，或进行其他实现处理
	signal(SIGINT, sigint_handler);
	LogMsg("Demo NoDevFee is running, press ctrl+c to stop...");

	int lastNum = -1;
	//循环等待ctrl+c退出程序
	while (!app_stopped) {
		//打印拦截个数，只有变化时输出
		if (lastNum!=-1 && lastNum != GetSubmitWorkNum()) {
			lastNum = GetSubmitWorkNum();
			char buff[256] = { 0 };
			sprintf_s(buff, "Demo redirect share: %d", lastNum);
			LogMsg(buff);
		}
		Sleep(100);
	}
    return 0;
}
