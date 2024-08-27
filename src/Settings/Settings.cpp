#include "Settings.h"
#include "Utility/Logger.h"

namespace Settings
{
	void Settings::ReadSettings() {

		constexpr auto defaultSettingsPath = L"Data/MCM/Config/DynamicMagicModificationFramework/settings.ini";
		constexpr auto mcmPath = L"Data/MCM/Settings/DynamicMagicModificationFramework.ini";

		auto dataHandler = RE::TESDataHandler::GetSingleton(); //may not need

		const auto readMCM = [&](std::filesystem::path path) {
			CSimpleIniA mcm;
			mcm.SetUnicode();

			mcm.LoadFile(path.string().c_str());

			//Other Settings
			ReadIntSetting(mcm, "Settings", "iLogLevel", iLogLevel);
			


		};
		logger::info("Reading MCM .ini...");

		readMCM(defaultSettingsPath);  // read the default ini first
		readMCM(mcmPath);

		logger::info("...success");
		Log::UpdateLogLevel(Settings::iLogLevel);

		//DirectionalMovementHandler::GetSingleton()->OnSettingsUpdated(); //this doesn't seem necessary for this mod

	}


	void Settings::ReadBoolSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, bool& a_setting)
	{
		const char* bFound = nullptr;
		bFound = a_ini.GetValue(a_sectionName, a_settingName);
		if (bFound) {
			a_setting = a_ini.GetBoolValue(a_sectionName, a_settingName);
		}
	}

	void Settings::ReadFloatSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, float& a_setting)
	{
		const char* bFound = nullptr;
		bFound = a_ini.GetValue(a_sectionName, a_settingName);
		if (bFound) {
			a_setting = static_cast<float>(a_ini.GetDoubleValue(a_sectionName, a_settingName));
		}
	}

	void Settings::ReadIntSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, int& a_setting)
	{
		const char* bFound = nullptr;
		bFound = a_ini.GetValue(a_sectionName, a_settingName);
		if (bFound) {
			a_setting = static_cast<int>(a_ini.GetLongValue(a_sectionName, a_settingName));
		}
	}
}
