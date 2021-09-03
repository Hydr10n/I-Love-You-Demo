#pragma once

#include "AppData.h"

#include "WindowHelpers.h"

#include <filesystem>

struct MyAppData {
	class Settings {
	public:
		enum class Key_bool { ShowFPS, ShowHelpAtStartup };

		static BOOL Save(Key_bool key, bool data) {
			const auto sectionKeyPair = ToSectionKeyPair(key);
			return m_AppData.Save(sectionKeyPair.first, sectionKeyPair.second, std::to_wstring(data).c_str());
		}

		static BOOL Load(Key_bool key, bool& data) {
			const auto sectionKeyPair = ToSectionKeyPair(key);
			return m_AppData.Load(sectionKeyPair.first, sectionKeyPair.second, data);
		}

		static BOOL Save(WindowHelpers::WindowMode data) { return m_AppData.Save(Sections::Display, Keys::WindowMode, ToString(data)); }

		static BOOL Load(WindowHelpers::WindowMode& data) {
			WCHAR buf[12];
			auto ret = m_AppData.Load(Sections::Display, Keys::WindowMode, buf, ARRAYSIZE(buf));
			if (ret) {
				RemoveSubstringStartingWithSpace(buf);

				ret = ToValue(buf, data);
			}

			return ret;
		}

		static BOOL Save(const SIZE& data) { return m_AppData.Save(Sections::Display, Keys::ResolutionWidth, data.cx) && m_AppData.Save(Sections::Display, Keys::ResolutionHeight, data.cy); }

		static BOOL Load(SIZE& data) { return m_AppData.Load(Sections::Display, Keys::ResolutionWidth, data.cx) && m_AppData.Load(Sections::Display, Keys::ResolutionHeight, data.cy); }

	private:
		using SectionKeyPair = std::pair<LPCWSTR, LPCWSTR>;

		struct Sections { static constexpr LPCWSTR Display = L"Display", Miscellaneous = L"Miscellaneous"; };

		struct Keys { static constexpr LPCWSTR WindowMode = L"WindowMode", ResolutionWidth = L"ResolutionWidth", ResolutionHeight = L"ResolutionHeight"; };

		static const Hydr10n::Data::AppData m_AppData;

		static bool RemoveSubstringStartingWithSpace(wchar_t* str) {
			const std::wstring wstr(str);
			const auto iteratorBegin = wstr.cbegin(), iteratorEnd = wstr.cend(), iterator = std::find_if(iteratorBegin, iteratorEnd, [](wchar_t ch) { return iswspace((wint_t)ch); });
			if (iterator == iteratorEnd)
				return false;

			str[(iterator - iteratorBegin)] = 0;

			return true;
		}

		static constexpr SectionKeyPair ToSectionKeyPair(Key_bool val) {
			switch (val) {
			case Key_bool::ShowFPS: return SectionKeyPair(Sections::Display, L"ShowFPS");
			case Key_bool::ShowHelpAtStartup: return SectionKeyPair(Sections::Miscellaneous, L"ShowHelpAtStartup");
			default: throw;
			}
		}

		static constexpr LPCWSTR ToString(WindowHelpers::WindowMode val) { return std::initializer_list<LPCWSTR>({ L"Windowed", L"Borderless", L"Fullscreen" }).begin()[static_cast<size_t>(val)]; }

		static constexpr bool ToValue(LPCWSTR str, WindowHelpers::WindowMode& val) {
			using WindowHelpers::WindowMode;

			const std::wstring wstr(str);
			if (wstr == ToString(WindowMode::Windowed))
				val = WindowMode::Windowed;
			else if (wstr == ToString(WindowMode::Borderless))
				val = WindowMode::Borderless;
			else if (wstr == ToString(WindowMode::Fullscreen))
				val = WindowMode::Fullscreen;
			else
				return false;

			return true;
		}
	};
};