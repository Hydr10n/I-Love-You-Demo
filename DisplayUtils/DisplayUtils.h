/*
 * Header File: DisplayUtils.h
 * Last Update: 2021/08/01
 *
 * Copyright (C) Hydr10n@GitHub. All Rights Reserved.
 */

#pragma once

#include <vector>
#include "Resolution.h"

namespace Hydr10n {
	namespace DisplayUtils {
		std::vector<Resolution> GetDisplayResolutions(LPCWSTR lpszDeviceName = nullptr) {
			std::vector<Resolution> resolutions;
			Resolution minResolution, maxResolution;
			DEVMODEW devMode;
			devMode.dmSize = sizeof(devMode);
			for (DWORD i = 0; EnumDisplaySettingsW(lpszDeviceName, i++, &devMode);) {
				const auto& iteratorEnd = resolutions.cend();
				const Resolution resolution = { static_cast<UINT>(devMode.dmPelsWidth), static_cast<UINT>(devMode.dmPelsHeight) };
				if (std::find(resolutions.cbegin(), iteratorEnd, resolution) == iteratorEnd) {
					resolutions.push_back(resolution);
					if (resolution < minResolution || minResolution == Resolution())
						minResolution = resolution;
					if (resolution > maxResolution)
						maxResolution = resolution;
				}
			}
			return resolutions;
		}
	}
}