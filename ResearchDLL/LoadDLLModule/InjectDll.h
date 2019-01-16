#pragma once
#include <string>

#define TARGET_PROCESS_NAME		"kp.exe"
#define DLL_NAME				"Hecate.dll"


bool InjectDllToTarget(const std::string &strTargetName, const std::string &strDllName);
//bool UnInjectDllFromTarget(const std::string &strTargetName, const std::string &strDllName);




