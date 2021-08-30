/*
 * Header File: AppData.h
 * Last Update: 2021/08/30
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

			static BOOL Save(LPCWSTR lpPath, LPCWSTR lpSection, LPCWSTR lpKey, LPCWSTR data) { return WritePrivateProfileStringW(lpSection, lpKey, data, lpPath); }

			BOOL Save(LPCWSTR lpSection, LPCWSTR lpKey, LPCWSTR data) const { return Save(m_path.c_str(), lpSection, lpKey, data); }

			template <class T>
			static BOOL Save(LPCWSTR lpPath, LPCWSTR lpSection, LPCWSTR lpKey, const T& data) { return Save(lpPath, lpSection, lpKey, std::to_wstring(data).c_str()); }

			template <class T>
			BOOL Save(LPCWSTR lpSection, LPCWSTR lpKey, const T& data) const { return Save(m_path.c_str(), lpSection, lpKey, data); }

			static BOOL Load(LPCWSTR lpPath, LPCWSTR lpSection, LPCWSTR lpKey, LPWSTR data, DWORD nSize) { return GetPrivateProfileStringW(lpSection, lpKey, nullptr, data, nSize, lpPath), GetLastError() == ERROR_SUCCESS; }

			BOOL Load(LPCWSTR lpSection, LPCWSTR lpKey, LPWSTR data, DWORD nSize) const { return Load(m_path.c_str(), lpSection, lpKey, data, nSize); }

			template <class T>
			static BOOL Load(LPCWSTR lpPath, LPCWSTR lpSection, LPCWSTR lpKey, T& data) {
				WCHAR buf[1025];
				BOOL ret = Load(lpPath, lpSection, lpKey, buf, ARRAYSIZE(buf));
				if (ret) {
					WCHAR ch;
					std::wistringstream istringstream(buf);
					ret = (istringstream >> data) && !(istringstream >> ch);
				}
				return ret;
			}

			template <class T>
			BOOL Load(LPCWSTR lpSection, LPCWSTR lpKey, T& data) const { return Load(m_path.c_str(), lpSection, lpKey, data); }

		private:
			std::wstring m_path;
		};
	}
}