#pragma once
#ifndef __ERROR_H__
#define __ERROR_H__
#include <iostream>
#include <assert.h>
void Error(const std::wstring);


// ERROR2 ´ýÖØ¹¹
#define ERROR2(a) \
	{ \
		MessageBox(0, a, TEXT("Error"), MB_ICONERROR); \
		free(protectedCodeRVA); \
		free(protectedCoedFOA); \
		return; \
	}
#endif // !__ERROR_H__

