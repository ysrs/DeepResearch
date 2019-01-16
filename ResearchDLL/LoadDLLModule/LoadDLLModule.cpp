// LoadDLLModule.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <Windows.h>
#include "InjectDll.h"


DWORD WINAPI ThreadFunc(LPVOID param)
{
	HMODULE hMod = LoadLibrary("DllModule.dll");

	if (hMod == nullptr)
	{
		printf("Load module failed\n");
	}
	else
	{
		printf("Load module succeed\n");
	}

	for (int i=0; i<20; ++i)
	{
		printf("remain %d second to \n", 19 - i);
		Sleep(1000);
	}

	FreeLibrary(hMod);

	return 0;
}


int main()
{
	//CreateThread(nullptr, 0, ThreadFunc, nullptr, 0, nullptr);

	//InjectDllToTarget("InjectMe", "DllModule.dll");

	HMODULE hMod = LoadLibrary("DllModule.dll");

	if (hMod == nullptr)
	{
		printf("Load module failed\n");
	}
	else
	{
		printf("Load module succeed\n");
	}

	//for (int i = 0; i<20; ++i)
	//{
	//	printf("remain %d second to \n", 19 - i);
	//	Sleep(1000);
	//}

	printf("key boarrd to free1 begin\n");
	getchar();
	printf("key boarrd to free1 end\n");
	//FreeLibrary(hMod);

	FreeLibraryAndExitThread(hMod, 0);

	//printf("key boarrd to free2 begin\n");
	//getchar();
	//printf("key boarrd to free2 end\n");
	//FreeLibrary(hMod);

	Sleep(-1);
    return 0;
}

