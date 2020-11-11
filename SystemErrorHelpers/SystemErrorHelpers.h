/*
 * Header File: SystemErrorHelpers.h
 * Last Update: 2020/10/17
 *
 * Copyright (C) Hydr10n@GitHub. All Rights Reserved.
 */

#pragma once

#include <Windows.h>
#include <system_error>

namespace Hydr10n {
	namespace SystemErrorHelpers {
		inline void WINAPI throw_system_error(int val) { throw std::system_error(std::error_code(val, std::system_category())); }

		inline void WINAPI ThrowIfFailed(BOOL val) {
			if (!val)
				throw_system_error((int)GetLastError());
		}

		inline void WINAPI ThrowIfFailed(HRESULT val) {
			if (FAILED(val))
				throw_system_error((int)val);
		}
	}
}