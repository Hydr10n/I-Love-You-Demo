#pragma once

#include <Windows.h>
#include <system_error>

namespace Hydr10n {
	namespace SystemErrorHelpers {
		inline void WINAPI throw_system_error(int val) { throw std::system_error(std::error_code(val, std::system_category())); }

		inline void WINAPI ThrowIfFailed(BOOL b) {
			if (!b)
				throw_system_error((int)GetLastError());
		}

		inline void WINAPI ThrowIfFailed(HRESULT hr) {
			if (FAILED(hr))
				throw_system_error((int)hr);
		}
	}
}