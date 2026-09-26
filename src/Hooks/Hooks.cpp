#define NOMINMAX

#include "Utility/Utility.h"
#include "Utility/CreateForm.h"
#include "Hooks.h"
#include "DMMF/AlteredCast.h"
#include "DMMF/ChargeTime.h"
#include "DMMF/Magnitude.h"
#include "DMMF/Cost.h"
#include "DMMF/Resource.h"
#include "DMMF/AlternateAV.h"
#include "DMMF/Spell.h"
#include "Utility/MagicNode.h"
#include "Settings/Settings.h"

#include "DMMF/DMMF_API.h"


namespace Hooks
{
	using namespace ChargeTime;
	using namespace Cast;
	using namespace Resource;
	using namespace Cost;
	using namespace Magnitude;
	using namespace Spell;

	void EvaluateAPICalls(RE::ActorMagicCaster* caster)
	{
		//maybe I need a request value for each value
		//Get results from API
		auto result = DMMF_API::g_api.RequestValue(caster);

		auto& currentCast = Cast::GetCastInstance(caster);

		//CastTime
		if (!currentCast.charge) {
			currentCast.charge = std::make_unique<Cast::AlteredCharge>();
		}
		if (result.time >= 0) {
			currentCast.charge->newBaseTime = result.time;
			logger::info("API set base cast time to {}", result.time);
		}
		if (result.timeMod != 0) {
			currentCast.charge->modifiers.push_back(result.timeMod);
			logger::info("API added time modifier of {}", result.timeMult);
		}
		if (result.timeMult != 1) {
			currentCast.charge->modifiers.push_back(result.timeMult);
			logger::info("API added time multiplier of {}", result.timeMult);
		}

		//Cost
		if (!currentCast.cost) {
			currentCast.cost = std::make_unique<Cast::AlteredCost>();
		}
		if (result.cost >= 0) {
			currentCast.cost->newBaseCost = result.cost;
			logger::info("API set cost to {}", result.cost);
		}
		if (result.costMult != 1) {
			currentCast.cost->multipliers.push_back(result.costMult);
			logger::info("API added cost multiplier of {}", result.costMult);
		}
		if (result.costMod != 0) {
			currentCast.cost->modifiers.push_back(result.costMod);
			logger::info("API added cost multiplier of {}", result.costMod);
		}

		if (!currentCast.spell) {
			currentCast.spell = std::make_unique<Cast::AlteredSpell>();
		}
		if (result.spell) {
			currentCast.spell->modified = result.spell;
		}

		//Resource
		if (result.resource != RE::ActorValue::kNone) {
			currentCast.resource = result.resource;
			logger::info("API set resource to {}", result.resource);
		}
	}


	//-------------------
	// Charge Time Hooks
	// ------------------
	
	//If the initial charge time is 0 returns 0.0001 instead to allow for compatibility with casting bar. 
	float GetChargeTimeVHook::GetScrollChargeTime(RE::ScrollItem* spell)
	{
		if (spell) {
			float origin = funcScroll64(spell);
			if (origin == 0){
				return 0.0001;
			} else {
				return origin;
			}
		}

		return funcScroll64(spell);
	}

	//If the initial charge time is 0 returns 0.0001 instead to allow for compatibility with casting bar. 
	float GetChargeTimeVHook::GetEnchantChargeTime(RE::EnchantmentItem* spell)
	{
		if (spell && spell->GetSpellType() == RE::MagicSystem::SpellType::kStaffEnchantment) {
			
			float origin = funcEnchant64(spell);
			if (origin == 0) {
				return 0.0001;
			} else {
				return origin;
			}
			
		}

		return funcEnchant64(spell);
	}

	//If the initial charge time is 0 returns 0.0001 instead to allow for compatibility with casting bar. 
	float GetChargeTimeVHook::GetSpellChargeTime(RE::SpellItem* spell)
	{
		if (spell) {
			float origin = func64(spell);
			if (origin == 0) {
				return 0.0001;
			} else {
				return origin;
			}
		}

		return func64(spell);
	}

	//-------------------
	// ActorMagicCaster Hooks
	// ------------------

	//Initiate Cast is called first

	void CasterHook::RequestCastImpl(RE::ActorMagicCaster* caster)
	{
		logger::debug("RequestCast");

		if (!caster) {
			logger::info("RequestCast - Caster was null");
			return func3(caster);
		}
		auto currentSpell = caster->currentSpell;

		if (!currentSpell || (currentSpell->GetSpellType() != RE::MagicSystem::SpellType::kSpell && currentSpell->GetSpellType() != RE::MagicSystem::SpellType::kScroll && currentSpell->GetSpellType() != RE::MagicSystem::SpellType::kStaffEnchantment)) {
			logger::info("RequestCast - Not spell, staff, or scroll.");
			return func3(caster);
		}

		auto costliest = currentSpell->GetCostliestEffectItem()->baseEffect;
		if (costliest) {
			auto av = costliest->GetMagickSkill();
			auto* actor = caster->GetCasterAsActor();
			if (actor && av != RE::ActorValue::kNone) {
				auto& currentCast = Cast::GetCastInstance(caster);

				if (Settings::bReplaceVFX) {
					//Update spell sound and visuals incase it was changed during previous cast
					MagicNode::UpdateSpellSounds(caster, caster->currentSpell);
				}
				
				// Reset the AlteredCast to a clean state for this new cast
				currentCast.ResetForNewCast();

				// Ensure we have a spell container ready
				currentCast.spell = std::make_unique<AlteredSpell>();
				currentCast.spell->original = currentSpell;
				currentCast.spellItem = currentSpell;

				logger::debug("Reset AlteredCast for new cast instance");
			}
		}

		int i = 0;
		for (auto sound : caster->sounds) {
			logger::trace("For sound {}, soundID is {}", i, sound.soundID);
			i++;
		}

		//was in startcharge
		logger::debug("Current spell is {}", caster->currentSpell->GetFullName());
		
		EvaluateAPICalls(caster);  
		
		//Evaluate condtions and add appropriate changes to the AlteredCast instance
		AddChargeOverridesOnCast(caster);
		AddChargeMultipliersOnCast(caster);
		AddChargeModifiersOnCast(caster);
		AddResourceOnCast(caster);
		AddCostOverridesOnCast(caster);
		AddCostMultipliersOnCast(caster);
		AddCostModifiersOnCast(caster);
		AddMagOverridesOnCast(caster);
		AddMagMultipliersOnCast(caster);
		AddMagModifiersOnCast(caster);
		AddDurOverridesOnCast(caster);
		AddDurMultipliersOnCast(caster);
		AddDurModifiersOnCast(caster);
		AddReplacementSpellOnCast(caster);

		auto magic = caster->currentSpell;
		float origin = magic->GetChargeTime();

		//Calculate new charge time and cost
		CalculateNewChargeTime(caster, origin);
		CalculateNewCost(caster, true);

		auto& currentCast = Cast::GetCastInstance(caster);

		auto actor = caster->GetCasterAsActor();

		float chargeTime = origin;
		if (currentCast.charge) {
			chargeTime = currentCast.charge->updatedTime;
		}

		float cost = caster->currentSpellCost;
		if (currentCast.cost) {
			cost = currentCast.cost->updatedCost;
		}
		caster->currentSpellCost = cost;

		float ogMag = caster->currentSpell->GetCostliestEffectItem()->effectItem.magnitude;
		float newMag = ogMag;
		float ogDur = caster->currentSpell->GetCostliestEffectItem()->effectItem.duration;
		float newDur = ogDur;

		if (currentCast.magnitude) {
			logger::debug("calculating new magnitude");
			newMag = Magnitude::CalculateNewMag(ogMag, currentCast.magnitude.get());
		}
		if (currentCast.duration) {
			logger::debug("Calculating new duration.");
			newDur = Magnitude::CalculateNewDur(ogDur, currentCast.duration.get());
		}

		auto resource = currentCast.resource;

		//weird behavior when replacing spells with different casting types or delivery types. 
		//By default, only changes if new and old spells match casting type and delivery type. New options added to MCM to allow unmatched types.
		ReplaceSpell(caster);
		
		logger::info(
			"Starting charge for {}'s {} spell. Replacing with {}. Charge time is {} seconds, cost is {}, magnitude is {}, duration is {}, and resource used is {}\n",
			actor ? actor->GetDisplayFullName() : "<null actor>",
			magic ? magic->GetFullName() : "<null spell>",
			caster->currentSpell ? caster->currentSpell->GetFullName() : "<null spell>",
			chargeTime,
			cost,
			newMag,
			newDur,
			resource);

		func3(caster); 
	}
	
	void CasterHook::StartChargeImpl(RE::ActorMagicCaster* caster)
	{
		logger::debug("StartCharge");
		
		/* if (!caster) {
			logger::info("Caster was null");
			return func4(caster);
		}
		auto currentSpell = caster->currentSpell;

		if (!currentSpell || (currentSpell->GetSpellType() != RE::MagicSystem::SpellType::kSpell && currentSpell->GetSpellType() != RE::MagicSystem::SpellType::kScroll && currentSpell->GetSpellType() != RE::MagicSystem::SpellType::kStaffEnchantment)) {
			logger::info("Not spell, staff, or scroll.");
			return func4(caster);
		}*/

		func4(caster);
	}

	//Currently not used for anything
	void CasterHook::StartReadyImpl(RE::ActorMagicCaster* caster)
	{
		logger::debug("StartReady");
		
		logger::debug("Current spell is {}", caster->currentSpell->GetFullName());

		func5(caster);
	}

	void CasterHook::StartCastImpl(RE::ActorMagicCaster* caster) 
	{
		logger::debug("StartCastImpl");

		//It might still be valuable to re-evaluate replacement spells
		//could be re-evaluated at every step. Cost and charge time only really matter for the beginning, but replacement spell and magnitude could get updated at any time.
		//AddReplacementSpellOnCast(caster); //Check here in case conditions have changed
		//ReplaceSpell(caster); 
		
		logger::debug("Current spell is {}", caster->currentSpell->GetFullName());
		
		func6(caster);
	}

	//Doesn't seem to ever get called
	void CasterHook::FinishCastImpl(RE::ActorMagicCaster* caster)
	{

		logger::debug("FinishCastImpl - MagicCaster");
		func7(caster);
	}

	void CasterHook::InterruptCastHelper(RE::ActorMagicCaster* caster, RE::MagicSystem::CannotCastReason reason) { //seems to roughly correspond to ActorMagicCaster__sub_1405b1c20
		logger::debug("InterruptCastHelper");
		logger::debug("Cannot cast reason {}", reason);
		if (caster->currentSpell &&
			caster->actor == RE::PlayerCharacter::GetSingleton()) {
			RE::PlayerCharacter::GetSingleton()->PlayMagicFailureSound(caster->currentSpell->GetSpellType());
			//Will need to update this logic if I want it to flash the secondary meter. Or add this functionality within check cast.
			auto ResourceAcV = RE::MagicUtilities::GetAssociatedResource(caster->currentSpell, caster->castingSource);
			if (ResourceAcV == RE::ActorValue::kMagicka) { 
				ResourceAcV = UpdateCastingResource(caster);  
			}
			auto& currentCast = Cast::GetCastInstance(caster);
			auto SecondaryResourceAcV = currentCast.secondaryResource;

			if (SecondaryResourceAcV != RE::ActorValue::kNone) {
				auto cost = caster->currentSpellCost;
				auto primaryPool = AlternateAV::GetActorValue(caster->actor, ResourceAcV);

				auto secondaryPool = AlternateAV::GetActorValue(caster->actor, SecondaryResourceAcV);

				bool primaryFail = cost > primaryPool;
				bool secondaryFail = cost > secondaryPool;

				if (secondaryFail && !primaryFail) {
					logger::debug("Failure due to not enough secondary resource only.");
					ResourceAcV = SecondaryResourceAcV;
				}
				//could set it up so both flash, but I think it's fine for only one to flash for now.
			}

			switch (reason) 
			{
				case RE::MagicSystem::CannotCastReason::kMagicka:
					switch (ResourceAcV) {
					case RE::ActorValue::kMagicka:
						logger::debug("AV was Magicka");
						RE::HUDMenu::FlashMeter(ResourceAcV);
						break;

					case RE::ActorValue::kHealth:
						logger::debug("AV was Health");
						AlternateAV::FlashHealthMeter(caster->actor);
						break;

					case RE::ActorValue::kStamina:
						logger::debug("AV was stamina");
						AlternateAV::FlashStaminaMeter(caster->actor);
						break;
					default:
						RE::HUDMenu::FlashMeter(RE::ActorValue::kVoiceRate);
						break;
					}
					break;

				case RE::MagicSystem::CannotCastReason::kShoutWhileRecovering:
					{
						RE::HUDMenu::FlashMeter(RE::ActorValue::kVoiceRate);
						break;
					}

				default:
					break;
			}

			AlternateAV::ShowCannotCastReason(reason, ResourceAcV);
			if (reason == RE::MagicSystem::CannotCastReason::kItemCharge &&
				caster->currentSpell->GetSpellType() == RE::MagicSystem::SpellType::kStaffEnchantment) {
				//RE::TutorialMenu::OpenTutorialMenu(RE::DEFAULT_OBJECT::kHelpWeaponCharge); //this only seems to be defined for flatrim
			}
		}

		if (Settings::bReplaceVFX) {
			auto& currentCast = Cast::GetCastInstance(caster);
			currentCast.fired = false;
			if (currentCast.spell && currentCast.spell->original) {
				MagicNode::UpdateSpellVisualsAndSounds(caster, currentCast.spell->original);
			}
		}

		if (caster->actor) {
			AlternateAV::InterruptActor(caster->actor, caster->castingSource);
		}

		caster->SetCurrentSpell(nullptr);
		caster->flags.reset(
			RE::ActorMagicCaster::Flags::kNone,
			RE::ActorMagicCaster::Flags::kDualCasting);
	}

	void CasterHook::InterruptCastImpl(RE::ActorMagicCaster* caster, bool refund) {
		logger::debug("InterruptCastImpl");

		//only consider spells, scrolls, and staves
		if (!caster) {
			return func8(caster, refund);
		}
		auto currentSpell = caster->currentSpell;

		if (!currentSpell || (currentSpell->GetSpellType() != RE::MagicSystem::SpellType::kSpell && currentSpell->GetSpellType() != RE::MagicSystem::SpellType::kScroll && currentSpell->GetSpellType() != RE::MagicSystem::SpellType::kStaffEnchantment)) {
			return func8(caster, refund);
		}

		auto& currentCast = Cast::GetCastInstance(caster);

		if (Settings::bReplaceVFX) {
			
			currentCast.fired = false;
			if (currentCast.spell && currentCast.spell->original) {
				MagicNode::UpdateSpellVisualsAndSounds(caster, currentCast.spell->original);
			}
		}


		if (refund &&
			RE::MagicUtilities::UsesResourceWhileCharging(caster->currentSpell, caster->castingSource) &&
			caster->costCharged > 0.0f) {
			if (caster->state.get() <= RE::MagicCaster::State::kRelease ||
				caster->state.get() == RE::MagicCaster::State::kConcentrating) {
				auto ResourceAcV = RE::MagicUtilities::GetAssociatedResource(caster->currentSpell, caster->castingSource);
				if (ResourceAcV == RE::ActorValue::kMagicka) { 
					ResourceAcV = UpdateCastingResource(caster);  
				}
				if (ResourceAcV != RE::ActorValue::kNone) {
					AlternateAV::RestoreActorValue(caster->actor, ResourceAcV, caster->costCharged); 
				}
				
				auto SecondaryResourceAcV = currentCast.secondaryResource;
				auto SecondaryCost = caster->costCharged * currentCast.secondaryMult;
				
				if (SecondaryResourceAcV != RE::ActorValue::kNone && SecondaryResourceAcV != ResourceAcV) {
					AlternateAV::RestoreActorValue(caster->actor, SecondaryResourceAcV, SecondaryCost);
				}
			}
		}

		if (caster->flags.none(RE::ActorMagicCaster::Flags::kCheckDeferredInterrupt) ||
			caster->flags.all(RE::ActorMagicCaster::Flags::kDeferInterrupt)) {
			AlternateAV::InterruptActor(caster->actor, caster->castingSource);
		} else {
			caster->flags.reset(
				RE::ActorMagicCaster::Flags::kCheckDeferredInterrupt,
				RE::ActorMagicCaster::Flags::kDeferInterrupt);
			caster->flags.set(RE::ActorMagicCaster::Flags::kDeferInterrupt);
		}

		if (caster->interruptHandler) {
			caster->interruptHandler(caster->actor);
		}

		caster->flags.reset(
			RE::ActorMagicCaster::Flags::kNone,
			RE::ActorMagicCaster::Flags::kDualCasting);
	}

	void CasterHook::SpellCast(RE::ActorMagicCaster* caster, bool a_doCast, std::uint32_t a_arg2, RE::MagicItem* a_spell)
	{
		logger::info("SpellCast");

		if (!caster) {
			logger::info("Caster was null");
			return func9(caster, a_doCast, a_arg2, a_spell);
		}

		if (!a_spell) {
			logger::info("Spell was null");
			return func9(caster, a_doCast, a_arg2, a_spell);
		}

		if ((a_spell->GetSpellType() != RE::MagicSystem::SpellType::kSpell && a_spell->GetSpellType() != RE::MagicSystem::SpellType::kScroll && a_spell->GetSpellType() != RE::MagicSystem::SpellType::kStaffEnchantment)) {
			logger::info("Not spell, staff, or scroll.");
			return func9(caster, a_doCast, a_arg2, a_spell);
		}

		auto spell = a_spell->As<RE::SpellItem>();
		if (spell) {
			SpellCastImpl(caster, a_doCast, a_arg2, spell);
		}

		
	}

	void CasterHook::SpellCastImpl(RE::ActorMagicCaster* caster, bool a_success, std::uint32_t a_targetCount, RE::SpellItem* a_spell)
	{
		logger::info("SpellCastImpl called");

		if (!a_success) {
			return;
		}

		if (!a_spell) {
			if (!caster->currentSpell) {
				return;
			}

			a_spell = caster->currentSpell->As<RE::SpellItem>();
			if (!a_spell) {
				return;
			}
		}

		switch (a_spell->GetSpellType()) {
		case RE::MagicSystem::SpellType::kSpell:
		case RE::MagicSystem::SpellType::kDisease:
		case RE::MagicSystem::SpellType::kPower:
		case RE::MagicSystem::SpellType::kLesserPower:
			{
				caster->actor->AddCastPower(a_spell);
				break;
			}

		case RE::MagicSystem::SpellType::kVoicePower:
			{
				if (!caster->actor->IsCurrentShout(a_spell)) {
					break;
				}

				if (auto CurrentShout = caster->actor->GetCurrentShout()) {
					for (auto i = 0; i < 3; i++) {
						auto Variation = CurrentShout->variations[i];
						if (Variation.spell) {
							caster->actor->AddCastPower(Variation.spell);
							continue;
						}

						break;
					}
				}

				break;
			}

		case RE::MagicSystem::SpellType::kScroll:
			{
				caster->actor->RemoveCastScroll(a_spell, caster->castingSource);
				break;
			}

		default:
			break;
		}
	
		if (!a_spell->IsFood() && (a_spell->GetSpellType() != RE::MagicSystem::SpellType::kEnchantment || a_spell->GetDelivery() != RE::MagicSystem::Delivery::kTouch || a_targetCount) && RE::MagicUtilities::UsesResourceOnRelease(a_spell, caster->castingSource)) {
			auto MagickaCost = AlternateAV::GetMagickaCost(a_spell, caster->actor, caster->GetIsDualCasting());
			
			auto ResourceAcV = RE::MagicUtilities::GetAssociatedResource(a_spell, caster->castingSource);

			auto& currentCast = Cast::GetCastInstance(caster);
			auto SecondaryResourceAcV = currentCast.secondaryResource;
			auto SecondaryCost = MagickaCost * currentCast.secondaryMult;

			if (ResourceAcV == RE::ActorValue::kMagicka) {
				ResourceAcV = UpdateCastingResource(caster);  
			}
			
			if (ResourceAcV != RE::ActorValue::kNone && MagickaCost > 0.0f) {
				AlternateAV::DamageActorValue(caster->actor, ResourceAcV, MagickaCost);
			}

			if (SecondaryResourceAcV != RE::ActorValue::kNone && SecondaryCost > 0.0f) {
				AlternateAV::DamageActorValue(caster->actor, SecondaryResourceAcV, SecondaryCost);
			}
		}

		//if (a_spell->HasEffect(RE::EffectArchetype::kInvisibility)) {
		logger::debug("dispelling invisibility");
		caster->actor->DispelAlteredStates(RE::EffectArchetype::kInvisibility);
		//}

		//if (a_spell->HasEffect(RE::EffectArchetype::kEtherealize)) {
		logger::debug("dispelling etherealize");
			caster->actor->DispelAlteredStates(RE::EffectArchetype::kEtherealize);
		//}

		if (caster->actor == RE::PlayerCharacter::GetSingleton()) {
			if (auto EffectSetting = a_spell->GetAVEffect()) {
				if (auto ImageSpaceMod = EffectSetting->data.imageSpaceMod) {
					ImageSpaceMod->TriggerIfNotActive(1.0, nullptr);
				}
			}
		}

		if (auto ScriptEventSourceHolder = RE::ScriptEventSourceHolder::GetSingleton()) {
			if (auto RefHandle = caster->actor->CreateRefHandle()) {
				ScriptEventSourceHolder->SendSpellCastEvent(RefHandle.get(), a_spell->formID);
			}
		}

		auto& currentCast = Cast::GetCastInstance(caster);
	

		if (Settings::bReplaceVFX) {
			currentCast.fired = true;

			if (!currentCast.spell) {
				logger::warn("no current spell object; something weird has happened");
				return;
			}
			auto spellToRestore = currentCast.spell->original;

			SKSE::GetTaskInterface()->AddTask([caster, spellToRestore]() {
				if (!caster) {
					return;
				}
				if (!spellToRestore) {
					return;
				}

				MagicNode::UpdateCastingArt(caster, spellToRestore);
			});
		}
		
	}

	void CasterHook::SetCurrentSpellImpl(RE::ActorMagicCaster* caster, RE::MagicItem* spell) {
		logger::debug("SetCurrentSpellImpl Called");

		func10(caster, spell);
	}
	
	void CasterHook::SelectSpellImpl(RE::ActorMagicCaster* caster) {
		logger::debug("SelectSpellImpl Called");

		func11(caster);
	}
		
	void CasterHook::DeselectSpellImpl(RE::ActorMagicCaster* caster) {
		logger::debug("DeselectSpellImpl Called");

		func12(caster);
	}

	//need to add checks for secondaryAV
	bool CasterHook::CheckCast(RE::ActorMagicCaster* caster, RE::MagicItem* spell, bool dualCast, float* alchStrength, RE::MagicSystem::CannotCastReason* reason, bool useBaseValueForCost) {
		if (caster->state != RE::MagicCaster::State::kNone) {
			if (spell->GetSpellType() != RE::MagicSystem::SpellType::kEnchantment || caster->currentSpell->GetSpellType() != RE::MagicSystem::SpellType::kEnchantment) {
				logger::debug("CheckCast");

				AlternateAV::SafeSet(reason, RE::MagicSystem::CannotCastReason::kOK);
				if (caster->flags.any(RE::ActorMagicCaster::Flags::kSkipCheckCast)) {
					return true;
				}

				if (!spell) {
					if (!caster->currentSpell) {
						return false;
					}

					spell = caster->currentSpell;
					dualCast = caster->GetIsDualCasting();
				}

				auto bIsPlayer = caster->actor == RE::PlayerCharacter::GetSingleton();
				auto bIsPlayerAndGodMode = bIsPlayer && RE::PlayerCharacter::IsGodMode();

				if (bIsPlayerAndGodMode &&
					spell->GetSpellType() != RE::MagicSystem::SpellType::kWortCraft) {
					return true;
				}

				auto bHasResource{ true };
				auto bBlockMulticasting{ false };
				auto bBlockShoutRecovery{ false };
				auto bBlockWhileSCasting{ false };
				auto bBlockWhileShouting{ false };

				if (!bIsPlayerAndGodMode) {
					if (caster->actor->GetVoiceRecoveryTime() > 0.0f &&
						spell->GetSpellType() == RE::MagicSystem::SpellType::kVoicePower) {
						bBlockShoutRecovery = true;
					}

					auto MagickaCost = 0.0f;
					auto ResourceAcV = RE::MagicUtilities::GetAssociatedResource(spell, caster->castingSource);
					
					
					if (ResourceAcV == RE::ActorValue::kMagicka) {
						ResourceAcV = UpdateCastingResource(caster);  
					}
					if (ResourceAcV != RE::ActorValue::kNone) {

						if (auto* currentCast = Cast::FindCastInstance(caster); currentCast && currentCast->cost) {
							MagickaCost = currentCast->cost->updatedCost;
						} else {
							MagickaCost = AlternateAV::GetMagickaCost(spell, caster->actor, dualCast);
						}
					}

					auto& currentCast = Cast::GetCastInstance(caster);
					auto SecondaryResourceAcV = currentCast.secondaryResource;
					auto SecondaryCost = MagickaCost * currentCast.secondaryMult;

					auto bUsesResource{ false };
					switch (caster->state.get()) {
					case RE::MagicCaster::State::kNone:
					case RE::MagicCaster::State::kStart:
						{
							bUsesResource = ResourceAcV != RE::ActorValue::kNone;
							break;
						}

					case RE::MagicCaster::State::kRelease:
						{
							bUsesResource = RE::MagicUtilities::UsesResourceWhileCasting(spell, caster->castingSource);
							break;
						}

					case RE::MagicCaster::State::kCharging:
						{
							bUsesResource = RE::MagicUtilities::UsesResourceOnRelease(spell, caster->castingSource);
							break;
						}
					case RE::MagicCaster::State::kUnk05: //for some reason this is 5 instead of 6 for check cast.
						{
							bUsesResource = RE::MagicUtilities::UsesResourceWhileCasting(spell, caster->castingSource); //This ensures that bUsesResource will be set to true for concentration spells while casting. For some reason this is not necessary for when the resource is magicka, but is necessary if the resource is changed to something else.
							break;
						}

					default:
						break;
					}
					
					if (SecondaryResourceAcV != RE::ActorValue::kNone) {
						if (bUsesResource &&
							MagickaCost > 0.0f) {
							auto Magicka = (useBaseValueForCost) ? AlternateAV::GetBaseActorValue(caster->actor, ResourceAcV) : AlternateAV::GetActorValue(caster->actor, ResourceAcV);

							auto other = (useBaseValueForCost) ? AlternateAV::GetBaseActorValue(caster->actor, SecondaryResourceAcV) : AlternateAV::GetActorValue(caster->actor, SecondaryResourceAcV);

							//could still die with concentration spells when set to 1; updated to 5.
							if (Settings::bCanDieFromCastingWithHealth || (ResourceAcV != RE::ActorValue::kHealth && SecondaryResourceAcV != RE::ActorValue::kHealth)) { //should probably evaluate these separately and have additional branching options, though this should be fine, even if not perfect
								bHasResource = ((spell->GetCastingType() == RE::MagicSystem::CastingType::kConcentration) ? Magicka > 0.0f : bHasResource = Magicka >= MagickaCost) && ((spell->GetCastingType() == RE::MagicSystem::CastingType::kConcentration) ? other > 0.0f : bHasResource = other >= SecondaryCost);
							} else {
								bHasResource = ((spell->GetCastingType() == RE::MagicSystem::CastingType::kConcentration) ? Magicka > 5.0f : bHasResource = Magicka >= MagickaCost) && ((spell->GetCastingType() == RE::MagicSystem::CastingType::kConcentration) ? other > 5.0f : bHasResource = other >= SecondaryCost);
							}
						}
					} else {
						if (bUsesResource &&
							MagickaCost > 0.0f) {
							auto Magicka = (useBaseValueForCost) ? AlternateAV::GetBaseActorValue(caster->actor, ResourceAcV) : AlternateAV::GetActorValue(caster->actor, ResourceAcV);

							//could still die with concentration spells when set to 1; updated to 5.
							if (Settings::bCanDieFromCastingWithHealth || ResourceAcV != RE::ActorValue::kHealth) {
								bHasResource = (spell->GetCastingType() == RE::MagicSystem::CastingType::kConcentration) ? Magicka > 0.0f : bHasResource = Magicka >= MagickaCost;
							} else {
								bHasResource = (spell->GetCastingType() == RE::MagicSystem::CastingType::kConcentration) ? Magicka > 5.0f : bHasResource = Magicka >= MagickaCost;
							}
						}
					}
				}

				if (spell->GetSpellType() != RE::MagicSystem::SpellType::kEnchantment &&
					spell->GetCastingType() != RE::MagicSystem::CastingType::kConstantEffect) {
					auto Reason{ RE::MagicSystem::CannotCastReason::kOK };
					for (std::uint32_t i = 0; i < spell->effects.size(); i++) {
						if (auto effect = spell->effects[i]) {
							if (bIsPlayer ? !RE::PlayerCharacter::GetSingleton()->CheckCast(spell, effect, Reason) : !RE::ActiveEffectFactory::CheckCast(caster, spell, effect, Reason)) {
								AlternateAV::SafeSet(reason, Reason);
								return false;
							}

							if (auto BaseEffect = effect->baseEffect) {
								auto ActiveCasters = caster->actor->WhoIsCasting();
								if (BaseEffect->data.delivery == RE::MagicSystem::Delivery::kSelf &&
									BaseEffect->data.archetype == RE::EffectSetting::Archetype::kAccumulateMagnitude) {
									auto source = caster->castingSource;
									/*
									kLeftHand = 0,
									kRightHand = 1,
									kOther = 2,
									kInstant = 3
									*/
									int sourceInt;
									switch (source) {
									case RE::MagicSystem::CastingSource::kLeftHand:
										sourceInt = 0;
										break;
									case RE::MagicSystem::CastingSource::kRightHand:
										sourceInt = 1;
										break;
									case RE::MagicSystem::CastingSource::kOther:
										sourceInt = 2;
										break;
									case RE::MagicSystem::CastingSource::kInstant:
										sourceInt = 3;
										break;
									default:
										sourceInt = 0;
										break;
									}

									bBlockMulticasting = (ActiveCasters & ~(1 << sourceInt)) != 0;  //bBlockMulticasting = (ActiveCasters & ~(1 << stl::to_underlying(caster->castingSource))) != 0;
								}

								bBlockWhileShouting = (ActiveCasters & 4) != 0 &&
								                      caster->castingSource <= RE::MagicSystem::CastingSource::kRightHand;
								bBlockWhileSCasting = (ActiveCasters & 3) != 0 &&
								                      caster->castingSource == RE::MagicSystem::CastingSource::kOther;

								if (bBlockMulticasting || bBlockWhileSCasting || bBlockWhileShouting) {
									break;
								}
							}
						}
					}
				}

				if (bBlockShoutRecovery) {
					AlternateAV::SafeSet(reason, RE::MagicSystem::CannotCastReason::kShoutWhileRecovering);
				} else if (bHasResource) {
					if (bBlockMulticasting) {
						AlternateAV::SafeSet(reason, RE::MagicSystem::CannotCastReason::kMultipleCast);
					} else if (bBlockWhileShouting) {
						AlternateAV::SafeSet(reason, RE::MagicSystem::CannotCastReason::kCastWhileShouting);
					} else if (bBlockWhileSCasting) {
						AlternateAV::SafeSet(reason, RE::MagicSystem::CannotCastReason::kShoutWhileCasting);
					}
				} else {	
					AlternateAV::SafeSet(reason, RE::MagicUtilities::GetAssociatedResourceReason(spell, caster->castingSource));	
				}

				switch (spell->GetSpellType()) {
				case RE::MagicSystem::SpellType::kSpell:
				case RE::MagicSystem::SpellType::kPoison:
				case RE::MagicSystem::SpellType::kStaffEnchantment:
					{
						if (bHasResource && !bBlockWhileShouting) {
							return true;
						}

						return false;
					}

				case RE::MagicSystem::SpellType::kPower:
					{
						if (caster->actor->IsInCastPowerList(spell->As<RE::SpellItem>())) {
							AlternateAV::SafeSet(reason, RE::MagicSystem::CannotCastReason::kPowerUsed);
							return false;
						}

						if (bHasResource && !bBlockWhileShouting) {
							return true;
						}

						return false;
					}

				case RE::MagicSystem::SpellType::kLesserPower:
					{
						if (caster->actor->IsInCastPowerList(spell->As<RE::SpellItem>())) {
							return false;
						}

						if (bHasResource && !bBlockWhileShouting) {
							return true;
						}

						return false;
					}

				case RE::MagicSystem::SpellType::kWortCraft:
					{
						if (alchStrength) {
							if (spell->IsFood()) {
								*alchStrength = 0.0f;
							} else {
								auto AlchemyValue = caster->actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kAlchemy);
								*alchStrength = RE::MagicFormulas::GetWortcraftEffectStrength(AlchemyValue);
							}
						}

						return true;
					}

				case RE::MagicSystem::SpellType::kVoicePower:
					{
						if (bBlockShoutRecovery || bBlockMulticasting || bBlockWhileSCasting) {
							return false;
						}

						if (caster->state.get() == RE::MagicCaster::State::kCharging &&
							caster->actor->IsCurrentShout(spell->As<RE::SpellItem>()) &&
							caster->actor->IsInCastPowerList(spell->As<RE::SpellItem>())) {
							AlternateAV::SafeSet(reason, RE::MagicSystem::CannotCastReason::kPowerUsed);
							return false;
						}

						return true;
					}

				case RE::MagicSystem::SpellType::kScroll:
					{
						if (!bBlockMulticasting && !bBlockWhileShouting) {
							return true;
						}

						return false;
					}

				default:
					return true;
				}
			} else {
				funcA(caster, spell, dualCast, alchStrength, reason, useBaseValueForCost);
			}
		} else {
			funcA(caster, spell, dualCast, alchStrength, reason, useBaseValueForCost);
		}
	}

	/*
	*This function is called once every frame.CastingTimer is decremented, the casting cost actor value is drained, cast can be interrupted if actor value gets too low. 
	*We change this function to utilize a new delta (basically rate of updating) to simulate a certain charge time. As a result,  magicka drain is consistent with the new charge time.
	*By fully replacing the function with this RE'd version (rather than calling the original) we can alter other things, like the actorvalue used to cast spells, in addition to the effective charge time.
	*The original delta is intrinsically linked to framerate 
	* caster->UpdateImpl is an existing function which we call at the end of Update (as the original function does) which, amongst other things, is one of the avenues to advance from kCharge to KReady
	*Big credits to shad0wshayd3 and Fenix for REing what the function does and how it can be used
	*/ 

	//may want to look into changing how charge time is updated. Might be more efficient overall to do it by another method, but need to see.
	void CasterHook::Update(RE::ActorMagicCaster* caster, float delta)
	{

		//First two if statements limit when we actually alter the charge time.
	
		using S = RE::MagicCaster::State;
		auto state = caster->state.underlying();
		
		if (!caster) {
			logger::info("Update - Caster was null");
			return func1D(caster, delta);
		}
		
		//could basically do the same exact mod but have it be for enchantments could also do an enchantment equivalent to ControlledCasting (Engineered Enchanting perhaps?)
		if (state == static_cast<uint32_t>(S::kStart) || state == static_cast<uint32_t>(S::kCharging)) {
			if (auto a = caster->GetCasterAsActor(); a && caster->currentSpell && (caster->currentSpell->GetSpellType() == RE::MagicSystem::SpellType::kSpell || caster->currentSpell->GetSpellType() == RE::MagicSystem::SpellType::kStaffEnchantment || caster->currentSpell->GetSpellType() == RE::MagicSystem::SpellType::kScroll)) {
				float origin = caster->currentSpell->GetChargeTime();

				auto& currentCast = Cast::GetCastInstance(caster);

				if (!currentCast.charge) {
					currentCast.charge = std::make_unique<Cast::AlteredCharge>();
					currentCast.charge->caster = caster;
					currentCast.charge->newBaseTime = origin;
					currentCast.charge->updatedTime = origin;
				}

				float newTime = currentCast.charge->updatedTime;

				float k = newTime > 0.00001f ? (origin) / newTime : 1000000.0f;

				Update_Impl(caster, delta * k);
				if (!caster->castingArt) {
					RE::BSAnimationUpdateData updateData{};
					updateData.deltaTime = delta * k;
					updateData.flags = static_cast<std::uint16_t>(0x1000000);
					updateData.unk2C = true;
					updateData.unk2E = true;
					caster->UpdateAnimationGraphManager(updateData);
				}

				caster->UpdateImpl(delta * k);
			} else {
				Update_Impl(caster, delta);
				if (!caster->castingArt) {
					RE::BSAnimationUpdateData updateData{};
					updateData.deltaTime = delta;
					updateData.flags = static_cast<std::uint16_t>(0x1000000);
					updateData.unk2C = true;
					updateData.unk2E = true;
					caster->UpdateAnimationGraphManager(updateData);
				}

				caster->UpdateImpl(delta);
			}
		} else {
			if (false) {
				//logger::info("not charging or start");
				auto& currentCast = Cast::GetCastInstance(caster);
				if (currentCast.fired && !(caster->sounds[2].IsPlaying() || caster->sounds[3].IsPlaying())) {
					//logger::info("spell fired and release sound is not playing");
					//MagicNode::UpdateSpellSounds(caster, currentCast.spell->original);
					currentCast.fired = false;
				}
			}

			Update_Impl(caster, delta);
			if (!caster->castingArt) {
				RE::BSAnimationUpdateData updateData{};
				updateData.deltaTime = delta;
				updateData.flags = static_cast<std::uint16_t>(0x1000000);
				updateData.unk2C = true;
				updateData.unk2E = true;
				caster->UpdateAnimationGraphManager(updateData);
			}

			caster->UpdateImpl(delta);
		}	
		
	}

	//This is the RE'd 'innards' of the Update function. To simplify the above function, this is moved to its own function.
	//Again thanks and credits to shad0wshayd3
	void CasterHook::Update_Impl(RE::ActorMagicCaster* caster, float delta)
	{
		caster->CheckAttachCastingArt();
		if (!caster->currentSpell || !caster->actor) {
			return;
		}

		auto bIsPlayer = caster->actor == RE::PlayerCharacter::GetSingleton();
		if (caster->state.get() == RE::MagicCaster::State::kConcentrating && caster->currentSpell->GetCastingType() == RE::MagicSystem::CastingType::kConcentration) {
			if (caster->currentSpell->GetDelivery() == RE::MagicSystem::Delivery::kAimed) {
				if (bIsPlayer && ((caster->projectileTimer - delta) <= 0.0f)) {
					caster->actor->ProcessVATSAttack(
						caster,
						false,
						nullptr,
						caster->castingSource == RE::MagicSystem::CastingSource::kLeftHand);
				}
			} else {
				RE::MagicItem::SkillUsageData SkillUsageData{};
				if (caster->currentSpell->GetSkillUsageData(SkillUsageData)) {
					if (!SkillUsageData.custom) {
						caster->actor->UseSkill(
							SkillUsageData.skill,
							SkillUsageData.magnitude * delta,
							nullptr);
					}
				}
			}
		}

		//instead of one resource, have a list of resources and call for each later on; if that doesn't work, maybe have a primary that functions as normal and others are decremented manually
		//Need to also have a multiplier for each other attribute; for example, if you want the second AV to only cost half as much as the actual cost.

		//This could also be where cost in general is modified rather than changing the magiccaster costCharged thing
		auto ResourceAcV = RE::MagicUtilities::GetAssociatedResource(caster->currentSpell, caster->castingSource);
		if (ResourceAcV == RE::ActorValue::kMagicka) {
			ResourceAcV = UpdateCastingResource(caster);  // need to be careful with this regarding spell vs enchantment
		}
		if (ResourceAcV != RE::ActorValue::kNone &&
			(!bIsPlayer || !RE::PlayerCharacter::IsGodMode()) &&
			caster->flags.none(RE::ActorMagicCaster::Flags::kSkipCheckCast)) {
			auto bUsesWhileCasting = RE::MagicUtilities::UsesResourceWhileCasting(caster->currentSpell, caster->castingSource);
			auto bUsesWhileCharging = RE::MagicUtilities::UsesResourceWhileCharging(caster->currentSpell, caster->castingSource);
			auto MagickaPool = AlternateAV::GetActorValue(caster->actor, ResourceAcV);
			auto MagickaCost = caster->GetCurrentSpellCost(); 
			auto MagickaDiff = MagickaCost * delta;

			auto& currentCast = Cast::GetCastInstance(caster);
			auto SecondaryResourceAcV = currentCast.secondaryResource;
			auto SecondaryCost = MagickaCost * currentCast.secondaryMult;
			auto SecondaryDiff = SecondaryCost * delta;
			float SecondaryPool = 0;

			if (SecondaryResourceAcV != RE::ActorValue::kNone && SecondaryResourceAcV != ResourceAcV) {
				SecondaryPool = AlternateAV::GetActorValue(caster->actor, SecondaryResourceAcV);
			}

			if (caster->state.get() == RE::MagicCaster::State::kConcentrating && bUsesWhileCasting) {
				MagickaDiff = (std::min)(MagickaDiff, MagickaPool);  
				if (MagickaDiff > 0.0f) {
					AlternateAV::DamageActorValue(caster->actor, ResourceAcV, MagickaDiff);
				}
				if (SecondaryResourceAcV != RE::ActorValue::kNone && SecondaryResourceAcV != ResourceAcV) {
					SecondaryDiff = (std::min)(SecondaryDiff, SecondaryPool);
					if (SecondaryDiff > 0.0f) {
						AlternateAV::DamageActorValue(caster->actor, SecondaryResourceAcV, SecondaryDiff);
					}
				}
			} else if (caster->state.get() == RE::MagicCaster::State::kCharging && bUsesWhileCharging) {
				if (!bUsesWhileCasting) {
					float origin = caster->currentSpell->GetChargeTime();
					
					MagickaDiff /= origin;
					SecondaryDiff /= origin;
				}

				caster->costCharged += MagickaDiff;
				if (!bUsesWhileCasting && caster->costCharged > MagickaCost) {
					MagickaDiff -= (caster->costCharged - MagickaCost);
					caster->costCharged -= (caster->costCharged - MagickaCost);
				}

				if ((MagickaCost - caster->costCharged) <= MagickaPool) {
					AlternateAV::DamageActorValue(caster->actor, ResourceAcV, MagickaDiff);
				} else {
					caster->InterruptCast(false);
				}
				if (SecondaryResourceAcV != RE::ActorValue::kNone && SecondaryResourceAcV != ResourceAcV) {
					if ((SecondaryCost - (caster->costCharged * currentCast.secondaryMult)) <= SecondaryPool) {
						AlternateAV::DamageActorValue(caster->actor, SecondaryResourceAcV, SecondaryDiff);
					} else {
						caster->InterruptCast(false);
					}
				}
			}
		}

		if (bIsPlayer) {
			auto& ReticuleController = RE::ReticuleController::GetSingleton();
			if (ReticuleController.data.size() == 0) {
				return;
			}

			if (GetTickCount64() < ReticuleController.nextUpdate) {
				return;
			}

			ReticuleController.nextUpdate = GetTickCount64() + 200;
			if (ReticuleController.data.back() != caster->castingSource) {
				return;
			}

			RE::bhkPickData bhkPickData{};
			RE::NiPoint3 TargetLocation;
			RE::TESObjectCELL* TargetCell{ nullptr };
			caster->FindPickTarget(TargetLocation, &TargetCell, bhkPickData);

			auto bValidPosition{ true };
			if (bhkPickData.pickFailed || !bhkPickData.rayOutput.rootCollidable) {
				bValidPosition = false;
			} else {
				auto MagicItemDataCollector = RE::MagicItemDataCollector{ caster->currentSpell };
				caster->currentSpell->Traverse(MagicItemDataCollector);

				std::uint32_t i{ 0 };
				do {
					if (i >= MagicItemDataCollector.projectileEffectList.size()) {
						break;
					}

					auto Effect = MagicItemDataCollector.projectileEffectList[i++];
					bValidPosition = caster->TestProjectilePlacement(*Effect, bhkPickData);
				} while (bValidPosition);
			}

			RE::HUDMenu::UpdateCrosshairMagicTarget(bValidPosition);
		}
	}

	void CasterHook::AdjustActiveEffect(RE::MagicCaster* caster, RE::ActiveEffect* effect, float power, bool onlyAdjustHostile) {
		logger::debug("Adjust Active Effect Called");

		if (!caster) {
			logger::info("AdjustActiveEffect - No caster, returning without modification");
			return;
		}

		float newMag = effect->GetMagnitude();  //defaults as the original magnitude
		float ogMag = effect->GetMagnitude();
		float newDur = effect->duration;
		float ogDur = effect->duration;
		auto newEffMult = power;
		auto newDurMult = 1;

		logger::debug("Original magnitude is {}, original power is {}", ogMag, power);
		logger::debug("Original duration is {}", ogDur);

		//Rather than adjust power, should I adjust effect->magnitude directly? I think adjusting power may effect duration or magnitude depending on the magic effect flags (Power Affects Magnitude vs Duration)
		//Update Magnitude; This will not really effect spells that check the level of targets. Those spells don't take into consideration power when calculating effectiveness. Other function would need to be hooked, which unfortunately do not have an easy way to access the magic caster.
		auto& currentCast = Cast::GetCastInstance(caster);
		if (currentCast.magnitude) {
			logger::debug("calculating new magnitude");
			newMag = CalculateNewMag(ogMag, currentCast.magnitude.get());
			newEffMult = newMag / ogMag;

			//logger::debug("Calculated a new effectiveness multiplier of {}", newEffMult);
			//newEffMult = std::clamp(newEffMult, 0.0f, 1000.0f);
			//logger::debug("Clamped to {}", newEffMult);
			//logger::info("Adjusting magnitude from {} to {} with an effectiveness multiplier {}", ogMag, newMag, newEffMult);
			//power = newEffMult;
			effect->magnitude = newMag;
			logger::info("Adjusting magnitude from {} to {}", ogMag, newMag);

		}
		//Update Duration
		if (currentCast.duration) {
			logger::debug("Calculating a new duration.");
			newDur = CalculateNewDur(ogDur, currentCast.duration.get());
			
			logger::info("Adjusting duration from {} to {}.", ogDur, newDur);
			effect->duration = newDur;
		}

		func1C(caster, effect, power, onlyAdjustHostile);
	}

	// Install our hook at the specified address
	void GetChargeTimeVHook::Install()
	{
		logger::info("GetChargeTimeVHook hook set!");

		REL::Relocation<uintptr_t> SpellItemVtbl{ RE::VTABLE_SpellItem[0] };
		REL::Relocation<uintptr_t> EnchantItemVtbl{ RE::VTABLE_EnchantmentItem[0] };
		REL::Relocation<uintptr_t> ScrollItemVtbl{ RE::VTABLE_ScrollItem[0] };
		REL::Relocation<uintptr_t> MagicItemVtbl{ RE::VTABLE_MagicItem[0] };

		//Hooks the GetChargeTime function from respective class
		GetChargeTimeVHook::func64 = SpellItemVtbl.write_vfunc(0x64, &GetChargeTimeVHook::GetSpellChargeTime);
		GetChargeTimeVHook::funcScroll64 = ScrollItemVtbl.write_vfunc(0x64, &GetChargeTimeVHook::GetScrollChargeTime);
		GetChargeTimeVHook::funcEnchant64 = EnchantItemVtbl.write_vfunc(0x64, &GetChargeTimeVHook::GetEnchantChargeTime);
	}

	void CasterHook::Install()
	{
		logger::info("CasterHook hook set!");

		REL::Relocation<uintptr_t> ActorCasterVtbl{ RE::VTABLE_ActorMagicCaster[0] };
		REL::Relocation<uintptr_t> CasterVtbl{ RE::VTABLE_MagicCaster[0] };

		//Hooks the following functions from ActorMagicCaster
		CasterHook::func3 = ActorCasterVtbl.write_vfunc(0x3, &CasterHook::RequestCastImpl);
		CasterHook::func4 = ActorCasterVtbl.write_vfunc(0x4, &CasterHook::StartChargeImpl);
		CasterHook::func5 = ActorCasterVtbl.write_vfunc(0x5, &CasterHook::StartReadyImpl);
		CasterHook::func6 = ActorCasterVtbl.write_vfunc(0x6, &CasterHook::StartCastImpl);
		CasterHook::func7 = CasterVtbl.write_vfunc(0x7, &CasterHook::FinishCastImpl);
		CasterHook::func8 = ActorCasterVtbl.write_vfunc(0x8, &CasterHook::InterruptCastImpl);
		CasterHook::func9 = ActorCasterVtbl.write_vfunc(0x9, &CasterHook::SpellCast);
		//CasterHook::func10 = ActorCasterVtbl.write_vfunc(0x10, &CasterHook::SetCurrentSpellImpl);
		//CasterHook::func11 = ActorCasterVtbl.write_vfunc(0x11, &CasterHook::SelectSpellImpl);
		//CasterHook::func12 = ActorCasterVtbl.write_vfunc(0x12, &CasterHook::DeselectSpellImpl);
		CasterHook::funcA = ActorCasterVtbl.write_vfunc(0xA, &CasterHook::CheckCast);
		CasterHook::func1D = ActorCasterVtbl.write_vfunc(0x1D, &CasterHook::Update);
		CasterHook::func1C = ActorCasterVtbl.write_vfunc(0x1C, &CasterHook::AdjustActiveEffect);
		//CasterHook::funcE = ActorCasterVtbl.write_vfunc(0xE, &CasterHook::GetMagicNode);  //doesn't seem to be too useful at the moment.

		{
			REL::Relocation<std::uintptr_t> target1{ RELOCATION_ID(33355, 34136), OFFSET(0x51, 0x51) };  //OFFSET(0x51, 0x51)
			REL::Relocation<std::uintptr_t> target2{ RELOCATION_ID(33358, 34139), OFFSET(0xB9, 0xB9) };  // OFFSET(0xB9, 0xB9)

		    //REL::Relocation<std::uintptr_t> target3{ RELOCATION_ID(33629, 34407) }; //FindTargets
			//REL::Relocation<std::uintptr_t> target3{ RELOCATION_ID(33623, 34401), OFFSET(0x11D, 0x11D) };  //1405BB720 + 0x11D Call site of SetSpellandTimer withing InitiateCast

			//REL::Relocation<std::uintptr_t> target4{ RELOCATION_ID(33629, 34407), OFFSET(0xE9, 0xE4) }; //Call site of FindTargets within Fire
			//REL::Relocation<std::uintptr_t> target4{ RELOCATION_ID(33273, 34048), OFFSET(0x3F, 0x48) }; //call site of HandleEntryPoint within sub function within ShouldApply

			auto& trampoline = SKSE::GetTrampoline();
			

			trampoline.write_call<5>(target1.address(), InterruptCastHelper);
			trampoline.write_call<5>(target2.address(), InterruptCastHelper);
			
			//_SetSpellandTimer = trampoline.write_call<5>(target3.address(), SetSpellandTimer); //call from initiatecast
			//_FindTargets = trampoline.write_call<5>(target4.address(), FindTargets);
			//REL::Relocation<std::uintptr_t> functionc{ RELOCATION_ID(33629, 34407), REL::Relocate(0xE9, 0xE4) };
			//stl::write_thunk_call<FindTargetsHook>(functionc.address());

			//REL::Relocation<std::uintptr_t> functionce{ RELOCATION_ID(33672, 34452), REL::Relocate(0x377, 0x354) };
			//stl::write_thunk_call<FindTargetHook>(functionce.address());

			//REL::Relocation<std::uintptr_t> function2{ RELOCATION_ID(33375, 34156)};
			//stl::write_thunk_branch<ResetNodeHook>(function2.address());  //AE 34156


		}
	}
}


