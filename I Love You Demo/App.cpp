/*
 * Project: I Love You Demo
 * Last Update: 2020/11/11
 *
 * This project is hosted on https://github.com/Hydr10n/I-Love-You-Demo
 * Copyright (C) Hydr10n@GitHub. All Rights Reserved.
 */

#include "pch.h"
#include "MainWindow.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
	using namespace std;
	bool isExceptionCaught = true;
	DWORD ret;
	try {
		ret = MainWindow().Run();
		isExceptionCaught = false;
	}
	catch (system_error& e) { ret = (DWORD)e.code().value(); }
	catch (exception& e) {
		MessageBoxA(NULL, e.what(), NULL, MB_OK | MB_ICONERROR);
		return ERROR_CAN_NOT_COMPLETE;
	}
	catch (...) { ret = GetLastError(); }
	if (isExceptionCaught && ret != ERROR_SUCCESS)
		MessageBoxA(NULL, system_category().message((int)ret).c_str(), NULL, MB_OK | MB_ICONERROR);
	return (int)ret;
}