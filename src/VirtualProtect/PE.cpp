#include "PE.h"
#include "Error.h"

PE::PE(const std::wstring pathName, bool &ok)
	:m_moduleHandle(NULL)
{
	/*
	* 加载PE文件到内存中，ReadFile会采用内存对齐
	*/
	HANDLE hFile = CreateFile(pathName.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		ok = false;
		Error(TEXT("Cannot open input file."));
	}

	DWORD tmp;
	DWORD fSize = GetFileSize(hFile, 0);
	try
	{
		m_moduleHandle = (BYTE*)malloc(fSize);// 后期更换为new
	}
	catch (const std::exception&)
	{
		free(m_moduleHandle);
	}

	if (!m_moduleHandle)
	{
		ok = false;
		Error(TEXT("Cannot allocate memory."));
	}
	ok = ReadFile(hFile, m_moduleHandle, fSize, &tmp, 0);
	CloseHandle(hFile);
}

PE::~PE()
{
	free(m_moduleHandle);
}