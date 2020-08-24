#pragma once

#include <Windows.h>
#include <vector>
#include <algorithm>

namespace Hydr10n {
	namespace DisplayUtils {
		struct DisplayResolution {
			DWORD PixelWidth{}, PixelHeight{};

			bool operator==(const DisplayResolution& rhs) const { return PixelWidth == rhs.PixelWidth && PixelHeight == rhs.PixelHeight; }

			bool operator!=(const DisplayResolution& rhs) const { return !operator==(rhs); }

			bool operator<(const DisplayResolution& rhs) const {
				if (PixelWidth < rhs.PixelWidth)
					return true;
				if (PixelWidth > rhs.PixelWidth)
					return false;
				return PixelHeight < rhs.PixelHeight;
			}

			bool operator>=(const DisplayResolution& rhs) const { return !operator<(rhs); }

			bool operator>(const DisplayResolution& rhs) const { return rhs < *this; }

			bool operator<=(const DisplayResolution& rhs) const { return !operator>(rhs); }
		};

		class SystemDisplayResolutionSet final {
		public:
			SystemDisplayResolutionSet() {
				DEVMODEW devMode{};
				devMode.dmSize = sizeof(devMode);
				for (DWORD i = 0; EnumDisplaySettingsW(NULL, i++, &devMode);) {
					const DisplayResolution displayResolution = { devMode.dmPelsWidth, devMode.dmPelsHeight };
					if (!Contains(displayResolution)) {
						m_SystemDisplayResolutions.push_back(displayResolution);
						if (displayResolution < m_MinDisplayResolution || m_MinDisplayResolution == DisplayResolution{ 0, 0 })
							m_MinDisplayResolution = displayResolution;
						if (displayResolution > m_MaxDisplayResolution)
							m_MaxDisplayResolution = displayResolution;
					}
				}
			}

			size_t Count() const { return m_SystemDisplayResolutions.size(); }

			int IndexOf(const DisplayResolution& displayResolution) const {
				const auto iteratorBegin = m_SystemDisplayResolutions.cbegin(), iteratorEnd = m_SystemDisplayResolutions.cend(), iterator = std::find(iteratorBegin, iteratorEnd, displayResolution);
				if (iterator == iteratorEnd)
					return -1;
				return (int)(iterator - iteratorBegin);
			}

			bool Contains(const DisplayResolution& displayResolution) const { return IndexOf(displayResolution) != -1; }

			bool IsEmpty() const { return m_SystemDisplayResolutions.empty(); }

			const DisplayResolution& GetMinDisplayResolution() const { return m_MinDisplayResolution; }

			const DisplayResolution& GetMaxDisplayResolution() const { return m_MaxDisplayResolution; }

			const DisplayResolution& operator[](size_t i) const { return m_SystemDisplayResolutions[i]; }

		private:
			DisplayResolution m_MinDisplayResolution, m_MaxDisplayResolution;
			std::vector<DisplayResolution> m_SystemDisplayResolutions;
		};
	}
}