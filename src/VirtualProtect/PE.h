#pragma once
#ifndef __PE_H__
#define __PE_H_
#include<iostream>
#include<Windows.h>

class PE
{
public:
	PE(const std::wstring pathName, bool& ok);
	DWORD GetNtHeaders();
	DWORD GetFileHeaders();
	DWORD GetOptionalHeaders();
	DWORD GetSectionHeaders();
	~PE();

private:
	BYTE* m_moduleHandle;
};


#endif // !__PE_H__
