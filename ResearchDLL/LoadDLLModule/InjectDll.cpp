#include "InjectDll.h"
#include <Windows.h>
#include <TlHelp32.h>


//判断当前操作系统版本
int GetOSVerion()
{
	OSVERSIONINFO osver;
	osver.dwOSVersionInfoSize = sizeof osver;
	GetVersionEx(&osver);
	if (osver.dwPlatformId == 2)
	{
		if (osver.dwMajorVersion == 5 && osver.dwMinorVersion == 1)
		{
			//	printf("windows xp\n");
			return 2;
		}
		if (osver.dwMinorVersion == 5 && osver.dwMinorVersion == 2)
		{
			//	printf("windows 2003\n");
			return 3;
		}
		if (osver.dwMajorVersion == 6 && osver.dwMinorVersion == 0)
		{
			//	printf("vista 2008 and 2008\n");
			return 4;
		}
		if (osver.dwMajorVersion == 6 && osver.dwMinorVersion == 1)
		{
			//	printf("2008 R2 and windows 7\n");
			return 5;
		}
		if (osver.dwMajorVersion == 6 && osver.dwMinorVersion == 2)
		{
			//	printf("windows 10\n");
			return 6;
		}
	}
	return 0;

}

BOOL EnableDebugPrivilege()
{
	HANDLE hToken;
	BOOL fOk = FALSE;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) //Get Token
	{
		TOKEN_PRIVILEGES tp;
		tp.PrivilegeCount = 1;
		if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid))//Get Luid
			printf("Can't lookup privilege value.\n");
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;//这一句很关键，修改其属性为SE_PRIVILEGE_ENABLED
		if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL))//Adjust Token
			printf("Can't adjust privilege value.\n");
		fOk = (GetLastError() == ERROR_SUCCESS);
		CloseHandle(hToken);
	}
	return fOk;
}

static bool InjectDllProxy(DWORD dwPid, PCSTR pszProxyFile)
{
	bool bRet = false;
	HANDLE hProcess;
	size_t iPorxyFileLen = strlen(pszProxyFile) * sizeof(CHAR) + 1;

	printf("pszProxyFile: %s", pszProxyFile);

	if (GetOSVerion() == 2)
	{
		EnableDebugPrivilege();  // 提权
		printf("EnableDebugPrivilege XP系统提权函数已调用...");
	}

	do
	{
		OutputDebugStringA("InjectDllProxy after OpenProcess...\n");
		hProcess = OpenProcess(PROCESS_ALL_ACCESS/* | PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE*/, FALSE, dwPid);

		if (nullptr == hProcess)
		{
			printf("OpenProcess 失败, 错误码：%d", GetLastError());
			break;
		}

		printf("InjectDllProxy after VirtualAllocEx...");
		PSTR pwszPara = static_cast<PSTR>(VirtualAllocEx(hProcess, nullptr, iPorxyFileLen, MEM_COMMIT, PAGE_READWRITE));
		if (nullptr == pwszPara)
		{
			printf("VirtualAllocEx 失败, 错误码：%d", GetLastError());
			break;
		}

		printf("InjectDllProxy after WriteProcessMemory...\n");
		if (!WriteProcessMemory(hProcess, pwszPara, (PVOID)pszProxyFile, iPorxyFileLen, nullptr))
		{
			printf("WriteProcessMemory 失败, 错误码：%d", GetLastError());
			break;
		}

		printf("InjectDllProxy after GetProcAddress...\n");
		FARPROC pfnThreadBtn = GetProcAddress(GetModuleHandleA("Kernel32.dll"), "LoadLibraryA");
		if (nullptr == pfnThreadBtn)
		{
			printf("GetProcAddress 失败, 错误码：%d", GetLastError());
			break;
		}

		printf("InjectDllProxy after CreateRemoteThread...\n");
		HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0, (LPTHREAD_START_ROUTINE)pfnThreadBtn, pwszPara, 0, nullptr);
		if (nullptr == hThread)
		{
			printf("CreateRemoteThread 失败, 错误码：%d", GetLastError());
			break;
		}

		printf("InjectDllProxy  success...\n");
		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);
		VirtualFreeEx(hProcess, pwszPara, 0, MEM_RELEASE);

		bRet = true;
	} while (false);

	if (hProcess)
	{
		CloseHandle(hProcess);
	}
	OutputDebugStringA("InjectDllProxy  Out...\n");
	return bRet;
}

static bool UnInjectDllProxy(DWORD dwPid, PCSTR pszProxyFile)
{
	bool bRet = false;
	HANDLE hProcess = nullptr, hThread = nullptr, hthSnapshot = nullptr;
	MODULEENTRY32 hMod = { sizeof(hMod) };

	do
	{
		hthSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPid);
		if (hthSnapshot == nullptr)
		{
			break;
		}

		BOOL bMoreMods = Module32First(hthSnapshot, &hMod);
		if (bMoreMods == FALSE)
		{
			break;
		}

		for (; bMoreMods; bMoreMods = Module32Next(hthSnapshot, &hMod))
		{
			if ((!_stricmp(hMod.szExePath, pszProxyFile)) || (!_stricmp(hMod.szModule, pszProxyFile)))
			{
				bRet = true;
				break;
			}
		}

		if (!bRet)
		{
			break;
		}

		hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION, FALSE, dwPid);
		if (hProcess == nullptr)
		{
			break;
		}

		PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle("Kernel32.dll"), "FreeLibrary");
		if (pfnThreadRtn == nullptr)
		{
			break;
		}

		hThread = CreateRemoteThread(hProcess, nullptr, 0, pfnThreadRtn, hMod.modBaseAddr, 0, nullptr);
		if (hThread == nullptr)
		{
			break;
		}

		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);

		bRet = true;
	}
	while (false);

	if (hthSnapshot)
	{
		CloseHandle(hthSnapshot);
	}
	if (hProcess)
	{
		CloseHandle(hProcess);
	}

	return bRet;
}

//static DWORD GetPidByProcessName(const std::string &strTargetProcessName)
//{
//	DWORD dwPid = 0;
//
//	CTopEnumer topEnumer;
//	for (size_t i=0; i<topEnumer.Windows().size(); ++i)
//	{
//		HWND hwnd = topEnumer.Windows()[i];
//		DWORD dwProcessId = 0;
//		GetWindowThreadProcessId(hwnd, &dwProcessId);
//		if (true)
//		{
//			std::string strProcessName = CProcessUtil::GetProcessNameByPid(dwProcessId);
//			if (strProcessName == strTargetProcessName)
//			{
//				dwPid = dwProcessId;
//				break;
//			}
//		}
//	}
//
//	return dwPid;
//}

bool InjectDllToTarget(const std::string &strTargetName, const std::string &strDllName)
{
	bool bRet = false;

	printf("strTargetName: %s          strDllName: %s", strTargetName.c_str(), strDllName.c_str());

	DWORD dwPid = 0;
	HWND hWnd = FindWindow(nullptr, "InjectMe");
	if (hWnd)
	{
		DWORD dwThreadId = GetWindowThreadProcessId(hWnd, &dwPid);
	}

	//dwPid = CProcessUtil::GetProcessPidByName(TARGET_PROCESS_NAME);
	if (0 == dwPid)
	{
		//DLOG_INFO("没有找到目标，错误码:%d", GetLastError());
	}
	else
	{
		printf("找到目标，dwPid:%d", dwPid);
		bRet = InjectDllProxy(dwPid, strDllName.c_str());
	}

	return bRet;
}

//bool UnInjectDllFromTarget(const std::string & strTargetName, const std::string & strDllName)
//{
//	bool bRet = false;
//
//	DWORD dwPid = GetPidByProcessName(strTargetName);
//	if (0 == dwPid)
//	{
//		printf("没有找到目标，错误码:%d", GetLastError());
//	}
//	else
//	{
//		bRet = UnInjectDllProxy(dwPid, strDllName.c_str());
//	}
//
//	return bRet;
//}



