// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"

HMODULE g_hMod = nullptr;

DWORD WINAPI ThreadFunc(LPVOID param)
{
	for (int i=0; i<40; i++)
	{
		Sleep(1000);
		OutputDebugString(L"----------------------------------count------------------\n");
	}

	if (g_hMod)
	{
		OutputDebugString(L"----------------------------------free start------------------\n");
		FreeLibrary(g_hMod);
		OutputDebugString(L"----------------------------------free end------------------\n");
	}

	 return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		OutputDebugString(L"--------------------DLL_PROCESS_ATTACH--------------------\n");
		g_hMod = hModule;
		CreateThread(nullptr, 0, ThreadFunc, nullptr, 0, nullptr);
		break;
	case DLL_THREAD_ATTACH:
		OutputDebugString(L"--------------------DLL_THREAD_ATTACH--------------------\n");
		break;
	case DLL_THREAD_DETACH:
		OutputDebugString(L"--------------------DLL_THREAD_DETACH--------------------\n");
		break;
	case DLL_PROCESS_DETACH:
		OutputDebugString(L"--------------------DLL_PROCESS_DETACH--------------------\n");
		break;
	default:
		OutputDebugString(L"--------------------UNKNOWN--------------------\n");
		break;
	}

	return TRUE;
}

