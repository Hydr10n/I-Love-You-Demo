#pragma once

#include "AppData.h"
#include "WindowUtils.h"
#include <utility>

class MyAppSettingsData final {
public:
	enum class Key_bool { ShowFramesPerSecond, ShowHelpAtStartup };

	static LPCWSTR GetFilePath() { return m_FilePath.c_str(); }

	static bool Save(Key_bool key, bool data) {
		const auto& sectionKeyPair = ToSectionKeyPair(key);
		return Hydr10n::DataUtils::AppData<LPCWSTR>::Save(ToString(sectionKeyPair.first), sectionKeyPair.second, GetFilePath(), ToString(data));
	}

	static bool Load(Key_bool key, bool& data) {
		WCHAR buffer[7];
		const auto& sectionKeyPair = ToSectionKeyPair(key);
		bool ret = Hydr10n::DataUtils::AppData<LPCWSTR>::Load(ToString(sectionKeyPair.first), sectionKeyPair.second, GetFilePath(), buffer, ARRAYSIZE(buffer));
		if (ret) {
			RemoveSubstringStartingWithSpace(buffer);
			ret = ToValue(buffer, data);
		}
		else
			data = {};
		return ret;
	}

	static bool Save(Hydr10n::WindowUtils::WindowMode data) { return Hydr10n::DataUtils::AppData<LPCWSTR>::Save(ToString(Section::DisplaySettings), KeyWindowMode, GetFilePath(), ToString(data)); }

	static bool Load(Hydr10n::WindowUtils::WindowMode& data) {
		WCHAR buffer[12];
		bool ret = Hydr10n::DataUtils::AppData<LPCWSTR>::Load(ToString(Section::DisplaySettings), KeyWindowMode, GetFilePath(), buffer, ARRAYSIZE(buffer));
		if (ret) {
			RemoveSubstringStartingWithSpace(buffer);
			ret = ToValue(buffer, data);
		}
		else
			data = {};
		return ret;
	}

	static bool Save(const Hydr10n::DisplayUtils::DisplayResolution& data) {
		using Hydr10n::DataUtils::AppData;
		const LPCWSTR lpcwSection = ToString(Section::DisplaySettings), lpcwFilePath = GetFilePath();
		return AppData<DWORD>::Save(lpcwSection, KeyResolutionWidth, lpcwFilePath, data.PixelWidth) && AppData<DWORD>::Save(lpcwSection, KeyResolutionHeight, lpcwFilePath, data.PixelHeight);
	}

	static bool Load(Hydr10n::DisplayUtils::DisplayResolution& data) {
		using Hydr10n::DataUtils::AppData;
		const LPCWSTR lpcwSection = ToString(Section::DisplaySettings), lpcwFilePath = GetFilePath();
		const bool ret = AppData<DWORD>::Load(lpcwSection, KeyResolutionWidth, lpcwFilePath, data.PixelWidth) && AppData<DWORD>::Load(lpcwSection, KeyResolutionHeight, lpcwFilePath, data.PixelHeight);
		if (!ret)
			data = {};
		return ret;
	}

private:
	enum class Section { DisplaySettings, MiscellaneousSettings };

	using SectionKeyPair = std::pair<Section, LPCWSTR>;

	static constexpr LPCWSTR KeyWindowMode = L"WindowMode",
		KeyResolutionWidth = L"ResolutionWidth", KeyResolutionHeight = L"ResolutionHeight";

	static std::wstring m_FilePath;

	static bool RemoveSubstringStartingWithSpace(LPWSTR str) {
		const std::wstring wstr(str);
		const auto iteratorBegin = wstr.cbegin(), iteratorEnd = wstr.cend(), iterator = std::find_if(iteratorBegin, iteratorEnd, [](wchar_t ch) { return iswspace((wint_t)ch); });
		if (iterator == iteratorEnd)
			return false;
		str[(iterator - iteratorBegin)] = 0;
		return true;
	}

	static SectionKeyPair ToSectionKeyPair(Key_bool val) {
		switch (val) {
		case Key_bool::ShowFramesPerSecond: return SectionKeyPair(Section::DisplaySettings, L"ShowFramesPerSecond");
		case Key_bool::ShowHelpAtStartup: return SectionKeyPair(Section::MiscellaneousSettings, L"ShowHelpAtStartup");
		default: throw;
		}
	}

	static LPCWSTR ToString(Section val) {
		switch (val) {
		case Section::DisplaySettings: return L"Display Settings";
		case Section::MiscellaneousSettings: return L"Miscellaneous Settings";
		default: throw;
		}
	}

	static LPCWSTR ToString(bool val) { return val ? L"true" : L"false"; }

	static bool ToValue(LPCWSTR str, bool& val) {
		const std::wstring wstr(str);
		if (wstr == ToString(true))
			val = true;
		else {
			val = {};
			if (wstr != ToString(val))
				return false;
		}
		return true;
	}

	static LPCWSTR ToString(Hydr10n::WindowUtils::WindowMode val) {
		const LPCWSTR strs[] = { L"Windowed", L"Borderless", L"FullScreen" };
		return strs[(size_t)val];
	}

	static bool ToValue(LPCWSTR str, Hydr10n::WindowUtils::WindowMode& val) {
		using Hydr10n::WindowUtils::WindowMode;
		const std::wstring wstr(str);
		if (wstr == ToString(WindowMode::Borderless))
			val = WindowMode::Borderless;
		else if (wstr == ToString(WindowMode::FullScreen))
			val = WindowMode::FullScreen;
		else {
			val = {};
			if (wstr != ToString(val))
				return false;
		}
		return true;
	}

	static const struct static_constructor {
		static_constructor() noexcept(false) {
			WCHAR szFileName[MAX_PATH];
			if (GetModuleFileNameW(NULL, szFileName, ARRAYSIZE(szFileName))) {
				m_FilePath = szFileName;
				m_FilePath.replace(m_FilePath.find_last_of(L'\\') + 1, m_FilePath.length(), L"Settings.ini");
			}
		}
	} m_static_constructor;
};

decltype(MyAppSettingsData::m_FilePath) MyAppSettingsData::m_FilePath;
decltype(MyAppSettingsData::m_static_constructor) MyAppSettingsData::m_static_constructor;