#include "PluginData.h"
#include "Utility/Utility.h"

namespace PluginData
{
	void GetForms()
	{
		logger::info("Getting Plugin Forms");
		//castingTimeMult = Utility::TES::GetFormFromEditorID<RE::EffectSetting>("_DMMF_MGEF_FF_CastingTimeMult");

		//castingTimeMultConst = Utility::TES::GetFormFromEditorID<RE::EffectSetting>("_DMMF_MGEF_CONST_CastingTimeMult");

		PositiveCastTimeMultKYWD = Utility::TES::GetFormFromEditorID<RE::BGSKeyword>("_DMMF_KYWD_PositiveCastTimeMult");
		NegativeCastTimeMultKYWD = Utility::TES::GetFormFromEditorID<RE::BGSKeyword>("_DMMF_KYWD_NegativeCastTimeMult");
		HealthResourceKYWD = Utility::TES::GetFormFromEditorID<RE::BGSKeyword>("_DMMF_KYWD_HealthResource");
		StaminaResourceKYWD = Utility::TES::GetFormFromEditorID<RE::BGSKeyword>("_DMMF_KYWD_StaminaResource");
		MagickaResourceKYWD = Utility::TES::GetFormFromEditorID<RE::BGSKeyword>("_DMMF_KYWD_MagickaResource");
		PositiveCostMultKYWD = Utility::TES::GetFormFromEditorID<RE::BGSKeyword>("_DMMF_KYWD_PositiveCostMult");
		NegativeCostMultKYWD = Utility::TES::GetFormFromEditorID<RE::BGSKeyword>("_DMMF_KYWD_NegativeCostMult");
	}
}
