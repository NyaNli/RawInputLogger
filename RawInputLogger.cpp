#include<iostream>
#include<Windows.h>
#include<tchar.h>
#include "RawInput.h"

HWND RegisterReceiverWindow(HINSTANCE hInstance, LPCTSTR title);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
int DoEvent();
const char* GetKeyName(USHORT keyCode);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ PTCHAR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	FILE* out = nullptr;
	AllocConsole();
	freopen_s(&out, "CONOUT$", "w", stdout);
	std::cout << "Console Initialized." << std::endl;

	HWND hwnd = RegisterReceiverWindow(hInstance, TEXT("RawInputLogger"));
	std::cout << "Windows Message Receiver Initialized. HWND: " << std::hex << hwnd << std::dec << std::endl;

	RAWINPUTDEVICE rids[2];
	rids[0].usUsagePage = 0x01;
	rids[0].usUsage = 0x06;
	rids[0].dwFlags = RIDEV_INPUTSINK;
	rids[0].hwndTarget = hwnd;
	rids[1].usUsagePage = 0x01;
	rids[1].usUsage = 0x02;
	rids[1].dwFlags = RIDEV_INPUTSINK;
	rids[1].hwndTarget = hwnd;

	BOOL reg = RegisterRawInputDevices(rids, 2, sizeof(RAWINPUTDEVICE));
	if (reg)
		std::cout << "Raw Input Devices Registered" << std::endl;
	else
	{
		std::cout << "Raw Input Devices register failed." << std::endl << "Error Code: " << GetLastError() << std::endl;
		return -1;
	}

	return DoEvent();
}

HWND RegisterReceiverWindow(HINSTANCE hInstance, LPCTSTR title)
{
	WNDCLASSEXW wcex;

	memset(&wcex, 0, sizeof(WNDCLASSEX));

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.lpszClassName = L"SystemMsgReceiver";

	RegisterClassExW(&wcex);

	HWND hwnd = CreateWindow(TEXT("SystemMsgReceiver"), title, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, HWND_MESSAGE, NULL, hInstance, NULL);

	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	return hwnd;
}

#define _str(s) #s
#define CheckBtn(btn) if (rawinput->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_##btn##_DOWN)\
	std::cout << _str(Button_##btn##_Down) << " ";\
else if (rawinput->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_##btn##_UP)\
	std::cout << _str(Button_##btn##_Up) << " ";

void showMsg(RAWINPUT* rawinput)
{
	if (rawinput->header.dwType == RIM_TYPEMOUSE)
	{
		std::cout << std::endl;
		std::cout << "Moust Event: ";
		if (rawinput->data.mouse.usButtonFlags & MOUSE_MOVE_ABSOLUTE)
			std::cout << "Position: (" << rawinput->data.mouse.lLastX << "," << rawinput->data.mouse.lLastY << ") ";
		else if ((rawinput->data.mouse.usButtonFlags & MOUSE_MOVE_ABSOLUTE) == 0)
			std::cout << "Motion: (" << rawinput->data.mouse.lLastX << "," << rawinput->data.mouse.lLastY << ") ";
		std::cout << "Button: ";
		CheckBtn(1);
		CheckBtn(2);
		CheckBtn(3);
		CheckBtn(4);
		CheckBtn(5);
		if (rawinput->data.mouse.usButtonFlags & RI_MOUSE_WHEEL)
			std::cout << "Wheel: " << (SHORT)rawinput->data.mouse.usButtonData << std::endl;
	}
	else if (rawinput->header.dwType == RIM_TYPEKEYBOARD && rawinput->data.keyboard.VKey != 0xFF)
	{
		std::cout << std::endl;
		std::cout << "Keyboard Event: ";
		if (rawinput->data.keyboard.Flags & RI_KEY_BREAK)
			std::cout << "Status: Key Up ";
		else if ((rawinput->data.keyboard.Flags & RI_KEY_BREAK) == 0)
			std::cout << "Status: Key Down";
		std::cout << "Key: " << GetKeyName(rawinput->data.keyboard.VKey);
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_INPUT)
	{
		PRAWINPUT* rawInputArray = GetRawInputDatas(lParam);
		int i = 0;
		for (; rawInputArray[i] != NULL; i++)
			showMsg(rawInputArray[i]);
		LRESULT result = DefRawInputProc(rawInputArray, i, sizeof(RAWINPUTHEADER));
		for (i = 0; rawInputArray[i] != NULL; i++)
		{
			PBYTE bytes = (PBYTE)rawInputArray[i];
			delete[]bytes;
		}
		delete[]rawInputArray;
		return result;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

int DoEvent()
{
	
	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
	

	/*
	MSG msg;
	while (1)
	{
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	*/
	
}

#define CaseKey(key) case key: return #key;break;

const char* GetKeyName(USHORT keyCode)
{
	static char ascii[5] = "VK_\0";
	if (keyCode >= '0' && keyCode <= '9' || keyCode >= 'A' && keyCode <= 'Z')
	{
		ascii[3] = (char)keyCode;
		return ascii;
	}
	switch (keyCode)
	{
		CaseKey(VK_LBUTTON);
		CaseKey(VK_RBUTTON);
		CaseKey(VK_CANCEL);
		CaseKey(VK_MBUTTON);
		CaseKey(VK_XBUTTON1);
		CaseKey(VK_XBUTTON2);
		CaseKey(VK_BACK);
		CaseKey(VK_TAB);
		CaseKey(VK_CLEAR);
		CaseKey(VK_RETURN);
		CaseKey(VK_SHIFT);
		CaseKey(VK_CONTROL);
		CaseKey(VK_MENU);
		CaseKey(VK_PAUSE);
		CaseKey(VK_CAPITAL);
		CaseKey(VK_KANA);
		CaseKey(VK_JUNJA);
		CaseKey(VK_FINAL);
		CaseKey(VK_KANJI);
		CaseKey(VK_ESCAPE);
		CaseKey(VK_CONVERT);
		CaseKey(VK_NONCONVERT);
		CaseKey(VK_ACCEPT);
		CaseKey(VK_MODECHANGE);
		CaseKey(VK_SPACE);
		CaseKey(VK_PRIOR);
		CaseKey(VK_NEXT);
		CaseKey(VK_END);
		CaseKey(VK_HOME);
		CaseKey(VK_LEFT);
		CaseKey(VK_UP);
		CaseKey(VK_RIGHT);
		CaseKey(VK_DOWN);
		CaseKey(VK_SELECT);
		CaseKey(VK_PRINT);
		CaseKey(VK_EXECUTE);
		CaseKey(VK_SNAPSHOT);
		CaseKey(VK_INSERT);
		CaseKey(VK_DELETE);
		CaseKey(VK_HELP);
		CaseKey(VK_LWIN);
		CaseKey(VK_RWIN);
		CaseKey(VK_APPS);
		CaseKey(VK_SLEEP);
		CaseKey(VK_NUMPAD0);
		CaseKey(VK_NUMPAD1);
		CaseKey(VK_NUMPAD2);
		CaseKey(VK_NUMPAD3);
		CaseKey(VK_NUMPAD4);
		CaseKey(VK_NUMPAD5);
		CaseKey(VK_NUMPAD6);
		CaseKey(VK_NUMPAD7);
		CaseKey(VK_NUMPAD8);
		CaseKey(VK_NUMPAD9);
		CaseKey(VK_MULTIPLY);
		CaseKey(VK_ADD);
		CaseKey(VK_SEPARATOR);
		CaseKey(VK_SUBTRACT);
		CaseKey(VK_DECIMAL);
		CaseKey(VK_DIVIDE);
		CaseKey(VK_F1);
		CaseKey(VK_F2);
		CaseKey(VK_F3);
		CaseKey(VK_F4);
		CaseKey(VK_F5);
		CaseKey(VK_F6);
		CaseKey(VK_F7);
		CaseKey(VK_F8);
		CaseKey(VK_F9);
		CaseKey(VK_F10);
		CaseKey(VK_F11);
		CaseKey(VK_F12);
		CaseKey(VK_F13);
		CaseKey(VK_F14);
		CaseKey(VK_F15);
		CaseKey(VK_F16);
		CaseKey(VK_F17);
		CaseKey(VK_F18);
		CaseKey(VK_F19);
		CaseKey(VK_F20);
		CaseKey(VK_F21);
		CaseKey(VK_F22);
		CaseKey(VK_F23);
		CaseKey(VK_F24);
		CaseKey(VK_NAVIGATION_VIEW);
		CaseKey(VK_NAVIGATION_MENU);
		CaseKey(VK_NAVIGATION_UP);
		CaseKey(VK_NAVIGATION_DOWN);
		CaseKey(VK_NAVIGATION_LEFT);
		CaseKey(VK_NAVIGATION_RIGHT);
		CaseKey(VK_NAVIGATION_ACCEPT);
		CaseKey(VK_NAVIGATION_CANCEL);
		CaseKey(VK_NUMLOCK);
		CaseKey(VK_SCROLL);
		CaseKey(VK_OEM_FJ_JISHO);
		CaseKey(VK_OEM_FJ_MASSHOU);
		CaseKey(VK_OEM_FJ_TOUROKU);
		CaseKey(VK_OEM_FJ_LOYA);
		CaseKey(VK_OEM_FJ_ROYA);
		CaseKey(VK_LSHIFT);
		CaseKey(VK_RSHIFT);
		CaseKey(VK_LCONTROL);
		CaseKey(VK_RCONTROL);
		CaseKey(VK_LMENU);
		CaseKey(VK_RMENU);
		CaseKey(VK_BROWSER_BACK);
		CaseKey(VK_BROWSER_FORWARD);
		CaseKey(VK_BROWSER_REFRESH);
		CaseKey(VK_BROWSER_STOP);
		CaseKey(VK_BROWSER_SEARCH);
		CaseKey(VK_BROWSER_FAVORITES);
		CaseKey(VK_BROWSER_HOME);
		CaseKey(VK_VOLUME_MUTE);
		CaseKey(VK_VOLUME_DOWN);
		CaseKey(VK_VOLUME_UP);
		CaseKey(VK_MEDIA_NEXT_TRACK);
		CaseKey(VK_MEDIA_PREV_TRACK);
		CaseKey(VK_MEDIA_STOP);
		CaseKey(VK_MEDIA_PLAY_PAUSE);
		CaseKey(VK_LAUNCH_MAIL);
		CaseKey(VK_LAUNCH_MEDIA_SELECT);
		CaseKey(VK_LAUNCH_APP1);
		CaseKey(VK_LAUNCH_APP2);
		CaseKey(VK_OEM_1);
		CaseKey(VK_OEM_PLUS);
		CaseKey(VK_OEM_COMMA);
		CaseKey(VK_OEM_MINUS);
		CaseKey(VK_OEM_PERIOD);
		CaseKey(VK_OEM_2);
		CaseKey(VK_OEM_3);
		CaseKey(VK_GAMEPAD_A);
		CaseKey(VK_GAMEPAD_B);
		CaseKey(VK_GAMEPAD_X);
		CaseKey(VK_GAMEPAD_Y);
		CaseKey(VK_GAMEPAD_RIGHT_SHOULDER);
		CaseKey(VK_GAMEPAD_LEFT_SHOULDER);
		CaseKey(VK_GAMEPAD_LEFT_TRIGGER);
		CaseKey(VK_GAMEPAD_RIGHT_TRIGGER);
		CaseKey(VK_GAMEPAD_DPAD_UP);
		CaseKey(VK_GAMEPAD_DPAD_DOWN);
		CaseKey(VK_GAMEPAD_DPAD_LEFT);
		CaseKey(VK_GAMEPAD_DPAD_RIGHT);
		CaseKey(VK_GAMEPAD_MENU);
		CaseKey(VK_GAMEPAD_VIEW);
		CaseKey(VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON);
		CaseKey(VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON);
		CaseKey(VK_GAMEPAD_LEFT_THUMBSTICK_UP);
		CaseKey(VK_GAMEPAD_LEFT_THUMBSTICK_DOWN);
		CaseKey(VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT);
		CaseKey(VK_GAMEPAD_LEFT_THUMBSTICK_LEFT);
		CaseKey(VK_GAMEPAD_RIGHT_THUMBSTICK_UP);
		CaseKey(VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN);
		CaseKey(VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT);
		CaseKey(VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT);
		CaseKey(VK_OEM_4);
		CaseKey(VK_OEM_5);
		CaseKey(VK_OEM_6);
		CaseKey(VK_OEM_7);
		CaseKey(VK_OEM_8);
		CaseKey(VK_OEM_AX);
		CaseKey(VK_OEM_102);
		CaseKey(VK_ICO_HELP);
		CaseKey(VK_ICO_00);
		CaseKey(VK_PROCESSKEY);
		CaseKey(VK_ICO_CLEAR);
		CaseKey(VK_PACKET);
		CaseKey(VK_OEM_RESET);
		CaseKey(VK_OEM_JUMP);
		CaseKey(VK_OEM_PA1);
		CaseKey(VK_OEM_PA2);
		CaseKey(VK_OEM_PA3);
		CaseKey(VK_OEM_WSCTRL);
		CaseKey(VK_OEM_CUSEL);
		CaseKey(VK_OEM_ATTN);
		CaseKey(VK_OEM_FINISH);
		CaseKey(VK_OEM_COPY);
		CaseKey(VK_OEM_AUTO);
		CaseKey(VK_OEM_ENLW);
		CaseKey(VK_OEM_BACKTAB);
		CaseKey(VK_ATTN);
		CaseKey(VK_CRSEL);
		CaseKey(VK_EXSEL);
		CaseKey(VK_EREOF);
		CaseKey(VK_PLAY);
		CaseKey(VK_ZOOM);
		CaseKey(VK_NONAME);
		CaseKey(VK_PA1);
		CaseKey(VK_OEM_CLEAR);
	}
	return "VK_UNKNOWN";
}
