#include "pch.h"

#include "MainWindow.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
	int ret;
	try { ret = static_cast<int>(MainWindow().Run()); }
	catch (const std::system_error& e) {
		ret = e.code().value();
		MessageBoxA(nullptr, e.what(), nullptr, MB_OK | MB_ICONERROR);
	}
	catch (const std::exception& e) {
		ret = ERROR_CAN_NOT_COMPLETE;
		MessageBoxA(nullptr, e.what(), nullptr, MB_OK | MB_ICONERROR);
	}
	catch (...) {
		if ((ret = static_cast<int>(GetLastError())) != ERROR_SUCCESS)
			MessageBoxA(nullptr, std::system_category().message(ret).c_str(), nullptr, MB_OK | MB_ICONERROR);
	}
	return ret;
}