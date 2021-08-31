/*
 * Header File: AppData.h
 * Last Update: 2021/08/31
 *
 * Copyright (C) Hydr10n@GitHub. All Rights Reserved.
 */

#pragma once

#include <Windows.h>
#include <sstream>

namespace Hydr10n {
	namespace Data {
		class AppData {
		public:
			AppData(LPCWSTR lpPath) : m_path(lpPath) {}

			LPCWSTR GetPath() const { return m_path.c_str(); }

			BOOL Save(LPCWSTR lpSection, LPCWSTR lpKey, LPCWSTR data) const { return WritePrivateProfileStringW(lpSection, lpKey, data, m_path.c_str()); }

			template <class T>
			BOOL Save(LPCWSTR lpSection, LPCWSTR lpKey, const T& data) const { return Save(lpSection, lpKey, std::to_wstring(data).c_str()); }

			BOOL Load(LPCWSTR lpSection, LPCWSTR lpKey, LPWSTR data, DWORD nSize) const { return GetPrivateProfileStringW(lpSection, lpKey, nullptr, data, nSize, m_path.c_str()), GetLastError() == ERROR_SUCCESS; }

			template <class T>
			BOOL Load(LPCWSTR lpSection, LPCWSTR lpKey, T& data) const {
				WCHAR buf[1025];
				BOOL ret = Load(lpSection, lpKey, buf, ARRAYSIZE(buf));
				if (ret) {
					WCHAR ch;
					std::wistringstream istringstream(buf);
					ret = (istringstream >> data) && !(istringstream >> ch);
				}
				return ret;
			}

		private:
			const std::wstring m_path;
		};
	}
}
