#pragma once

#include <Windows.h>
#include <sstream>

namespace Hydr10n {
	namespace DataUtils {
		template <class T>
		struct AppData final {
			static bool Save(LPCWSTR lpcwSection, LPCWSTR lpcwKey, LPCWSTR lpcwFilePath, const T& data) { return WritePrivateProfileStringW(lpcwSection, lpcwKey, std::to_wstring(data).c_str(), lpcwFilePath); }

			static bool Load(LPCWSTR lpcwSection, LPCWSTR lpcwKey, LPCWSTR lpcwFilePath, T& data) {
				WCHAR buffer[1025];
				bool ret = GetPrivateProfileStringW(lpcwSection, lpcwKey, NULL, buffer, ARRAYSIZE(buffer), lpcwFilePath);
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

		template <>
		struct AppData<LPCWSTR> final {
			static bool Save(LPCWSTR lpcwSection, LPCWSTR lpcwKey, LPCWSTR lpcwFilePath, LPCWSTR data) { return WritePrivateProfileStringW(lpcwSection, lpcwKey, data, lpcwFilePath); }

			static bool Load(LPCWSTR lpcwSection, LPCWSTR lpcwKey, LPCWSTR lpcwFilePath, LPWSTR data, DWORD nSize) { return GetPrivateProfileStringW(lpcwSection, lpcwKey, NULL, data, nSize, lpcwFilePath); }
		};
	}
}