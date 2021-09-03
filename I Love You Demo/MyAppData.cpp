#include "MyAppData.h"

decltype(MyAppData::Settings::m_AppData) MyAppData::Settings::m_AppData(std::filesystem::path(*__wargv).replace_filename("Settings.ini").c_str());