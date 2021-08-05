/*
 * Header File: Resolution.h
 * Last Update: 2021/08/01
 *
 * Copyright (C) Hydr10n@GitHub. All Rights Reserved.
 */

#pragma once

#include <Windows.h>

namespace Hydr10n {
	namespace DisplayUtils {
		struct Resolution {
			UINT Width{}, Height{};

			bool operator<(const Resolution& rhs) const {
				if (Width < rhs.Width)
					return true;
				if (Width > rhs.Width)
					return false;
				return Height < rhs.Height;
			}

			bool operator>=(const Resolution& rhs) const { return !operator<(rhs); }

			bool operator>(const Resolution& rhs) const { return rhs < *this; }

			bool operator<=(const Resolution& rhs) const { return !operator>(rhs); }

			bool operator==(const Resolution& rhs) const { return Width == rhs.Width && Height == rhs.Height; }

			bool operator!=(const Resolution& rhs) const { return !operator==(rhs); }
		};
	}
}