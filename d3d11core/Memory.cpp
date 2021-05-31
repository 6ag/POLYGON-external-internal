﻿#include "Memory.h"
#include <TlHelp32.h>

#if defined(_MSC_VER)
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE (1)
#endif
#pragma warning(disable : 4996)
#endif

DWORD Memory::getProcessId(const char * processName)
{
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);
	HANDLE hDump = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (Process32First(hDump, &pe32))
	{
		do
		{
			if (!stricmp(processName, pe32.szExeFile))
			{
				CloseHandle(hDump);
				return pe32.th32ProcessID;
			}
		}
		while (Process32Next(hDump, &pe32));
	}
	return 0;
}

DWORD Memory::getProcessId(HWND hWnd)
{
	DWORD pId;
	GetWindowThreadProcessId(hWnd, &pId);
	return pId;
};

DWORD Memory::getProcessId(const char * className, const char * windowName)
{
	DWORD pId;
	HWND hWnd = FindWindow(className, windowName);
	if (hWnd == NULL)
	{
		return 0;
	}
	GetWindowThreadProcessId(hWnd, &pId);
	return pId;
};

HANDLE Memory::getProcessHandle(DWORD pId)
{
	return OpenProcess(PROCESS_ALL_ACCESS, FALSE, pId);
}

uintptr_t Memory::GetModuleBaseAddr(const char * moduleName)
{
#ifdef EXTERNAL_DRAW
	MODULEENTRY32 me32;
	me32.dwSize = sizeof(MODULEENTRY32);
	HANDLE hDump = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, GlobalVars::get().pId);
	if (Module32First(hDump, &me32))
	{
		do
		{
			if (!stricmp(moduleName, me32.szModule))
			{
				CloseHandle(hDump);
				return (uintptr_t)me32.modBaseAddr;
			}
		}
		while (Module32Next(hDump, &me32));
	}
	return -1337;
#else
	return (uintptr_t)GetModuleHandleA(moduleName);
#endif
}
