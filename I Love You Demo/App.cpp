/*
 * Project Name: I Love You Demo
 * Last Update: 2020/08/14
 * 
 * This project is hosted on https://github.com/Hydr10n/I-Love-You-Demo
 * Copyright (C) Programmer-Yang_Xun@outlook.com. All Rights Reserved.
 */

#include "pch.h"
#include "MainWindow.h"

#pragma warning(disable:28252)
int APIENTRY wWinMain(HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd) {
	bool isExceptionCaught = true;
	DWORD ret;
	try {
		MainWindow mainWindow;
		ret = mainWindow.Run();
		isExceptionCaught = false;
	}
	catch (HRESULT hr) { ret = Hydr10n::SystemErrorHelpers::WIN32_FROM_HRESULT(hr); }
	catch (DWORD dwLastError) { ret = dwLastError; }
	catch (...) { ret = GetLastError(); }
	if (isExceptionCaught && ret != ERROR_SUCCESS)
		MessageBoxW(NULL, SystemErrorMessage(ret), NULL, MB_OK | MB_ICONERROR);
	return (int)ret;
}