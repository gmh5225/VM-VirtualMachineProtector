


#ifndef _PROTECT_H_
#define _PROTECT_H_

#include <cstdio>
#include <ctime>
#include <Windows.h>
#include "macros.h"
#include "common.h"
#include "PE.h"


int vm_init(BYTE** retMem, DWORD* _vmInit, DWORD* _vmStart, BYTE* hvmMemory);

int vm_protect(BYTE* codeBase, int codeSize, BYTE* outCodeBuf, DWORD inExeFuncRVA, 
	const BYTE* relocBuf, DWORD imgBase);

#define VM_CALL_SIZE 35

void MAKE_VM_CALL2(BYTE* funcAddr, DWORD funcRVA, DWORD vmedFuncRVA, 
	DWORD fSize, DWORD vmStartRVA, BYTE* inLdrAddr, DWORD inLdrRVA);
DWORD SectionAlignment(DWORD a, DWORD b);
int GetVMByteCodeSize(int itemsCnt, DWORD* items2, HWND listBox, DWORD* protectedCodeOffset, PE& protectedFile);
bool MemoryWriteToFile(wchar_t* fileName, PE& protectedFile, DWORD oldNewSecSize, DWORD newSecSize, BYTE* hNewMem);
bool AddSection(IMAGE_SECTION_HEADER* sectionHeaders, PE& protectedFile, DWORD newRVA, DWORD newSecSize);

bool SetVMEntryPoint(PE& protectedFile, DWORD newRVA, int vmSize, DWORD vmInit, BYTE* hNewMem, int& curPos);
bool SetByteCode(PE& protectedFile, int& curPos, DWORD newRVA, DWORD vmStart, DWORD* protectedCodeOffset, 
	int itemsCnt, DWORD* items2, BYTE* hNewMem);
#endif