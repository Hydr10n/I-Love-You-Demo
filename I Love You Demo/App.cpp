/*
 * Project Name: I Love You Demo
 * Last Update: 2020/10/02
 * 
 * This project is hosted on https://github.com/Hydr10n/I-Love-You-Demo
 * Copyright (C) Programmer-Yang_Xun@outlook.com. All Rights Reserved.
 */

#include "MainWindow.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
	bool isExceptionCaught = true;
	DWORD ret;
	try {
		ret = MainWindow().Run();
		isExceptionCaught = false;
	}
	catch (const std::system_error& e) { ret = (DWORD)e.code().value(); }
	catch (const std::exception& e) {
		MessageBoxA(NULL, e.what(), NULL, MB_OK | MB_ICONERROR);
		return ERROR_CAN_NOT_COMPLETE;
	}
	catch (...) { ret = GetLastError(); }
	if (isExceptionCaught && ret != ERROR_SUCCESS)
		MessageBoxA(NULL, std::system_category().message((int)ret).c_str(), NULL, MB_OK | MB_ICONERROR);
	return (int)ret;
}