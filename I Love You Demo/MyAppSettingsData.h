#pragma once

#include "AppData.h"
#include "WindowHelpers.h"
#include <filesystem>

class MyAppSettingsData {
public:
	enum class Key_bool { ShowFPS, ShowHelpAtStartup };

	static LPCWSTR GetPath() { return m_Path.c_str(); }

	static bool Save(Key_bool key, bool data) {
		const auto& sectionKeyPair = ToSectionKeyPair(key);
		return Hydr10n::Data::AppData::Save(ToString(sectionKeyPair.first), sectionKeyPair.second, GetPath(), ToString(data));
	}

	static bool Load(Key_bool key, bool& data) {
		WCHAR buffer[7];
		const auto& sectionKeyPair = ToSectionKeyPair(key);
		bool ret = Hydr10n::Data::AppData::Load(ToString(sectionKeyPair.first), sectionKeyPair.second, GetPath(), buffer, ARRAYSIZE(buffer));
		if (ret) {
			RemoveSubstringStartingWithSpace(buffer);
			ret = ToValue(buffer, data);
		}
		else
			data = {};
		return ret;
	}

	static bool Save(Hydr10n::WindowHelpers::WindowMode data) { return Hydr10n::Data::AppData::Save(ToString(Section::DisplaySettings), KeyWindowMode, GetPath(), ToString(data)); }

	static bool Load(Hydr10n::WindowHelpers::WindowMode& data) {
		WCHAR buffer[12];
		bool ret = Hydr10n::Data::AppData::Load(ToString(Section::DisplaySettings), KeyWindowMode, GetPath(), buffer, ARRAYSIZE(buffer));
		if (ret) {
			RemoveSubstringStartingWithSpace(buffer);
			ret = ToValue(buffer, data);
		}
		else
			data = {};
		return ret;
	}

	static bool Save(const SIZE& data) {
		using Hydr10n::Data::AppData;
		const LPCWSTR lpcwSection = ToString(Section::DisplaySettings), lpcwPath = GetPath();
		return AppData::Save(lpcwSection, KeyResolutionWidth, lpcwPath, data.cx) && AppData::Save(lpcwSection, KeyResolutionHeight, lpcwPath, data.cy);
	}

	static bool Load(SIZE& data) {
		using Hydr10n::Data::AppData;
		const LPCWSTR lpcwSection = ToString(Section::DisplaySettings), lpcwPath = GetPath();
		const bool ret = AppData::Load(lpcwSection, KeyResolutionWidth, lpcwPath, data.cx) && AppData::Load(lpcwSection, KeyResolutionHeight, lpcwPath, data.cy);
		if (!ret)
			data = {};
		return ret;
	}

private:
	enum class Section { DisplaySettings, MiscellaneousSettings };

	using SectionKeyPair = std::pair<Section, LPCWSTR>;

	static constexpr LPCWSTR KeyWindowMode = L"WindowMode",
		KeyResolutionWidth = L"ResolutionWidth", KeyResolutionHeight = L"ResolutionHeight";

	static const std::wstring m_Path;

	static bool RemoveSubstringStartingWithSpace(wchar_t* str) {
		const std::wstring wstr(str);
		const auto iteratorBegin = wstr.cbegin(), iteratorEnd = wstr.cend(), iterator = std::find_if(iteratorBegin, iteratorEnd, [](wchar_t ch) { return iswspace((wint_t)ch); });
		if (iterator == iteratorEnd)
			return false;
		str[(iterator - iteratorBegin)] = 0;
		return true;
	}

	static SectionKeyPair ToSectionKeyPair(Key_bool val) {
		switch (val) {
		case Key_bool::ShowFPS: return SectionKeyPair(Section::DisplaySettings, L"ShowFPS");
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

	static LPCWSTR ToString(Hydr10n::WindowHelpers::WindowMode val) {
		constexpr LPCWSTR strs[] = { L"Windowed", L"Borderless", L"FullScreen" };
		return strs[(size_t)val];
	}

	static bool ToValue(LPCWSTR str, Hydr10n::WindowHelpers::WindowMode& val) {
		using Hydr10n::WindowHelpers::WindowMode;
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
};

decltype(MyAppSettingsData::m_Path) MyAppSettingsData::m_Path = std::filesystem::path(*__wargv).replace_filename("Settings.ini");