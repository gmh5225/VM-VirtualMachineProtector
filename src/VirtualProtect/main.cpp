// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <windows.h>
#include <cstdio>
#include <ctime>
#include <tchar.h>
#include "protect.h"
#include "StringOperator.h"
#include "Error.h"
#include "PEUtils.h"
#include "PE.h"

#include "resource.h"

#pragma warning (disable:4996)
#pragma warning(disable:4838)
#pragma warning(disable:4309)

#define MAKE_VM_CALL(imgBase, funcAddr, funcRVA, vmedFuncAddr, fSize, vmStartRVA) \
	*(BYTE*)(funcAddr) = 0x68; \
	*(DWORD*)((BYTE*)(funcAddr) + 1) = (vmedFuncAddr); \
	*((BYTE*)(funcAddr) + 5) = 0x68; \
	*(DWORD*)((BYTE*)(funcAddr) + 6) = imgBase + (funcRVA) + 16; \
	*((BYTE*)(funcAddr) + 10) = 0x68; \
	*(DWORD*)((BYTE*)(funcAddr) + 11) = imgBase + (vmStartRVA); \
	*((BYTE*)(funcAddr) + 15) = 0xC3; \
	memset((BYTE*)(funcAddr) + 16, 0x90, (fSize) - 16);

#define MAKE_VM_CALL2(imgBase, funcAddr, funcRVA, vmedFuncRVA, fSize, vmStartRVA, inLdrAddr, inLdrRVA) \
	*(BYTE*)(funcAddr) = 0xE9; \
	*(DWORD*)((BYTE*)(funcAddr) + 1) = (inLdrRVA) - (funcRVA) - 5; \
	memset((BYTE*)(funcAddr) + 5, 0x90, (fSize) - 5); \
	*(BYTE*)(inLdrAddr) = 0xE8; \
	*(DWORD*)((BYTE*)(inLdrAddr) + 1) = 0; \
	*((BYTE*)(inLdrAddr) + 5) = 0x9C; \
	*(DWORD*)((BYTE*)(inLdrAddr) + 6) = 0x04246C81; \
	*(DWORD*)((BYTE*)(inLdrAddr) + 10) = (inLdrRVA) - (vmedFuncRVA) + 5; \
	*((BYTE*)(inLdrAddr) + 14) = 0x9D; \
	*((BYTE*)(inLdrAddr) + 15) = 0xE8; \
	*(DWORD*)((BYTE*)(inLdrAddr) + 16) = 0; \
	*((BYTE*)(inLdrAddr) + 20) = 0x9C; \
	*(DWORD*)((BYTE*)(inLdrAddr) + 21) = 0x04246C81; \
	*(DWORD*)((BYTE*)(inLdrAddr) + 25) = (inLdrRVA) - ((funcRVA) + 5) + 20; \
	*((BYTE*)(inLdrAddr) + 29) = 0x9D; \
	*(BYTE*)((BYTE*)(inLdrAddr) + 30) = 0xE9; \
	*(DWORD*)((BYTE*)(inLdrAddr) + 31) = (vmStartRVA) - ((inLdrRVA) + 30) - 5;

#define VM_CALL_SIZE 35

//-----------------------------------------------------------------------------
DWORD ddFrom;
DWORD ddTo;
int WINAPI AddDialogProc(HWND hDlg, UINT uMSg, WPARAM wParam, LPARAM lParam)
{
	switch (uMSg)
	{
		case WM_CLOSE:
			EndDialog(hDlg, 0);
			break;
		case WM_COMMAND:
			{
				switch (wParam & 0xFFFF)
				{
					case BTN_ADD_NO:						
						EndDialog(hDlg, 0);
						break;
					case BTN_ADD_YES:
						wchar_t temp[20];
						ddFrom = 0;
						ddTo = 0;
						GetDlgItemText(hDlg, EDT_FROM, temp, 20);
						StringOperator<wchar_t*> cTemp(temp);
						sscanf(cTemp.wchar2char(), "%x", &ddFrom);
						GetDlgItemText(hDlg, EDT_TO, temp, 20);
						sscanf(cTemp.wchar2char(), "%x", &ddTo);
						if (ddTo && ddFrom) EndDialog(hDlg, 1);
						else MessageBox(hDlg, L"Error", L"Error", MB_ICONERROR);
						break;
				}
			}
			break;
	}
	return 0;
}


#define TRUNC(a, b) (a + (b - ((a % b) ? (a % b) : b)))

void doProtect(HWND listBox, wchar_t* fileName)
{

	DWORD tmp;

	PE protectedFile(fileName);
	BYTE* hInMem = protectedFile.GetPEHandle();
	IMAGE_NT_HEADERS* ntHeaders = protectedFile.GetNtHeaders();
	IMAGE_SECTION_HEADER* sectionHeaders = protectedFile.GetSectionHeaders();
	DWORD rel = protectedFile.GetBaseRelocationTable();
	DWORD fSize = protectedFile.GetPEFileSize();

	//build protection table:
	int itemsCnt = SendMessage(listBox, LB_GETCOUNT, 0, 0);
	if (!itemsCnt) Error(TEXT("Nothing to protect (add at least one range)."));
	DWORD* protectedCodeOffset = (DWORD*)GlobalAlloc(GMEM_FIXED, itemsCnt*8);
	DWORD* items2 = (DWORD*)GlobalAlloc(GMEM_FIXED, itemsCnt*8);
	//vm init
	BYTE* hVMMemory;
	DWORD vmInit;
	DWORD vmStart;
	srand(time(NULL));
	PE virtualMachineFile(TEXT("VirtualLoader.exe"));
	BYTE* hvmMemory = virtualMachineFile.GetPEHandle() + virtualMachineFile.GetSectionHeaders()->PointerToRawData;

	int vmSize = vm_init(&hVMMemory, &vmInit, &vmStart, hvmMemory);
	//
	int protSize = 0;
	for (int i = 0; i < itemsCnt; i++)
	{
		wchar_t temp[25];
		SendMessage(listBox, LB_GETTEXT, i, (LPARAM)temp);
		temp[8] = 0;
		swscanf(temp, TEXT("%x"), &protectedCodeOffset[i*2]);
		swscanf(temp + 11, TEXT("%x"), &protectedCodeOffset[i*2 + 1]);
		/*
		* items[0] = 401896
		* items[1] = 4018BA
		*/
		protectedCodeOffset[i*2] -= ntHeaders->OptionalHeader.ImageBase;
		protectedCodeOffset[i*2 + 1] -= ntHeaders->OptionalHeader.ImageBase;

		items2[i*2] = RvaToRaw(ntHeaders->FileHeader.NumberOfSections, sectionHeaders, protectedCodeOffset[i*2]);
		if (items2[i*2] == 0xFFFFFFFF) ERROR2(TEXT("Invalid range start"));
		items2[i*2 + 1] = protectedCodeOffset[i*2 + 1] - protectedCodeOffset[i*2];		//size
		int t = vm_protect(hInMem + items2[i*2], items2[i*2 + 1], 0, protectedCodeOffset[i*2], (BYTE*)rel, ntHeaders->OptionalHeader.ImageBase);
		if (t == -1) ERROR2(TEXT("[SIZE] Protection failed."));
		protSize += t;
	}
	//loader size = 0x3C

	//first protection layer
	//get new section rva
	DWORD newRVA = (sectionHeaders + ntHeaders->FileHeader.NumberOfSections - 1)->VirtualAddress + TRUNC((sectionHeaders + ntHeaders->FileHeader.NumberOfSections - 1)->Misc.VirtualSize, ntHeaders->OptionalHeader.SectionAlignment);	
	DWORD vAlloc = SearchFunction(hInMem, "VirtualAlloc");// 如果返回空，无找到函数
	//DWORD newSecSize = TRUNC((vmSize + protSize + 0x3C + itemsCnt*0x1F), inh->OptionalHeader.FileAlignment);
	DWORD newSecSize = vmSize + protSize + 0x3C + itemsCnt*VM_CALL_SIZE;
	BYTE* hNewMem = (BYTE*)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, newSecSize);
	int curPos = 0;
	memmove(hNewMem, hVMMemory, vmSize);
	curPos += vmSize;

	//setting new entry point
	DWORD oldEntry = ntHeaders->OptionalHeader.AddressOfEntryPoint;
	ntHeaders->OptionalHeader.AddressOfEntryPoint = newRVA + vmSize;

	//VirtualAlloc at 0xA0F8
	static char loaderAlloc[] = 
	{
		0xE8, 0x00, 0x00, 0x00, 0x00,       //CALL    vm_test2.004120DA
		0x5B,                               //POP     EBX
		0x81, 0xEB, 0xDA, 0x20, 0x01, 0x00, //SUB     EBX,120DA
		0x6A, 0x40,							//PUSH    40
		0x68, 0x00, 0x10, 0x00, 0x00,       //PUSH    1000
		0x68, 0x00, 0x10, 0x00, 0x00,       //PUSH    1000
		0x6A, 0x00,                         //PUSH    0
		0xB8, 0x34, 0x12, 0x00, 0x00,       //MOV     EAX,1234
		0x03, 0xC3,                         //ADD     EAX,EBX
		0xFF, 0x10,                         //CALL    [EAX]
		0x53,                               //PUSH    EBX
		0x05, 0x00, 0x10, 0x00, 0x00,       //ADD     EAX,1000
		0x50,                               //PUSH    EAX
		0xB8, 0x34, 0x12, 0x00, 0x00,       //MOV     EAX,1234
		0x03, 0xC3,                         //ADD     EAX,EBX
		0xFF, 0xD0,                         //CALL    EAX
		0xB8, 0x34, 0x12, 0x00, 0x00,       //MOV     EAX,1234
		0x03, 0xC3,                         //ADD     EAX,EBX
		0xFF, 0xE0                          //JMP     EAX
	};
	//correcting loader:
	*(DWORD*)(loaderAlloc + 8) = newRVA + vmSize + 5;
	*(DWORD*)(loaderAlloc + 27) = vAlloc;
	*(DWORD*)(loaderAlloc + 43) = newRVA + vmInit;
	*(DWORD*)(loaderAlloc + 52) = oldEntry;
	memmove(hNewMem + vmSize, loaderAlloc, sizeof(loaderAlloc));
	curPos += sizeof(loaderAlloc);

	for (int i = 0; i < itemsCnt; i++)
	{
		int _tts = vm_protect(hInMem + items2[i*2], items2[i*2 + 1], hNewMem + curPos, protectedCodeOffset[i*2], (BYTE*)rel, ntHeaders->OptionalHeader.ImageBase);
		MAKE_VM_CALL2(ntHeaders->OptionalHeader.ImageBase, hInMem + items2[i*2], protectedCodeOffset[i*2], newRVA + curPos, items2[i*2 + 1], newRVA + vmStart, hNewMem + curPos + _tts, newRVA + curPos + _tts);
		curPos += _tts + VM_CALL_SIZE;
	}

	
	DWORD oldNewSecSize = newSecSize;
	newSecSize = TRUNC(newSecSize, ntHeaders->OptionalHeader.FileAlignment);	

	//adding section	
	sectionHeaders += ntHeaders->FileHeader.NumberOfSections;
	ntHeaders->FileHeader.NumberOfSections++;
	memset(sectionHeaders, 0, sizeof(IMAGE_SECTION_HEADER));
	sectionHeaders->Characteristics = 0xE00000E0;
	sectionHeaders->Misc.VirtualSize = TRUNC(newSecSize, ntHeaders->OptionalHeader.SectionAlignment);
	ntHeaders->OptionalHeader.SizeOfImage += sectionHeaders->Misc.VirtualSize;
	memmove(sectionHeaders->Name, ".VM", 4);
	sectionHeaders->PointerToRawData = fSize;
	sectionHeaders->SizeOfRawData = newSecSize;
	sectionHeaders->VirtualAddress = newRVA;

	char newFName[MAX_PATH] = {0};
	StringOperator<wchar_t*> cFileName(fileName);
	strcpy(newFName, cFileName.wchar2char());
	newFName[strlen(newFName) - 4] = 0;
	strcat(newFName, "_vmed.exe");
	
	StringOperator<char*> wNewFName(newFName);
	HANDLE hFile = CreateFile(wNewFName.char2wchar(), GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	WriteFile(hFile, hInMem, fSize, &tmp, 0);
	WriteFile(hFile, hNewMem, oldNewSecSize, &tmp, 0);
	memset(hNewMem, 0, newSecSize - oldNewSecSize);
	WriteFile(hFile, hNewMem, newSecSize - oldNewSecSize, &tmp, 0);
	CloseHandle(hFile);

	GlobalFree(hNewMem);
	GlobalFree(protectedCodeOffset);
	GlobalFree(items2);
}

int WINAPI DialogProc(HWND hDlg, UINT uMSg, WPARAM wParam, LPARAM lParam)
{
	switch (uMSg)
	{
		case WM_CLOSE:
			EndDialog(hDlg, 0);
			break;
		case WM_COMMAND:
			{
				switch (wParam & 0xFFFF)
				{
					case BTN_EXIT:
						EndDialog(hDlg, 0);
						break;
					case BTN_ADD:
						if (DialogBoxParam((HINSTANCE)0x400000, (LPCWSTR)IDD_DIALOG2, hDlg, AddDialogProc, 0))
						{
							char temp[50];
							sprintf(temp, "%08X - %08X", ddFrom, ddTo);
							StringOperator<char*> wTemp(temp);
							SendDlgItemMessage(hDlg, LB_LIST, LB_ADDSTRING, 0, (LPARAM)wTemp.char2wchar());
						}
						break;
					case BTN_OPEN:
						{
							OPENFILENAME ofn;
							memset(&ofn, 0, sizeof(OPENFILENAME));
							ofn.lStructSize = sizeof(OPENFILENAME);
							ofn.hwndOwner = hDlg;
							wchar_t fileName[MAX_PATH] = {0};
							ofn.lpstrFile = fileName;
							ofn.nMaxFile = MAX_PATH;
							ofn.Flags = OFN_FILEMUSTEXIST;
							GetOpenFileName(&ofn);
							SetDlgItemText(hDlg, EDT_FILE, fileName);
							SendDlgItemMessage(hDlg, LB_LIST, LB_RESETCONTENT, 0, 0);
						}
						break;
					case BTN_PROTECT:
						{
							wchar_t fileName[MAX_PATH];
							if (GetDlgItemText(hDlg, EDT_FILE, fileName, MAX_PATH))
								doProtect(GetDlgItem(hDlg, LB_LIST), fileName);

							MessageBox(hDlg, L"Finished.", L"Info", MB_ICONINFORMATION);
						}
						break;
				}
			}
			break;
	}
	return 0;
}

int WINAPI WinMain(__in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in_opt LPSTR lpCmdLine, __in int nShowCmd)
{
	DialogBoxParam(hInstance, (LPCWSTR)IDD_DIALOG1, 0, DialogProc, 0);

	return (int)VirtualAlloc;
}