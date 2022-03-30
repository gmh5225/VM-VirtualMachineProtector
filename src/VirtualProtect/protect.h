


#ifndef _PROTECT_H_
#define _PROTECT_H_

#include <cstdio>
#include <ctime>
#include <Windows.h>
#include "macros.h"
#include "common.h"

int vm_init(BYTE** retMem, DWORD* _vmInit, DWORD* _vmStart, BYTE* hvmMemory);
BYTE* vm_getVMImg();
DWORD vm_getVMSize();
int vm_protect(BYTE* codeBase, int codeSize, BYTE* outCodeBuf, DWORD inExeFuncRVA, BYTE* relocBuf, DWORD imgBase);

#endif