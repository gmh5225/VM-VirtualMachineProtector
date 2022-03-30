#pragma once
#ifndef __PE_H__
#define __PE_H_
#include<iostream>
#include<Windows.h>

class PE
{
public:
	PE(const std::wstring pathName);
	BYTE* GetPEHandle();
	BOOL GetResult();
	DWORD GetPEFileSize();
	PIMAGE_NT_HEADERS GetNtHeaders();
	PIMAGE_FILE_HEADER GetFileHeaders();
	PIMAGE_OPTIONAL_HEADER GetOptionalHeaders();
	WORD GetNumberOfSections();
	PIMAGE_SECTION_HEADER GetSectionHeaders();
	DWORD GetBaseRelocationTable();
	~PE();

private:
	BYTE* m_moduleHandle;
	DWORD m_fileSize;
	BOOL m_ok;
};


#endif // !__PE_H__
