#pragma once

#include <Windows.h>

namespace DisplayHelpers {
	struct Resolution : SIZE {
		friend bool operator<(const SIZE& lhs, const SIZE& rhs) {
			if (lhs.cx < rhs.cx)
				return true;
			if (lhs.cx > rhs.cx)
				return false;
			return lhs.cy < rhs.cy;
		}

		friend bool operator>=(const SIZE& lhs, const SIZE& rhs) { return !(lhs < Resolution{ rhs.cx, rhs.cy }); }

		friend bool operator>(const SIZE& lhs, const SIZE& rhs) { return Resolution{ rhs.cx, rhs.cy } < lhs; }

		friend bool operator<=(const SIZE& lhs, const SIZE& rhs) { return !(lhs > Resolution{ rhs.cx, rhs.cy }); }

		friend bool operator==(const SIZE& lhs, const SIZE& rhs) { return lhs.cx == rhs.cx && lhs.cy == rhs.cy; }

		friend bool operator!=(const SIZE& lhs, const SIZE& rhs) { return !(Resolution{ lhs.cx, lhs.cy } == rhs); }
	};
}