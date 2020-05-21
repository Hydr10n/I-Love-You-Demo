/*
Project Name: I Love You Demo
Last Update: 2020/05/21

This project is hosted on https://github.com/Hydr10n/I-Love-You-Demo
Copyright (C) Programmer-Yang_Xun@outlook.com. All Rights Reserved.
*/

#include "pch.h"
#include "MainWindow.h"

int APIENTRY wWinMain(HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd) {
	if (SUCCEEDED(CoInitialize(NULL))) {
		SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
		SetProcessDPIAware();
		MainWindow window;
		window.Run();
		CoUninitialize();
	}
	return 0;
}