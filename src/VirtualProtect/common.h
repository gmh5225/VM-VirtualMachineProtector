

#ifndef _COMMON_H_
#define _COMMON_H_

#include <windows.h>
#include "hde.h"


DWORD WINAPI _lde(BYTE* off);
//extern "C" DWORD WINAPI lde(BYTE* off);
#define lde _lde

typedef void (__stdcall *polyFunc)(BYTE* buf, DWORD size, DWORD pos);
extern polyFunc polyEnc;
extern polyFunc polyDec;
extern BYTE _vm_poly_dec[121];

int GetCodeMap(BYTE* codeBase, int codeSize, DWORD* codeMap);
void GetPolyEncDec();
void GetPermutation(BYTE* buf, int size);
void KeyToValue256(BYTE* buf);
void KeyToValue16(BYTE* buf);
void permutateJcc(WORD* buf, int elemCount, BYTE* permutation);
int GetRelocMap(const BYTE* relocSeg, DWORD funcRVA, int funcSize, DWORD* relocMap);

#endif