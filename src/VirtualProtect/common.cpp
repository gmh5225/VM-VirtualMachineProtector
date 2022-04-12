// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com


#include "common.h"
#include "poly_encdec.h"

extern BYTE _vm_poly_dec[121] = {0};

extern polyFunc polyEnc = (polyFunc)(BYTE*)_vm_poly_enc;
extern polyFunc polyDec = (polyFunc)(BYTE*)_vm_poly_dec;

DWORD WINAPI _lde(BYTE* off)
{
	HDE_STRUCT hdeStr;
	hde_disasm(off, &hdeStr);
	return ((hdeStr.p_66 | hdeStr.p_67 | hdeStr.p_lock | hdeStr.p_rep | hdeStr.p_seg) << 8) | hdeStr.len;
}

void GetPolyEncDec()
{
	//xor eax, dword ptr [esp+18h]
	//0x18244433
	memmove(_vm_poly_dec, _vm_poly_enc, sizeof(_vm_poly_enc));
	*(DWORD*)(_vm_poly_enc + 0x65) = 0x18244433;
	*(DWORD*)(_vm_poly_dec + 11) = 0x18244433;

	//XOR val 0x34 val
	//SUB val 0x2C val
	//ADD val 0x04 val
	//XOR CL  0x32 0xC1
	//SUB CL  0x2A 0xC1
	//ADD CL  0x02 0xC1
	//INC     0xFE 0xC0
	//DEC     0xFE 0xC8
	//ROR CL  0xD2 0xC8
	//ROL CL  0xD2 0xC0
	//junk    0xEB 0x01 xx


	// 加密解密函数是随机的
	int instr = 30;
	int junk = 10;
	int ptr = 11;
	while (instr || junk)
	{
		int w = rand() & 1;
		if (w && junk)
		{
			_vm_poly_enc[ptr] = 0xEB;
			_vm_poly_enc[ptr+1] = 0x01;
			_vm_poly_enc[ptr+2] = rand();	// 花指令
			_vm_poly_dec[114 - ptr - 1] = 0xEB;
			_vm_poly_dec[114 - ptr] = 0x01;
			_vm_poly_dec[114 - ptr + 1] = rand();
			ptr += 3;
			junk--;
		}
		else
		{
			int cinstr = rand() % 10;
			switch (cinstr)
			{
			case 0:
				_vm_poly_enc[ptr] = 0x34;
				_vm_poly_enc[ptr+1] = rand();
				_vm_poly_dec[114 - ptr] = 0x34;
				_vm_poly_dec[114 - ptr+1] = _vm_poly_enc[ptr+1];
				break;
			case 1:
				_vm_poly_enc[ptr] = 0x2C;
				_vm_poly_enc[ptr+1] = rand();
				_vm_poly_dec[114 - ptr] = 0x04;
				_vm_poly_dec[114 - ptr+1] = _vm_poly_enc[ptr+1];
				break;
			case 2:
				_vm_poly_enc[ptr] = 0x04;
				_vm_poly_enc[ptr+1] = rand();
				_vm_poly_dec[114 - ptr] = 0x2C;
				_vm_poly_dec[114 - ptr+1] = _vm_poly_enc[ptr+1];
				break;
			case 3:
				_vm_poly_enc[ptr] = 0x32;
				_vm_poly_enc[ptr+1] = 0xC1;
				_vm_poly_dec[114 - ptr] = 0x32;
				_vm_poly_dec[114 - ptr+1] = 0xC1;
				break;
			case 4:
				_vm_poly_enc[ptr] = 0x2A;
				_vm_poly_enc[ptr+1] = 0xC1;
				_vm_poly_dec[114 - ptr] = 0x02;
				_vm_poly_dec[114 - ptr+1] = 0xC1;
				break;
			case 5:
				_vm_poly_enc[ptr] = 0x02;
				_vm_poly_enc[ptr+1] = 0xC1;
				_vm_poly_dec[114 - ptr] = 0x2A;
				_vm_poly_dec[114 - ptr+1] = 0xC1;
				break;
			case 6:
				_vm_poly_enc[ptr] = 0xFE;
				_vm_poly_enc[ptr+1] = 0xC0;
				_vm_poly_dec[114 - ptr] = 0xFE;
				_vm_poly_dec[114 - ptr+1] = 0xC8;
				break;
			case 7:
				_vm_poly_enc[ptr] = 0xFE;
				_vm_poly_enc[ptr+1] = 0xC8;
				_vm_poly_dec[114 - ptr] = 0xFE;
				_vm_poly_dec[114 - ptr+1] = 0xC0;
				break;
			case 8:
				_vm_poly_enc[ptr] = 0xD2;
				_vm_poly_enc[ptr+1] = 0xC8;
				_vm_poly_dec[114 - ptr] = 0xD2;
				_vm_poly_dec[114 - ptr+1] = 0xC0;
				break;
			case 9:
				_vm_poly_enc[ptr] = 0xD2;
				_vm_poly_enc[ptr+1] = 0xC0;
				_vm_poly_dec[114 - ptr] = 0xD2;
				_vm_poly_dec[114 - ptr+1] = 0xC8;
				break;
			}
			ptr += 2;
			instr--;
		}
	}
}

int GetCodeMap(BYTE* codeBase, int codeSize, DWORD* codeMap)
{
	int curPos = 0;
	int instrCount = 0;
	//disasm_struct dis;
	struct  
	{
		DWORD disasm_len;
	} dis;
	while (curPos != codeSize)
	{		
		//dis.disasm_len = lde(codeBase + curPos) & 0xFF;
		// OPCODE length is 7
		// 8D8405 00000000	lea eax,dword ptr ss:[ebp+eax]
		// 8D9415 00000000	lea edx,dword ptr ss:[ebp+edx]
		// 8D8C0D 00000000	lea ecx,dword ptr ss:[ebp+ecx]
		// 8D9C1D 00000000	lea ebx,dword ptr ss:[ebp+ebx]
		if ((curPos - 3 < codeSize) && 
			((((*(DWORD*)(codeBase + curPos)) & 0xFFFFFF) == 0x05848D) ||
			(((*(DWORD*)(codeBase + curPos)) & 0xFFFFFF) == 0x15948D) ||
			(((*(DWORD*)(codeBase + curPos)) & 0xFFFFFF) == 0x0D8C8D) ||
			(((*(DWORD*)(codeBase + curPos)) & 0xFFFFFF) == 0x1D9C8D)
			)) dis.disasm_len = 7;
		//else if ((curPos - 2 < codeSize) && (*(WORD*)(codeBase + curPos) == 0x01EB)) dis.disasm_len = 3;
		else dis.disasm_len = lde(codeBase + curPos) & 0xFF;		
		//
		if (!dis.disasm_len) return -1;
		if (codeMap) codeMap[instrCount] = curPos;
		instrCount++;
		curPos += dis.disasm_len;
	}
	return instrCount;
}

void GetPermutation(BYTE* buf, int size)
{
	memset(buf, 0, size);
	int i = 0;
	while (i < size)
	{
		BYTE rnd = rand() % size;
		if (!buf[rnd]) 
		{
			buf[rnd] = i;
			i++;
		}
	}
}

void KeyToValue256(BYTE* buf)
{
	BYTE tmp[256];
	for (int i = 0; i < 256; i++)
	{
		tmp[buf[i]] = i; 
	}
	memmove(buf, tmp, 256);
}

void KeyToValue16(BYTE* buf)
{
	BYTE tmp[16];
	for (int i = 0; i < 16; i++)
	{
		tmp[buf[i]] = i; 
	}
	memmove(buf, tmp, 16);
}

void permutateJcc(WORD* buf, int elemCount, BYTE* permutation)
{
	WORD temp[16] = { 0 };
	for (int i = 0; i < elemCount; i++)
	{
		temp[i] = buf[permutation[i]];
		if (i > permutation[i])
		{
			WORD tmp = i - permutation[i];
			tmp <<= 9;
			temp[i] -= tmp;
		}
		else
		{
			WORD tmp = permutation[i] - i;
			tmp <<= 9;
			temp[i] += tmp;
		}
	}
	memmove(buf, temp, 2*16);
}

int GetRelocMap(const BYTE* relocSeg, DWORD funcRVA, int funcSize, DWORD* relocMap)
{
	// relocSeg 重定位表首地址
	// funcRVA 虚拟化地址起始地址
	// funcSize 虚拟化大小
	// relocMap 在虚拟化块需要重定位的地址
	// 返回值为需要修改的重定位地址数量
	int relCount = 0;
	while (*(DWORD*)relocSeg)
	{
		DWORD relocRVA = ((DWORD*)relocSeg)[0];
		DWORD blockSize = ((DWORD*)relocSeg)[1];
		for (int i = 0; i < (blockSize - 8) / 2; i++)
		{
			// 重定位表项：每个项大小为一个字，每个字的高四位被用来说明此重定位项的类型
			// &0xFFF 去除一个字的高四位
			// PE权威指南191页
			if ((relocRVA + (((WORD*)(relocSeg + 8))[i] & 0xFFF) >= funcRVA) &&
				(relocRVA + (((WORD*)(relocSeg + 8))[i] & 0xFFF) < funcRVA + funcSize))
			{
				if (relocMap) 
					relocMap[relCount] = relocRVA + (((WORD*)(relocSeg + 8))[i] & 0xFFF);

				relCount++;
			}
		}
		relocSeg += blockSize;	// 遍历下一个块
	}
	return relCount;
}
