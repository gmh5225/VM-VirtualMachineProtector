#include "PE.h"
#include "Error.h"
#include "PEUtils.h"

PE::PE(const std::wstring pathName)
	:m_moduleHandle(NULL),m_fileSize(NULL),m_ok(TRUE)
{
	/*
	* 加载PE文件到内存中，ReadFile会采用内存对齐
	*/
	HANDLE hFile = CreateFile(pathName.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		m_ok = false;
		Error(TEXT("Cannot open input file."));
	}

	DWORD tmp;
	m_fileSize = GetFileSize(hFile, 0);
	try
	{
		m_moduleHandle = (BYTE*)malloc(m_fileSize);// 后期更换为new
	}
	catch (const std::exception&)
	{
		free(m_moduleHandle);
	}

	if (!m_moduleHandle)
	{
		m_ok = false;
		Error(TEXT("Cannot allocate memory."));
	}
	m_ok = ReadFile(hFile, m_moduleHandle, m_fileSize, &tmp, 0);
	CloseHandle(hFile);
}

BYTE* PE::GetPEHandle()
{
	return m_moduleHandle;
}

DWORD PE::GetPEFileSize()
{
	return m_fileSize;
}

PIMAGE_NT_HEADERS PE::GetNtHeaders()
{
	PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)(m_moduleHandle + ((IMAGE_DOS_HEADER*)m_moduleHandle)->e_lfanew);
	return ntHeaders;
}


WORD PE::GetNumberOfSections()
{
	return PE::GetNtHeaders()->FileHeader.NumberOfSections;
}

PIMAGE_SECTION_HEADER PE::GetSectionHeaders()
{
	IMAGE_SECTION_HEADER* sectionHeaders = (IMAGE_SECTION_HEADER*)(m_moduleHandle + ((IMAGE_DOS_HEADER*)m_moduleHandle)->e_lfanew + PE::GetNtHeaders()->FileHeader.SizeOfOptionalHeader + sizeof(IMAGE_FILE_HEADER) + 4);
	//IMAGE_SECTION_HEADER* sectionHeaders = (IMAGE_SECTION_HEADER*)(PE::GetNtHeaders() + sizeof(PE::GetNtHeaders()->Signature) + sizeof(IMAGE_FILE_HEADER) + sizeof(IMAGE_OPTIONAL_HEADER));
	return sectionHeaders;
}

DWORD PE::GetBaseRelocationTable()
{
	DWORD dwRelocation = 0;
	if (PE::GetNtHeaders()->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress)
	{
		// PE::GetNtHeaders()->FileHeader.NumberOfSections 用 PE::GetNumberOfSections()替换
		dwRelocation = RvaToRaw(PE::GetNtHeaders()->FileHeader.NumberOfSections, PE::GetSectionHeaders(), PE::GetNtHeaders()->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
		if (dwRelocation == 0xFFFFFFFF)
		{
			m_ok = false;
			Error(TEXT("Invalid relocations RVA."));
		}
		dwRelocation += (DWORD)m_moduleHandle;

	}
	return dwRelocation;
}

BOOL PE::GetResult()
{
	return m_ok;
}

PE::~PE()
{
	free(m_moduleHandle);
}