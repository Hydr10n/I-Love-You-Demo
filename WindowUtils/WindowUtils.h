/*
 * Header File: WindowUtils.h
 * Last Update: 2021/07/20
 *
 * Copyright (C) Hydr10n@GitHub. All Rights Reserved.
 */

#pragma once

#include <Windows.h>
#include "../DisplayUtils/Resolution.h"

namespace Hydr10n {
	namespace WindowUtils {
		inline BOOL WINAPI CenterMainWindow(RECT& rc) {
			const LONG cxScreen = GetSystemMetrics(SM_CXSCREEN), cyScreen = GetSystemMetrics(SM_CYSCREEN);
			const BOOL ret = cxScreen && cyScreen;
			if (ret) {
				const LONG width = rc.right - rc.left, height = rc.bottom - rc.top, x = (cxScreen - width) / 2, y = (cyScreen - height) / 2;
				rc = { x, y, x + width, y + height };
			}
			return ret;
		}
	}
}