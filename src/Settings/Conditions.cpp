
#include "Conditions.h"
#include "Utility/Utility.h"
#include "Utility/KeyPress.h"
#include <cstdint>

#undef CompareString

namespace Conditions
{
	/*
	Conditions to add:

	Offhand Equip (staff, sword, etc.); maybe an option for form type and an option for speccific form
	Item Equipped (specific form or type)

	Previous Spell - Should be easy, but may work better as a separate mod
	Queue Number - definitely would be easier as a separate mod 
	Overcharge - may work better as a separate mod focused on under and over charging
	Detection State

	*/

	bool EvaluateCondition(const Condition& condition, RE::MagicCaster* caster)
	{
		if (!caster) {
			return false;
		}

		const std::string variable = detail::ToLower(condition.variable);
		const std::string op = condition.op;

		RE::Actor* actor = detail::GetCasterActor(caster);
		RE::MagicItem* magicItem = detail::GetCurrentMagicItem(caster);

		if (!actor || !magicItem) {
			return false;
		}

		logger::debug("Evaluating condition with variable: {}", variable);

		// -------------------------
		// String / categorical conditions
		// -------------------------

		if (variable == "magictype") {
			const auto expected = detail::GetStringValue(condition);
			if (!expected || !detail::IsEqualityOperator(op)) {
				return false;
			}
			return detail::CompareString(detail::GetMagicTypeString(magicItem), op, *expected);
		}

		if (variable == "casttype") {
			const auto expected = detail::GetStringValue(condition);
			if (!expected || !detail::IsEqualityOperator(op)) {
				return false;
			}
			return detail::CompareString(detail::GetCastingTypeString(magicItem), op, *expected);
		}

		if (variable == "school") {
			const auto expected = detail::GetStringValue(condition);
			if (!expected || !detail::IsEqualityOperator(op)) {
				return false;
			}
			return detail::CompareString(detail::GetSchoolString(magicItem), op, *expected);
		}

		if (variable == "caster") {
			const auto expected = detail::GetStringValue(condition);
			if (!expected || !detail::IsEqualityOperator(op)) {
				return false;
			}

			if (*expected == "player") {
				const bool isPlayer = actor->IsPlayerRef();
				return detail::CompareBool(isPlayer, op, true);
			}

			RE::TESNPC* expectedActor = Utility::TES::GetFormFromEditorID<RE::TESNPC>(*expected);

			if (!expectedActor) {
				return false;
			}
			
			return detail::CompareBool(actor->GetActorBase() == expectedActor, op, true);
		}

		if (variable == "keyword") {
			const auto expected = detail::GetStringValue(condition);
			if (!expected || !detail::IsEqualityOperator(op)) {
				return false;
			}

			RE::BGSKeyword* keyword = Utility::TES::GetFormFromEditorID<RE::BGSKeyword>(*expected);
			if (!keyword) {
				return false;
			}

			const bool hasKeyword = magicItem->HasKeyword(keyword);
			return detail::CompareBool(hasKeyword, op, true);
		}

		if (variable == "hand") {
			const auto expected = detail::GetStringValue(condition);
			if (!expected || !detail::IsEqualityOperator(op)) {
				return false;
			}
			return detail::CompareString(detail::GetHandString(caster), op, *expected);
		}

		if (variable == "perk") {
			const auto expected = detail::GetStringValue(condition);
			if (!expected || !detail::IsEqualityOperator(op)) {
				return false;
			}

			RE::BGSPerk* perk = Utility::TES::GetFormFromEditorID<RE::BGSPerk>(*expected);
			
			if (!perk) {
				return false;
			}

			const bool hasPerk = actor->HasPerk(perk);
			return detail::CompareBool(hasPerk, op, true);
		}

		if (variable == "form") {
			const auto expected = detail::GetStringValue(condition);
			if (!expected || !detail::IsEqualityOperator(op)) {
				return false;
			}

			RE::MagicItem* expectedMagicItem = Utility::TES::GetFormFromEditorID<RE::MagicItem>(*expected);
			
			if (!expectedMagicItem) {
				return false;
			}

			return detail::CompareBool(magicItem == expectedMagicItem, op, true);
		} 
		
		if (variable == "keypress") {
			auto expected = detail::GetIntValue(condition);
			auto key = static_cast<uint32_t>(*expected);
			logger::debug("Checking key code {}", key);
			if (KeyPress::g_keyState.IsKeyboardDown(key)) {
				return true;
			} else {
				return false;
			}
		}

		if (variable == "targetingactor" ){  
			
			RE::NiPoint3 start;
			RE::NiPoint3 dir;
			logger::debug("attempting to ray cast");
			auto result = Utility::RayCast::RayCast(start, dir, 10000, caster->GetCasterAsActor());
			auto hitRef = result.hitObjectRef;
			if (hitRef && hitRef->As<RE::Actor>()) {
				logger::debug("Raycasted an actor names {}", hitRef->GetName());  //it worked!!
			} 
			return detail::CompareBool((hitRef && hitRef->As<RE::Actor>()), op, true);
		}

		if (variable == "targetingref" ) {  //might work, might not, had some issues trying it on nazeem; might need to use form id instead. Feel like it has something to do with actors vs actorbase
			//test
			RE::NiPoint3 start;
			RE::NiPoint3 dir;
			logger::info("attempting to ray cast");
			auto result = Utility::RayCast::RayCast(start, dir, 10000, caster->GetCasterAsActor());
			auto hitRef = result.hitObjectRef;
			//might need to check if ref is an NPC. if NPC compare hitRef as actor->GetActorBase to variableDetail as TESNPC
			
			if (hitRef) {
				logger::info("Raycasted a ref named {}", hitRef->GetName());
				if (Utility::TES::GetFormFromEditorID<RE::TESNPC>(condition.variableDetail) && hitRef->As<RE::Actor>()) {
					logger::info("Raycasted an actor; comparing to NPC");
					return (hitRef->As<RE::Actor>()->GetActorBase() == Utility::TES::GetFormFromEditorID<RE::TESNPC>(condition.variableDetail));
				}

				if (Utility::TES::GetFormFromEditorID<RE::TESObjectREFR>(condition.variableDetail)) {
					logger::info("Raycasted an object ref; comparing to object ref");
					return (hitRef == Utility::TES::GetFormFromEditorID<RE::TESObjectREFR>(condition.variableDetail));
				}
			}
		}

		if (variable == "location") {

			logger::debug("checking location");
			const auto expected = detail::GetBoolValue(condition);
			if (!expected || condition.variableDetail.empty()) {
				return false;
			}
			auto currentLoc = actor->GetCurrentLocation();
				
			logger::debug("Getting condition location");
			auto condLoc = Utility::TES::GetFormFromEditorID<RE::BGSLocation>(condition.variableDetail);

			if (currentLoc)
				logger::debug("Current location is {} ({})", currentLoc->GetFullName(), currentLoc->GetFormID());
			if (condLoc && currentLoc) {
				logger::debug("Current location is {} and conditional location is {}", currentLoc->GetFullName(), condLoc->GetFullName());
				return detail::CompareBool(condLoc == currentLoc, op, *expected);
			}
		}

		// -------------------------
		// Numeric conditions
		// -------------------------

		if (variable == "skill") {
			const auto expected = detail::GetFloatValue(condition);
			if (!expected) {
				return false;
			}

			const RE::ActorValue schoolAV = magicItem->GetAssociatedSkill();
			if (schoolAV != RE::ActorValue::kNone) {
				const float currentValue = actor->AsActorValueOwner()->GetActorValue(schoolAV);
		
				return detail::CompareNumeric(currentValue, op, *expected);
			} else {
				return false;
			}
		}

		if (variable == "difficulty") {
			const auto expected = detail::GetFloatValue(condition);
			if (!expected) {
				return false;
			}
			auto costliest = magicItem->GetCostliestEffectItem();
			if (!costliest) {
				logger::warn("Costliest effect was null");
				return false;
			}

			auto baseEffect = costliest->baseEffect;
			if (!baseEffect) {
				logger::warn("Base effect was null");
				return false;
			}

			const float difficulty = baseEffect->GetMinimumSkillLevel();

			return detail::CompareNumeric(difficulty, op, *expected);
		}

		if (variable == "global") {
			const auto expected = detail::GetFloatValue(condition);
			if (!expected || condition.variableDetail.empty()) {
				return false;
			}

			RE::TESGlobal* global = Utility::TES::GetFormFromEditorID<RE::TESGlobal>(condition.variableDetail);
			
			if (!global) {
				return false;
			}

			return detail::CompareNumeric(global->value, op, *expected);
		}

		if (variable == "lightlevel") {

			const auto expected = detail::GetFloatValue(condition);
			
			if (!expected) {
				return false;
			}
			
			if (!actor->GetHighProcess()) {
				return false;
			}
			auto lightLevel = actor->GetHighProcess()->lightLevel;
			logger::debug("Checking Light level. Light level: {} Comparing to: {}", lightLevel, *expected);
			return detail::CompareNumeric(lightLevel, op, *expected);
			
		}

		float magicka = actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kMagicka);
		float health = actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kHealth);
		float stamina = actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina);
		float magickaPercent = 100 * (magicka / actor->AsActorValueOwner()->GetBaseActorValue(RE::ActorValue::kMagicka));
		float healthPercent = 100 * (health / actor->AsActorValueOwner()->GetBaseActorValue(RE::ActorValue::kHealth));
		float staminaPercent = 100 * (stamina / actor->AsActorValueOwner()->GetBaseActorValue(RE::ActorValue::kStamina));
		auto list = RE::ActorValueList::GetSingleton();
		auto av = list->LookupActorValueByName(variable.c_str());
		//May be able to use this to grab any AV including custom ones from AVG. Need to confirm where the AV name should be passed and if it should be the new custom AV name or the aliased one.

		if (variable == "magicka") {
			const auto expected = detail::GetFloatValue(condition);

			if (!expected) {
				return false;
			}

			return detail::CompareNumeric(magicka, op, *expected);

		} 
		if (variable == "health") {
			const auto expected = detail::GetFloatValue(condition);

			if (!expected) {
				return false;
			}

			return detail::CompareNumeric(health, op, *expected);

		}
		if (variable == "stamina") {
			const auto expected = detail::GetFloatValue(condition);

			if (!expected) {
				return false;
			}

			return detail::CompareNumeric(stamina, op, *expected);

		}
		if (variable == "magickapercent") {
			const auto expected = detail::GetFloatValue(condition);

			if (!expected) {
				return false;
			}

			return detail::CompareNumeric(magickaPercent, op, *expected);

		}
		if (variable == "healthpercent") {
			const auto expected = detail::GetFloatValue(condition);

			if (!expected) {
				return false;
			}

			return detail::CompareNumeric(healthPercent, op, *expected);

		}
		if (variable == "staminapercent") {
			const auto expected = detail::GetFloatValue(condition);

			if (!expected) {
				return false;
			}

			return detail::CompareNumeric(staminaPercent, op, *expected);
		}

		// -------------------------
		// Bool conditions
		// -------------------------

		if (variable == "issneaking") {
			const auto expected = detail::GetBoolValue(condition);
			if (!expected || !detail::IsEqualityOperator(op)) {
				return false;
			}

			return detail::CompareBool(actor->IsSneaking(), op, *expected);
		}

		if (variable == "isdualcasting") {
			const auto expected = detail::GetBoolValue(condition);
			if (!expected || !detail::IsEqualityOperator(op)) {
				return false;
			}

			const bool isDualCasting = caster->GetIsDualCasting();
			return detail::CompareBool(isDualCasting, op, *expected);
		}

		if (variable == "casterhaseffect") {
			const auto expected = detail::GetBoolValue(condition);
		
			if (!expected || condition.variableDetail.empty() || !detail::IsEqualityOperator(op)) {
				return false;
			}

			const auto effectEditorID = condition.variableDetail;
			auto checkedEffect = Utility::TES::GetFormFromEditorID<RE::EffectSetting>(effectEditorID);
			if (!checkedEffect) {
				return false;
			}
			bool hasEffect = false;

			auto& activeEffects = *actor->AsMagicTarget()->GetActiveEffectList();

			if (!&activeEffects) {
				return false;
			}
			
			for (auto& ae : activeEffects) {
					
				if (!ae) {
					continue;
				}
				if (!ae->effect || !ae->effect->baseEffect) {
					continue;
				}
				if (ae->effect->baseEffect == checkedEffect) {
					hasEffect = true;
				}
			}
			
			return detail::CompareBool(hasEffect, op, *expected);

		}

		if (variable == "casterhaseffectwithkeyword") {
			const auto expected = detail::GetBoolValue(condition);
			const auto effectKeyword = condition.variableDetail;

			if (!expected || condition.variableDetail.empty() || !detail::IsEqualityOperator(op)) {
				return false;
			}

			bool hasEffect = false;
			auto& activeEffects = *actor->AsMagicTarget()->GetActiveEffectList();

			if (!&activeEffects) {
				return false;
			}

			for (auto& ae : activeEffects) {
				if (!ae) {
					continue;
				}

				if (ae->effect->baseEffect->HasKeyword(effectKeyword)) {
					hasEffect = true;
				}
			}
			return detail::CompareBool(hasEffect, op, *expected);
		}

		return false;
		
	}

}

