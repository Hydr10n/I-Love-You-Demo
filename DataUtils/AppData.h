/*
 * Header File: AppData.h
 * Last Update: 2020/10/28
 *
 * Copyright (C) Hydr10n@GitHub. All Rights Reserved.
 */

#pragma once

#include <Windows.h>
#include <sstream>

namespace Hydr10n {
	namespace DataUtils {
		struct AppData final {
			static bool Save(LPCWSTR lpcwSection, LPCWSTR lpcwKey, LPCWSTR lpcwPath, LPCWSTR data) { return WritePrivateProfileStringW(lpcwSection, lpcwKey, data, lpcwPath); }

			template <class T>
			static bool Save(LPCWSTR lpcwSection, LPCWSTR lpcwKey, LPCWSTR lpcwPath, const T& data) { return Save(lpcwSection, lpcwKey, lpcwPath, std::to_wstring(data).c_str()); }

			static bool Load(LPCWSTR lpcwSection, LPCWSTR lpcwKey, LPCWSTR lpcwPath, LPWSTR data, DWORD nSize) { return GetPrivateProfileStringW(lpcwSection, lpcwKey, NULL, data, nSize, lpcwPath) || !GetLastError(); }

			template <class T>
			static bool Load(LPCWSTR lpcwSection, LPCWSTR lpcwKey, LPCWSTR lpcwPath, T& data) {
				WCHAR buffer[1025];
				bool ret = Load(lpcwSection, lpcwKey, lpcwPath, buffer, ARRAYSIZE(buffer));
				if (ret) {
					WCHAR ch;
					std::wistringstream istringstream(buffer);
					ret = (istringstream >> data) && !(istringstream >> ch);
				}
				if (!ret)
					data = {};
				return ret;
			}
		};
	}
}