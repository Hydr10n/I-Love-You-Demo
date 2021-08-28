/*
 * Header File: DisplayHelpers.h
 * Last Update: 2021/08/28
 *
 * Copyright (C) Hydr10n@GitHub. All Rights Reserved.
 */

#pragma once

#include <vector>
#include "Resolution.h"

namespace Hydr10n {
	namespace DisplayHelpers {
		std::vector<Resolution> WINAPI GetDisplayResolutions(LPCWSTR lpszDeviceName = nullptr) {
			std::vector<Resolution> resolutions;
			DEVMODEW devMode;
			devMode.dmSize = sizeof(devMode);
			for (DWORD i = 0; EnumDisplaySettingsW(lpszDeviceName, i++, &devMode);) {
				const auto& iteratorEnd = resolutions.cend();
				const Resolution resolution{ static_cast<LONG>(devMode.dmPelsWidth), static_cast<LONG>(devMode.dmPelsHeight) };
				if (std::find(resolutions.cbegin(), iteratorEnd, resolution) == iteratorEnd)
					resolutions.push_back(resolution);
			}
			return resolutions;
		}
	}
}