#pragma once
#include "Singleton.h"
#include "GlobalVars.h"

class Memory : public Singleton <Memory>
{
public:
	DWORD getProcessId(HWND hWnd);
	DWORD getProcessId(const char * className, const char * windowName);
	uintptr_t GetModuleBaseAddr(const char * moduleName);
	template <typename T> void write(uintptr_t addres, T buffer);
	template <typename T> T read(uintptr_t addres);
};

template<class T> inline T Memory::read(uintptr_t addres)
{
	if (!IsBadReadPtr((PVOID)addres, sizeof(T)))
	{
		return *(T *)addres;
	}
	T buffer{};
	return buffer;
}

template<class T> inline void Memory::write(uintptr_t addres, T value)
{
	if (!IsBadReadPtr((PVOID)addres, sizeof(T)))
	{
		*(T *)addres = value;
	}
}
