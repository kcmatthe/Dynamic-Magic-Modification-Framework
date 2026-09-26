#pragma once

namespace Settings
{
	//Functions
	
	void ReadSettings();


	static void ReadBoolSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, bool& a_setting);
	static void ReadFloatSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, float& a_setting);
	static void ReadIntSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, int& a_setting);
	
	
	inline int iLogLevel = 2;
	inline bool bReplaceVFX = false; //spell visuals and sounds are replaced; may causes crashes with EngineFixes unless one option is turned off
	inline bool bAllowUnmatchedCastType = false; //allows spells with different casting types to be replaced; some work, some don't, all have weird visuals
	inline bool bAllowUnmatchedDeliveryType = false; //allows spells with different delivery types to be replace; all should work if cast type is the same, but the animations may look weird
	inline bool bCanDieFromCastingWithHealth = false; //Can die from casting a spell using health. Primarily effects concentration spells.

	//may be an interesting future option to allow changing animations

}
