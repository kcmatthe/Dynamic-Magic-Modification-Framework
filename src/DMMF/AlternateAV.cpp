#include "AlternateAV.h"
//#include "TrueHUDAPI.h"
#include "Settings/Config.h"

namespace AlternateAV
{
	bool FlashTrueHUDMeter(RE::Actor* a_actor, RE::ActorValue a_actorValue, bool a_long)
	{
		/* auto TrueHUDAPI = static_cast<TRUEHUD_API::IVTrueHUD4*>(TRUEHUD_API::RequestPluginAPI(TRUEHUD_API::InterfaceVersion::V4));
		if (!TrueHUDAPI) {
			return false;
		}

		TrueHUDAPI->FlashActorValue(a_actor->GetHandle(), a_actorValue, a_long);
		return true;*/
	}

	void DamageActorValue(RE::Actor* a_actor, RE::ActorValue a_actorValue, float a_value)
	{
		return a_actor->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, a_actorValue, a_value);
	}

	void FlashHealthMeter(RE::Actor* a_actor)
	{
		if (FlashTrueHUDMeter(a_actor, RE::ActorValue::kHealth, true)) {
			return;
		}

		auto UI = RE::UI::GetSingleton();
		if (!UI) {
			return;
		}

		auto HUDMenu = UI->GetMenu<RE::HUDMenu>();
		if (!HUDMenu) {
			return;
		}

		RE::GFxValue Health, HealthFlash;
		if (!HUDMenu->GetRuntimeData().root.GetMember("Health", &Health)) {
			return;
		}

		if (Health.GetMember("HealthFlashInstance", &HealthFlash)) {
			std::array<RE::GFxValue, 1> args;
			Health.GetMember("_currentFrame", args.data());
			Health.Invoke("PlayForward", args);
			HealthFlash.GotoAndPlay("StartFlash");
		}
	}

	float GetActorValue(RE::Actor* a_actor, RE::ActorValue a_actorValue)
	{
		return a_actor->AsActorValueOwner()->GetActorValue(a_actorValue);
	}

	float GetBaseActorValue(RE::Actor* a_actor, RE::ActorValue a_actorValue)
	{
		return a_actor->AsActorValueOwner()->GetBaseActorValue(a_actorValue);
	}

	float GetMagickaCost(RE::MagicItem* a_magicItem, RE::Actor* a_actor, bool a_dualCast)
	{
		auto MagickaCost = a_magicItem->CalculateMagickaCost(a_actor);
		if (a_dualCast) {
			MagickaCost = RE::MagicFormulas::CalcDualCastCost(MagickaCost);
		}

		return MagickaCost;
	}

	std::int32_t GetStepCount()
	{
		auto PlayerCharacter = RE::PlayerCharacter::GetSingleton();
		if (!PlayerCharacter) {
			return 0;
		}

		auto HealthDamage = PlayerCharacter->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kHealth);
		if (HealthDamage >= 0.0) {
			return 0;
		}

		HealthDamage = abs(HealthDamage);
		return static_cast<std::int32_t>(floor(HealthDamage));  /// Settings::Modifiers::fBloodMagicStepRate));
	}

	void InterruptActor(RE::Actor* a_actor, RE::MagicSystem::CastingSource a_castingSource)
	{
		switch (a_castingSource) {
		case RE::MagicSystem::CastingSource::kLeftHand:
			RE::SourceActionMap::DoAction(a_actor, RE::DEFAULT_OBJECT::kActionLeftInterrupt);
			break;
		case RE::MagicSystem::CastingSource::kRightHand:
			RE::SourceActionMap::DoAction(a_actor, RE::DEFAULT_OBJECT::kActionRightInterrupt);
			break;
		case RE::MagicSystem::CastingSource::kOther:
			RE::SourceActionMap::DoAction(a_actor, RE::DEFAULT_OBJECT::kActionVoiceInterrupt);
			break;
		default:
			break;
		}
	}

	/*
	template <typename T>
	void SafeSet(T* a_ptr, T a_value)
	{
		if (a_ptr) {
			*a_ptr = a_value;
		}
	}
	*/

	void ShowCannotCastReason(RE::MagicSystem::CannotCastReason a_reason, RE::ActorValue av)
	{
		switch (a_reason) {
		case RE::MagicSystem::CannotCastReason::kMagicka:

			switch (av) {
			case RE::ActorValue::kMagicka:
				return RE::DebugNotification("You don't have enough Magicka", nullptr, true);
				break;

			case RE::ActorValue::kHealth:
				return RE::DebugNotification("You don't have enough Health", nullptr, true);
				break;

			case RE::ActorValue::kStamina:
				return RE::DebugNotification("You don't have enough Stamina", nullptr, true);
				break;
			default:
				return RE::DebugNotification("You don't have enough Energy", nullptr, true);
				break;
			}

			break;

		default:
			break;
		}

		auto CannotCastString = RE::MagicSystem::GetCannotCastString(a_reason);
		if (!CannotCastString) {
			return;
		}

		RE::DebugNotification(CannotCastString, nullptr, true);
	}
}
