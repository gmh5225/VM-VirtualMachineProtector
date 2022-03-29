#pragma once
#ifndef __PEUTILS_H__
#define __PEUTILS_H__
#include <Windows.h>
#include <iostream>

#define FullLoader 0
#define NoPEHeaderLoader 1

DWORD RvaToRaw(WORD NumOfSections, IMAGE_SECTION_HEADER* FSH, DWORD rva);
DWORD SearchFunction(BYTE* exeMem, const char* functionName);
BYTE* ReadPEToMemory(const std::wstring pathName, const int type);
#endif // !__PEUTILS_H__

