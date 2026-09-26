#include "AlternateAV.h"
#include "TrueHUDAPI.h"
#include "Settings/Config.h"
#include "Settings/Conditions.h"

namespace AlternateAV
{
	bool FlashTrueHUDMeter(RE::Actor* a_actor, RE::ActorValue a_actorValue, bool a_long)
	{
		auto TrueHUDAPI = static_cast<TRUEHUD_API::IVTrueHUD4*>(TRUEHUD_API::RequestPluginAPI(TRUEHUD_API::InterfaceVersion::V4));
		if (!TrueHUDAPI) {
			return false;
		}

		TrueHUDAPI->FlashActorValue(a_actor->GetHandle(), a_actorValue, a_long);
		return true;
		
	}
	
	void RestoreActorValue(RE::Actor* a_actor, RE::ActorValue a_actorValue, float a_value) {
		return a_actor->AsActorValueOwner()->RestoreActorValue(a_actorValue, a_value);
	}

	void DamageActorValue(RE::Actor* a_actor, RE::ActorValue a_actorValue, float a_value)
	{
		return a_actor->AsActorValueOwner()->DamageActorValue(a_actorValue, a_value);
	}

	//This doesn't actually do anything lmao; vanilla hud.swf doesn't have a flashing health bar; only works with TrueHUD installed
	void FlashHealthMeter(RE::Actor* a_actor)
	{
		logger::trace("FlashHealthMeter Called");
		if (FlashTrueHUDMeter(a_actor, RE::ActorValue::kHealth, true)) {
			//logger::info("trueHUD flash instead");
			return;
		}

		auto UI = RE::UI::GetSingleton();
		if (!UI) {
			//logger::info("no UI");
			return;
		}

		auto HUDMenu = UI->GetMenu<RE::HUDMenu>();
		if (!HUDMenu) {
			//logger::info("no hud menu");
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
		} else {
			logger::debug("did not flash health, flashing voice instead");
			RE::HUDMenu::FlashMeter(RE::ActorValue::kVoiceRate);
		}
	}

	void FlashStaminaMeter(RE::Actor* a_actor)
	{
		logger::trace("FlashStaminaMeter Called");
		if (FlashTrueHUDMeter(a_actor, RE::ActorValue::kStamina, true)) {
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

		RE::GFxValue Stamina, StaminaFlash;
		if (!HUDMenu->GetRuntimeData().root.GetMember("Stamina", &Stamina)) {
			return;
		}

		if (Stamina.GetMember("StaminaFlashInstance", &StaminaFlash)) {
			std::array<RE::GFxValue, 1> args;
			Stamina.GetMember("_currentFrame", args.data());
			Stamina.Invoke("PlayForward", args);
			StaminaFlash.GotoAndPlay("StartFlash");
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

	
	//also if using custom AVs, need to have this work for any AV and just fill in the blank with the AV name.
	//now supports localization
	void ShowCannotCastReason(RE::MagicSystem::CannotCastReason a_reason, RE::ActorValue av)
	{
		logger::trace("ShowCannotCastReason called");
		switch (a_reason) {
		case RE::MagicSystem::CannotCastReason::kMagicka:

			switch (av) {
			case RE::ActorValue::kMagicka:
				return RE::SendHUDMessage::ShowHUDMessage(config::magicka.c_str(), nullptr, true);
				break;

			case RE::ActorValue::kHealth:
				return RE::SendHUDMessage::ShowHUDMessage(config::health.c_str(), nullptr, true);
				break;

			case RE::ActorValue::kStamina:
				return RE::SendHUDMessage::ShowHUDMessage(config::stamina.c_str(), nullptr, true);
				break;
			default:
				return RE::SendHUDMessage::ShowHUDMessage(config::energy.c_str(), nullptr, true);
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

		RE::SendHUDMessage::ShowHUDMessage(CannotCastString, nullptr, true);
	}
}
