#include "Memory.h"

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

uintptr_t Memory::GetModuleBaseAddr(const char * moduleName)
{
	return (uintptr_t)GetModuleHandleA(moduleName);
}
