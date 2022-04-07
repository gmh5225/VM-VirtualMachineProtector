


#ifndef _PROTECT_H_
#define _PROTECT_H_

#include <cstdio>
#include <ctime>
#include <Windows.h>
#include "macros.h"
#include "common.h"

int vm_init(BYTE** retMem, DWORD* _vmInit, DWORD* _vmStart, BYTE* hvmMemory);

int vm_protect(BYTE* codeBase, int codeSize, BYTE* outCodeBuf, DWORD inExeFuncRVA, const BYTE* relocBuf, DWORD imgBase);

#endif