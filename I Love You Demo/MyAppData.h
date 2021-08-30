#pragma once

#include "AppData.h"
#include "WindowHelpers.h"
#include <filesystem>

struct MyAppData {
	static class SettingsData {
	public:
		enum class Key_bool { ShowFPS, ShowHelpAtStartup };

		static bool Save(Key_bool key, bool data) {
			const auto sectionKeyPair = ToSectionKeyPair(key);
			return m_AppData.Save(ToString(sectionKeyPair.first), sectionKeyPair.second, ToString(data));
		}

		static bool Load(Key_bool key, bool& data) {
			const auto sectionKeyPair = ToSectionKeyPair(key);
			WCHAR buf[7];
			auto ret = m_AppData.Load(ToString(sectionKeyPair.first), sectionKeyPair.second, buf, ARRAYSIZE(buf));
			if (ret) {
				RemoveSubstringStartingWithSpace(buf);
				ret = ToValue(buf, data);
			}
			return ret;
		}

		static bool Save(Hydr10n::WindowHelpers::WindowMode data) { return m_AppData.Save(ToString(Section::DisplaySettings), KeyWindowMode, ToString(data)); }

		static bool Load(Hydr10n::WindowHelpers::WindowMode& data) {
			WCHAR buf[12];
			auto ret = m_AppData.Load(ToString(Section::DisplaySettings), KeyWindowMode, buf, ARRAYSIZE(buf));
			if (ret) {
				RemoveSubstringStartingWithSpace(buf);
				ret = ToValue(buf, data);
			}
			return ret;
		}

		static bool Save(const SIZE& data) {
			const auto section = ToString(Section::DisplaySettings);
			return m_AppData.Save(section, KeyResolutionWidth, data.cx) && m_AppData.Save(section, KeyResolutionHeight, data.cy);
		}

		static bool Load(SIZE& data) {
			const auto section = ToString(Section::DisplaySettings);
			return m_AppData.Load(section, KeyResolutionWidth, data.cx) && m_AppData.Load(section, KeyResolutionHeight, data.cy);
		}

	private:
		enum class Section { DisplaySettings, MiscellaneousSettings };

		using SectionKeyPair = std::pair<Section, LPCWSTR>;

		static constexpr LPCWSTR KeyWindowMode = L"WindowMode",
			KeyResolutionWidth = L"ResolutionWidth", KeyResolutionHeight = L"ResolutionHeight";

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
			case Key_bool::ShowFPS: return SectionKeyPair(Section::DisplaySettings, L"ShowFPS");
			case Key_bool::ShowHelpAtStartup: return SectionKeyPair(Section::MiscellaneousSettings, L"ShowHelpAtStartup");
			default: throw;
			}
		}

		static constexpr LPCWSTR ToString(Section val) {
			switch (val) {
			case Section::DisplaySettings: return L"Display";
			case Section::MiscellaneousSettings: return L"Miscellaneous";
			default: throw;
			}
		}

		static constexpr LPCWSTR ToString(bool val) { return val ? L"true" : L"false"; }

		static constexpr bool ToValue(LPCWSTR str, bool& val) {
			const std::wstring wstr(str);
			if (wstr == ToString(true))
				val = true;
			else if (wstr == ToString(false))
				val = false;
			else
				return false;
			return true;
		}

		static constexpr LPCWSTR ToString(Hydr10n::WindowHelpers::WindowMode val) {
			constexpr LPCWSTR strs[]{ L"Windowed", L"Borderless", L"Fullscreen" };
			return strs[static_cast<size_t>(val)];
		}

		static constexpr bool ToValue(LPCWSTR str, Hydr10n::WindowHelpers::WindowMode& val) {
			using Hydr10n::WindowHelpers::WindowMode;
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
	} Settings;
};

decltype(MyAppData::SettingsData::m_AppData) MyAppData::SettingsData::m_AppData(std::filesystem::path(*__wargv).replace_filename("Settings.ini").c_str());