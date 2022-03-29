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

int vm_protect_vm(BYTE* vm_in_exe, BYTE* outBuf, DWORD imgBase, DWORD vmRVA, DWORD newRVA)
{
	BYTE* hVMMemory;
	DWORD vmInit;
	DWORD vmStart;
	int vmSize;
	DWORD* hVMImg;
	if (!outBuf) 
	{
		vmSize = vm_init(&hVMMemory, &vmInit, &vmStart);
		hVMImg = (DWORD*)vm_getVMImg();
	}
	else 
	{
		hVMImg = (DWORD*)vm_getVMImg();
		vmSize = vm_getVMSize();
		DWORD _ssss = (*(DWORD*)(hVMImg + 7))*4 + (*(DWORD*)(hVMImg + 8))*8 + 4;
		vmInit = *(DWORD*)(hVMImg + 2) - _ssss;
		vmStart = *(DWORD*)(hVMImg + 3) - _ssss;
		hVMMemory = (BYTE*)hVMImg + _ssss;
	}

	int cnt = hVMImg[7];
	cnt = hVMImg[cnt];
	int curOutPos = 0;

	if (outBuf) memmove(outBuf, hVMMemory, vmSize);
	curOutPos += vmSize;	

	for (int i = 0; i < cnt; i++)
	{
		int cur = hVMImg[7] + 2*i + 1;
		if (!outBuf)
		{
			hVMImg[cur] -= (0x401000 + hVMImg[7]*4 + cnt*8 + 4);
			hVMImg[cur + 1] -= (0x401000 + hVMImg[7]*4 + cnt*8 + 4);
		}

		int lalala = curOutPos;
		BYTE* __outBuf;
		if (outBuf) __outBuf = outBuf + curOutPos;
		else __outBuf = 0;
		curOutPos += vm_protect(vm_in_exe + hVMImg[cur], hVMImg[cur + 1] - hVMImg[cur], __outBuf, vmRVA + hVMImg[cur], 0, imgBase);
		//MAKE_VM_CALL(imgBase, vm_in_exe + hVMImg[cur], vmRVA + hVMImg[cur], imgBase + newRVA + lalala, hVMImg[cur + 1] - hVMImg[cur], newRVA + vmStart);
		if (outBuf)
		{
			MAKE_VM_CALL2(imgBase, vm_in_exe + hVMImg[cur], vmRVA + hVMImg[cur], newRVA + lalala, hVMImg[cur + 1] - hVMImg[cur], newRVA + vmStart, outBuf + curOutPos, newRVA + curOutPos);
		}
		curOutPos += VM_CALL_SIZE;
	}

	return curOutPos;
}
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

void doProtect(HWND listBox, bool vmovervm, wchar_t* fileName)
{
	/*
	* hInMem是需要加壳文件内存映射
	*/
	HANDLE hFile = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		Error(TEXT("Cannot open input file."));
		return;
	}
	DWORD tmp;
	DWORD fSize = GetFileSize(hFile, 0);
	BYTE* hInMem = (BYTE*)GlobalAlloc(GMEM_FIXED, fSize);
	if (!hInMem)
	{
		Error(TEXT("Cannot allocate memory."));
		return;
	}
	ReadFile(hFile, hInMem, fSize, &tmp, 0);
	CloseHandle(hFile);

	IMAGE_NT_HEADERS* ntHeaders = (IMAGE_NT_HEADERS*)(hInMem + ((IMAGE_DOS_HEADER*)hInMem)->e_lfanew);
	IMAGE_SECTION_HEADER* sectionHeaders = (IMAGE_SECTION_HEADER*)(hInMem + ((IMAGE_DOS_HEADER*)hInMem)->e_lfanew + ntHeaders->FileHeader.SizeOfOptionalHeader + sizeof(IMAGE_FILE_HEADER) + 4);

	//relocs check
	DWORD rel = 0;
	if (ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress)
	{
		rel = RvaToRaw(ntHeaders->FileHeader.NumberOfSections, sectionHeaders, ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
		if (rel == 0xFFFFFFFF) Error(TEXT("Invalid relocations RVA."));
		rel += (DWORD)hInMem;

	}

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
	int vmSize = vm_init(&hVMMemory, &vmInit, &vmStart);
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

	if (vmovervm)
	{
		//chuj wi czemu ;p
		//newSecSize -= 5;
		//
		int vmovmSize = vm_protect_vm(hNewMem, 0, ntHeaders->OptionalHeader.ImageBase, newRVA, newRVA + curPos);
		//BYTE* tmpMemPtr = (BYTE*)GlobalReAlloc(hNewMem, newSecSize + vmovmSize, GMEM_FIXED | GMEM_ZEROINIT);		
		BYTE* tmpMemPtr = (BYTE*)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, newSecSize + vmovmSize + sizeof(loaderAlloc));
		memmove(tmpMemPtr, hNewMem, newSecSize);
		GlobalFree(hNewMem);
		hNewMem = tmpMemPtr;
		vm_protect_vm(hNewMem, hNewMem + newSecSize, ntHeaders->OptionalHeader.ImageBase, newRVA, newRVA + curPos);
		curPos += vmovmSize;
		
		oldEntry = ntHeaders->OptionalHeader.AddressOfEntryPoint;
		ntHeaders->OptionalHeader.AddressOfEntryPoint = newRVA + curPos;

		*(DWORD*)(loaderAlloc + 8) = newRVA + curPos + 5;
		//*(DWORD*)(loaderAlloc + 27) = vAlloc;
		*(DWORD*)(loaderAlloc + 43) = newRVA + newSecSize + vmInit;
		*(DWORD*)(loaderAlloc + 52) = oldEntry;
		memmove(hNewMem + curPos, loaderAlloc, sizeof(loaderAlloc));
		curPos += sizeof(loaderAlloc);

		newSecSize += vmovmSize + sizeof(loaderAlloc);
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
	hFile = CreateFile(wNewFName.char2wchar(), GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	WriteFile(hFile, hInMem, fSize, &tmp, 0);
	WriteFile(hFile, hNewMem, oldNewSecSize, &tmp, 0);
	memset(hNewMem, 0, newSecSize - oldNewSecSize);
	WriteFile(hFile, hNewMem, newSecSize - oldNewSecSize, &tmp, 0);
	CloseHandle(hFile);

	GlobalFree(hNewMem);
	GlobalFree(protectedCodeOffset);
	GlobalFree(items2);
	vm_free();
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
							{
								bool vmovm = false;
								if (SendDlgItemMessage(hDlg, CHK_VMOVM, BM_GETCHECK, 0, 0) == BST_CHECKED) vmovm = true;
								doProtect(GetDlgItem(hDlg, LB_LIST), vmovm, fileName);
							}
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