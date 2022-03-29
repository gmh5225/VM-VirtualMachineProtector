// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "Error.h"
#include <Windows.h>

void Error(const std::wstring msg)
{
	MessageBox(0, msg.c_str(), TEXT("Error"), MB_ICONERROR); return;
}
