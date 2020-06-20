#include<Windows.h>

PRAWINPUT* GetRawInputDatas(LPARAM lParam)
{
#ifndef _WIN64
	// Fix Wow64
	BOOL isWow64 = false;
	IsWow64Process(GetCurrentProcess(), &isWow64);
#endif
	PRAWINPUT buffedData = NULL;
	UINT buffLength = 0;
	UINT dwSize = 0;
	UINT result = GetRawInputBuffer(NULL, &dwSize, sizeof(RAWINPUTHEADER));
	DWORD err = GetLastError();
	if ((INT)result == 0 && dwSize != 0)
	{
/*
#ifndef _WIN64
		if (isWow64)
		{
			UINT n = dwSize / (sizeof(RAWINPUTHEADER) + sizeof(RAWHID));
			dwSize += n * 8;
		}
#endif
*/
		dwSize *= 2;
		PBYTE bytes = new BYTE[dwSize];
		buffLength = GetRawInputBuffer((PRAWINPUT)bytes, &dwSize, sizeof(RAWINPUTHEADER));
		if ((int)buffLength == -1)
		{
			MessageBoxA(NULL, "ERROR", "ERROR", 0);
			buffLength = 0;
			delete[]bytes;
		}
		else
		{
			buffedData = (PRAWINPUT)bytes;
		}
	}

	PRAWINPUT* pointerArray = new PRAWINPUT[buffLength + 2];
	if (buffLength > 0)
	{
#ifndef _WIN64
		if (isWow64)
		{
			PRAWINPUT pointer = buffedData;
			for (unsigned int i = 0; i < buffLength; i++)
			{
				UINT realSize = pointer->header.dwSize - 8;
				PBYTE pbyte = (PBYTE)pointer;
				PRAWINPUT rawinput = (PRAWINPUT)new BYTE[realSize];
				rawinput->header.dwType = pointer->header.dwType;
				rawinput->header.dwSize = realSize;
				rawinput->header.hDevice = *((PHANDLE)(&pbyte[sizeof(DWORD) + sizeof(DWORD)]));
				rawinput->header.wParam = *((WPARAM*)(&pbyte[sizeof(DWORD) + sizeof(DWORD) + sizeof(HANDLE) + 4]));
				memcpy_s(&rawinput->data, realSize - sizeof(RAWINPUTHEADER), &pbyte[sizeof(RAWINPUTHEADER) + 8], realSize - sizeof(RAWINPUTHEADER));
				pointerArray[i] = rawinput;
				pointer = NEXTRAWINPUTBLOCK(pointer);
			}
			
		}
		else
		{
#else
#define QWORD UINT64
#endif
			PRAWINPUT pointer = buffedData;
			for (unsigned int i = 0; i < buffLength; i++)
			{
				PRAWINPUT rawinput = (PRAWINPUT)new BYTE[pointer->header.dwSize];
				memcpy_s(rawinput, pointer->header.dwSize, pointer, pointer->header.dwSize);
				pointerArray[i] = rawinput;
				pointer = NEXTRAWINPUTBLOCK(pointer);
			}
#ifndef _WIN64
		}
#endif
		delete[]buffedData;
		buffedData = NULL;
	}

	pointerArray[buffLength] = NULL;
	pointerArray[buffLength+1] = NULL;

	dwSize = 0;
	result = GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
	if (result == 0)
	{
		PBYTE bytes = new BYTE[dwSize];
		result = GetRawInputData((HRAWINPUT)lParam, RID_INPUT, bytes, &dwSize, sizeof(RAWINPUTHEADER));
		if (result != dwSize)
			return pointerArray;
		pointerArray[buffLength] = (PRAWINPUT)bytes;
	}

	return pointerArray;
}