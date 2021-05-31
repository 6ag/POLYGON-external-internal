#pragma once
#include "Singleton.h"
#include "GlobalVars.h"

class Memory : public Singleton <Memory>
{
public:
	DWORD getProcessId(const char * processName);
	DWORD getProcessId(HWND hWnd);
	DWORD getProcessId(const char * className, const char * windowName);
	HANDLE getProcessHandle(DWORD pId);
	uintptr_t GetModuleBaseAddr(const char * moduleName);
	template <typename T> void write(uintptr_t addres, T buffer);
	template <typename T> T read(uintptr_t addres);
};

template<class T> inline T Memory::read(uintptr_t addres)
{
#ifdef EXTERNAL_DRAW
	T buffer;
	ReadProcessMemory(GlobalVars::get().hProcess, LPVOID(addres), &buffer, sizeof(buffer), 0);
	return buffer;
#else
	if (!IsBadReadPtr((PVOID)addres, sizeof(T)))
		return *(T *)addres;
	T buffer;
	return buffer;
#endif
}

template<class T> inline void Memory::write(uintptr_t addres, T buffer)
{
#ifdef EXTERNAL_DRAW
	WriteProcessMemory(GlobalVars::get().hProcess, LPVOID(addres), &buffer, sizeof(buffer), 0);
#else
	if (!IsBadReadPtr((PVOID)addres, sizeof(T)))
		*(T *)addres = buffer;
#endif
}
