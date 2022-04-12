// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <windows.h>
#include <cstdio>
#include <ctime>
#include <tchar.h>
#include "protect.h"
#include "StringOperator.h"
#include "Error.h"
#include "resource.h"

#pragma warning(disable:4996)


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
						if(sscanf(cTemp.wchar2char(), "%x", &ddFrom) != 1)
							break;
						GetDlgItemText(hDlg, EDT_TO, temp, 20);
						if(sscanf(cTemp.wchar2char(), "%x", &ddTo) !=1)
							break;
						if (ddTo && ddFrom) EndDialog(hDlg, 1);
						else MessageBox(hDlg, L"Error", L"Error", MB_ICONERROR);
						break;
				}
			}
			break;
	}
	return 0;
}




void DoProtect(HWND listBox, wchar_t* fileName)
{
	PE protectedFile(fileName);

	//build protection table:
	int itemsSize = SendMessage(listBox, LB_GETCOUNT, 0, 0);
	if (!itemsSize) Error(TEXT("Nothing to protect (add at least one range)."));
	DWORD* protectedCodeRVA = NULL;
	protectedCodeRVA = (DWORD*)malloc(itemsSize * 8);
	if (protectedCodeRVA == NULL)
		return;
	DWORD* protectedCoedFOA = NULL;
	protectedCoedFOA = (DWORD*)malloc(itemsSize * 8);
	if (protectedCoedFOA == NULL)
	{
		free(protectedCodeRVA);
		return;
	}

	srand(time(NULL));
	PE virtualMachineFile(TEXT("VirtualLoader.exe"));

	BYTE* hVMMemory;
	DWORD dwVMInit;
	DWORD dwVMStart;
	int vmSize = vm_init(&hVMMemory, &dwVMInit, &dwVMStart, virtualMachineFile.GetPEHandle() + virtualMachineFile.GetSectionHeaders()->PointerToRawData);

	// 获取虚拟指令需要大小
	int vmByteCodeSize = 0;
	vmByteCodeSize = GetVMByteCodeSize(itemsSize, protectedCoedFOA, listBox, protectedCodeRVA, protectedFile);
	if (vmByteCodeSize == -1)
	{
		ERROR2(TEXT("[SIZE] Protection failed."));
		return;
	}

	DWORD dwNewSectionRVA = (protectedFile.GetSectionHeaders() + protectedFile.GetNtHeaders()->FileHeader.NumberOfSections - 1)->VirtualAddress + SectionAlignment((protectedFile.GetSectionHeaders() + protectedFile.GetNtHeaders()->FileHeader.NumberOfSections - 1)->Misc.VirtualSize, 
		protectedFile.GetNtHeaders()->OptionalHeader.SectionAlignment);
	
	DWORD dwNewSectionSize = vmSize + vmByteCodeSize + 0x3C + itemsSize*VM_CALL_SIZE;
	// 3C 就是VMEntry[]大小
	BYTE* hNewMem = NULL;
	hNewMem = (BYTE*)malloc(dwNewSectionSize);
	if (hNewMem == NULL)
	{
		ERROR2(TEXT("[SIZE] Protection failed."));
		return;
	}
	int curPos = 0;
	memmove(hNewMem, hVMMemory, vmSize);
	curPos += vmSize;

	SetVMEntryPoint(protectedFile, dwNewSectionRVA, vmSize, dwVMInit, hNewMem, curPos);

	SetByteCode(protectedFile, curPos, dwNewSectionRVA, dwVMStart, protectedCodeRVA, itemsSize, protectedCoedFOA, hNewMem);
	
	DWORD dwNewSectionFileAlignmentSize;
	dwNewSectionFileAlignmentSize = SectionAlignment(dwNewSectionSize, protectedFile.GetNtHeaders()->OptionalHeader.FileAlignment);

	AddSection(protectedFile.GetSectionHeaders(), protectedFile, dwNewSectionRVA, dwNewSectionFileAlignmentSize);

	MemoryWriteToFile(fileName, protectedFile, dwNewSectionSize, dwNewSectionFileAlignmentSize, hNewMem);

	free(hNewMem);
	free(protectedCodeRVA);
	free(protectedCoedFOA);
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
								DoProtect(GetDlgItem(hDlg, LB_LIST), fileName);

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