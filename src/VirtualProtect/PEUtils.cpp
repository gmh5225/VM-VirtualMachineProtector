#include "Error.h"
#include "PEUtils.h"

DWORD RvaToRaw(WORD numOfSections, IMAGE_SECTION_HEADER* FSH, DWORD rva)
{
	for (int i = numOfSections - 1; i >= 0; i--)
		if (FSH[i].VirtualAddress <= rva)
			return FSH[i].PointerToRawData + rva - FSH[i].VirtualAddress;
	return 0xFFFFFFFF;
}

DWORD SearchFunction(BYTE* exeMem, const char* functionName)
{
	IMAGE_NT_HEADERS* ntHeaders = (IMAGE_NT_HEADERS*)(exeMem + ((IMAGE_DOS_HEADER*)exeMem)->e_lfanew);
	IMAGE_SECTION_HEADER* sectionHeaders = (IMAGE_SECTION_HEADER*)(exeMem + ((IMAGE_DOS_HEADER*)exeMem)->e_lfanew + ntHeaders->FileHeader.SizeOfOptionalHeader + sizeof(IMAGE_FILE_HEADER) + 4);

	IMAGE_IMPORT_DESCRIPTOR* importTables = (IMAGE_IMPORT_DESCRIPTOR*)ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	importTables = (IMAGE_IMPORT_DESCRIPTOR*)(exeMem + RvaToRaw(ntHeaders->FileHeader.NumberOfSections, sectionHeaders, (DWORD)importTables));
	bool found = false;
	while (!found && importTables->Name)
	{
		char* libName = (char*)exeMem + RvaToRaw(ntHeaders->FileHeader.NumberOfSections, sectionHeaders, importTables->Name);
		if (!_stricmp(libName, "kernel32.dll"))
		{
			found = true;
			DWORD* thunks = (DWORD*)(importTables->FirstThunk);
			thunks = (DWORD*)(exeMem + RvaToRaw(ntHeaders->FileHeader.NumberOfSections, sectionHeaders, importTables->FirstThunk));
			bool found2 = false;
			int k = 0;
			while (!found2 && *thunks)
			{
				char* curName = (char*)exeMem + RvaToRaw(ntHeaders->FileHeader.NumberOfSections, sectionHeaders, *thunks);
				if (!_stricmp(curName + 2, functionName))
				{
					return importTables->FirstThunk + k * 4;
					found2 = true;
				}
				k++;
				thunks++;
			}
		}
		importTables++;
	}
	return NULL;
}

BYTE* ReadPEToMemory(const std::wstring pathName, const int type)
{
	HANDLE hFile = CreateFile(pathName.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE) Error(TEXT("Cannot open input file."));
	DWORD tmp;
	DWORD fSize = GetFileSize(hFile, 0);
	BYTE* hInMem = (BYTE*)GlobalAlloc(GMEM_FIXED, fSize);
	if (!hInMem) 
		Error(TEXT("Cannot allocate memory."));
	ReadFile(hFile, hInMem, fSize, &tmp, 0);
	CloseHandle(hFile);
	return hInMem;
}