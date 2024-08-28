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


namespace Hooks
{
	using namespace ChargeTime;
	using namespace Cast;
	using namespace Resource;
	using namespace Cost;
	using namespace Magnitude;
	using namespace Spell;
	
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

	void CasterHook::RequestCastImpl(RE::MagicCaster* caster)
	{
		logger::trace("RequestCast");

		if (caster) {
			auto spell = caster->currentSpell;
			if (spell && (spell->GetSpellType() == RE::MagicSystem::SpellType::kSpell || caster->currentSpell->GetSpellType() == RE::MagicSystem::SpellType::kStaffEnchantment || caster->currentSpell->GetSpellType() == RE::MagicSystem::SpellType::kScroll)) {
				auto costliest = spell->GetCostliestEffectItem()->baseEffect;
				if (costliest) {
					auto av = costliest->GetMagickSkill();
					auto* actor = caster->GetCasterAsActor();
					if (actor && av != RE::ActorValue::kNone) {
						auto it = GetCastInstance(caster);
						auto currentCast = *it;
						if (!currentCast.spell) {
							currentCast.spell = new AlteredSpell();
						}
						auto modifiedSpell = currentCast.spell->modified;
						if (modifiedSpell) {
							for (auto effect : modifiedSpell->effects) {
								logger::debug("Attempting to delete created effect {}", effect->baseEffect->fullName.c_str());
								DeleteCreatedForm(effect->baseEffect);
							}
							logger::debug("Attempting to delete created spell {}", modifiedSpell->fullName.c_str());
							DeleteCreatedForm(modifiedSpell);
						}
						casts.erase(it);
						logger::debug("cast instance erased");
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
						AddReplacementSpellOnCast(caster);
						
					}
				}
			}
		}

		func3(caster);
	}
	
	void CasterHook::StartChargeImpl(RE::MagicCaster* caster)
	{
		logger::trace("StartCharge");

		

		auto magic = caster->currentSpell;
		float origin = magic->GetChargeTime();

		CalculateNewChargeTime(caster, origin, true);
		CalculateNewCost(caster, true);
		ReplaceSpell(caster);  //Called here so that modifymagnitude won't overwrite replaced spell
		ModifyMagnitude(caster);
		auto it = GetCastInstance(caster);
		auto currentCast = *it;
		auto chargeTime = currentCast.charge->updatedTime;
		auto actor = caster->GetCasterAsActor();
		auto cost = currentCast.cost->updatedCost;
		caster->currentSpellCost = cost;

		logger::info("Starting charge for {}'s {} spell. Charge time is {} seconds, cost is {}\n", magic->GetFullName(), actor->GetDisplayFullName(), chargeTime, cost);

		func4(caster);
	}

	void CasterHook::StartReadyImpl(RE::MagicCaster* caster)
	{
		logger::trace("StartReady");

		func5(caster);
	}

	void CasterHook::StartCastImpl(RE::MagicCaster* caster) 
	{
		logger::trace("StartCastImpl");
		//This is where unattuned is currently hooking
		//Should I replace then modify, or modify then replace?
		//What if I determine this stuff during the charge and add it to the cast instance
		ReplaceSpell(caster); //Called again to replace spell if magnitude changed
		//ModifyMagnitude(caster);
		
		func6(caster);
	}

	//This doesn't seem to get called
	void CasterHook::FinishCastImpl(RE::MagicCaster* caster)
	{
		logger::trace("FinishCast");
		auto spell = caster->currentSpell;
		logger::info("Finished cast for spell {}", spell->fullName.c_str());
		func7(caster);
	}

	void CasterHook::InterruptCastHelper(RE::ActorMagicCaster* caster, RE::MagicSystem::CannotCastReason reason) {
		logger::trace("InterruptCastHelper");
		if (caster->currentSpell &&
			caster->actor == RE::PlayerCharacter::GetSingleton()) {
			RE::PlayerCharacter::GetSingleton()->PlayMagicFailureSound(caster->currentSpell->GetSpellType());
			auto ResourceAcV = RE::MagicUtilities::GetAssociatedResource(caster->currentSpell, caster->castingSource);
			if (ResourceAcV == RE::ActorValue::kMagicka) {
				ResourceAcV = UpdateCastingResource(caster);  // need to be careful with this regarding spell vs enchantment
			}
			switch (reason) {
			case RE::MagicSystem::CannotCastReason::kMagicka:
				{
					
					RE::HUDMenu::FlashMeter(ResourceAcV);
					break;
				}

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

		if (caster->actor) {
			AlternateAV::InterruptActor(caster->actor, caster->castingSource);
		}

		caster->SetCurrentSpell(nullptr);
		caster->flags.reset(
			RE::ActorMagicCaster::Flags::kNone,
			RE::ActorMagicCaster::Flags::kDualCasting);
	}

	void CasterHook::InterruptCastImpl(RE::ActorMagicCaster* caster, bool refund) {
		logger::trace("InterruptCastImpl");

		if (refund &&
			RE::MagicUtilities::UsesResourceWhileCharging(caster->currentSpell, caster->castingSource) &&
			caster->costCharged > 0.0f) {
			if (caster->state.get() <= RE::MagicCaster::State::kRelease ||
				caster->state.get() == RE::MagicCaster::State::kConcentrating) {
				auto ResourceAcV = RE::MagicUtilities::GetAssociatedResource(caster->currentSpell, caster->castingSource);
				if (ResourceAcV == RE::ActorValue::kMagicka) {
					ResourceAcV = UpdateCastingResource(caster);  // need to be careful with this regarding spell vs enchantment
				}
				if (ResourceAcV != RE::ActorValue::kNone) {
					AlternateAV::DamageActorValue(caster->actor, ResourceAcV, caster->costCharged);
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
		
		if (a_spell) {
			if (a_spell->GetSpellType() != RE::MagicSystem::SpellType::kEnchantment) {
				logger::trace("SpellCast\n");
				auto spell = a_spell->As<RE::SpellItem>();
				if (spell) {
					SpellCastImpl(caster, a_doCast, a_arg2, spell);
				}
			} else {
				func9(caster, a_doCast, a_arg2, a_spell);
			}
		} else {
			func9(caster, a_doCast, a_arg2, a_spell);
		}
	}

	void CasterHook::SpellCastImpl(RE::ActorMagicCaster* caster, bool a_success, std::uint32_t a_targetCount, RE::SpellItem* a_spell)
	{
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
			if (ResourceAcV == RE::ActorValue::kMagicka) {
				ResourceAcV = UpdateCastingResource(caster);  // need to be careful with this regarding spell vs enchantment
			}
			
			if (ResourceAcV != RE::ActorValue::kNone && MagickaCost > 0.0f) {
				AlternateAV::DamageActorValue(caster->actor, ResourceAcV, -MagickaCost);
			}
		}

		if (a_spell->HasEffect(RE::EffectArchetype::kInvisibility)) {
			caster->actor->DispelAlteredStates(RE::EffectArchetype::kInvisibility);
		}

		if (a_spell->HasEffect(RE::EffectArchetype::kEtherealize)) {
			caster->actor->DispelAlteredStates(RE::EffectArchetype::kEtherealize);
		}

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
	}

	bool CasterHook::CheckCast(RE::ActorMagicCaster* caster, RE::MagicItem* spell, bool dualCast, float* alchStrength, RE::MagicSystem::CannotCastReason* reason, bool useBaseValueForCost) {
		if (caster->state != RE::MagicCaster::State::kNone) {
			if (spell->GetSpellType() != RE::MagicSystem::SpellType::kEnchantment || caster->currentSpell->GetSpellType() != RE::MagicSystem::SpellType::kEnchantment) {
				logger::trace("CheckCast");

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
						ResourceAcV = UpdateCastingResource(caster);  // need to be careful with this regarding spell vs enchantment
					}
					if (ResourceAcV != RE::ActorValue::kNone) {
						MagickaCost = AlternateAV::GetMagickaCost(spell, caster->actor, dualCast);
					}

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
					logger::info("State: {}, av: {}, cost: {}, value: {}", caster->state.get(), ResourceAcV, MagickaCost);
					if (bUsesResource &&
						MagickaCost > 0.0f) {
						auto Magicka = (useBaseValueForCost) ? AlternateAV::GetBaseActorValue(caster->actor, ResourceAcV) : AlternateAV::GetActorValue(caster->actor, ResourceAcV);
						logger::info("State: {}, av: {}, cost: {}, value: {}", caster->state.get(), ResourceAcV, MagickaCost, Magicka);
						if (ResourceAcV == RE::ActorValue::kHealth) { 
							bHasResource = Magicka >= MagickaCost;//This ensures that player does not kill themselves when casting concentration spells
						} else {
							bHasResource = (spell->GetCastingType() == RE::MagicSystem::CastingType::kConcentration) ? Magicka > 0.0f : bHasResource = Magicka >= MagickaCost;
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

	//This function is called constantly and plays a role in progression through the casting animation, progression through the charge time, and draining of actor values. 
	//We change this function to utilize a new delta (basically rate of updating) to simulate a certain charge time. As a result, casting animation, magicka drain, etc. are consistent with the new charge time.
	//By fully replacing the function with this RE'd version (rather than calling the original) we can alter other things, like the actorvalue used to cast spells, in addition to the effective charge time.
	//Big credits to shad0wshayd3 and Fenix for REing what the function does and how it can be used
	void CasterHook::Update(RE::ActorMagicCaster* caster, float delta)
	{
		//First two if statements limit when we actually alter the charge time.

		using S = RE::MagicCaster::State;
		auto state = caster->state.underlying();
		if (state != static_cast<uint32_t>(S::kNone)) {
			//logger::trace("State is {}", state);
		}
		if (caster && caster->currentSpell && caster->currentSpell->GetSpellType() != RE::MagicSystem::SpellType::kEnchantment) {
			if (state == static_cast<uint32_t>(S::kStart) || state == static_cast<uint32_t>(S::kCharging)) {
				if (auto a = caster->GetCasterAsActor(); a && caster->currentSpell && (caster->currentSpell->GetSpellType() == RE::MagicSystem::SpellType::kSpell || caster->currentSpell->GetSpellType() == RE::MagicSystem::SpellType::kStaffEnchantment || caster->currentSpell->GetSpellType() == RE::MagicSystem::SpellType::kScroll)) {
					float origin = caster->currentSpell->GetChargeTime();
					auto it = Cast::GetCastInstance(caster);
					auto currentCast = *it;
					if (!currentCast.charge) {
						currentCast.charge = new Cast::AlteredCharge();
					}
					//float newTime = CalculateNewChargeTime(caster, (origin));
					//CalculateNewChargeTime(caster, (origin));
					float newTime = currentCast.charge->updatedTime;

					float k = newTime > 0.00001f ? (origin) / newTime : 1000000.0f;

					//func1D(caster, delta * k);

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
			func1D(caster, delta);
		}
		
	}

	//This is the RE'd 'innards' of the Update function. To simplify the above function, this is moved to its own function.
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

			if (caster->state.get() == RE::MagicCaster::State::kConcentrating && bUsesWhileCasting) {
				MagickaDiff = (std::min)(MagickaDiff, MagickaPool);  
				if (MagickaDiff > 0.0f) {
					AlternateAV::DamageActorValue(caster->actor, ResourceAcV, -MagickaDiff);
				}
			} else if (caster->state.get() == RE::MagicCaster::State::kCharging && bUsesWhileCharging) {
				if (!bUsesWhileCasting) {
					float origin = caster->currentSpell->GetChargeTime();
					auto it = Cast::GetCastInstance(caster);
					auto currentCast = *it;
					if (!currentCast.charge) {
						currentCast.charge = new Cast::AlteredCharge();
					}
					float newTime = currentCast.charge->updatedTime;
					/* auto origin = caster->currentSpell->GetChargeTime();
					if ((origin - 1) > 0) {
						MagickaDiff /= (origin - 1);
					} else {
						MagickaDiff /= origin;  //this might be an issue - or maybe not?
					}
					*/
					//MagickaDiff /= CalculateNewChargeTime(caster, origin - 1);
					if (newTime > 0) {
						MagickaDiff /= newTime;
					} else {
						MagickaDiff /= origin;
					}
				}

				caster->costCharged += MagickaDiff;
				if (!bUsesWhileCasting && caster->costCharged > MagickaCost) {
					MagickaDiff -= (caster->costCharged - MagickaCost);
					caster->costCharged -= (caster->costCharged - MagickaCost);
				}

				if ((MagickaCost - caster->costCharged) <= MagickaPool) {
					AlternateAV::DamageActorValue(caster->actor, ResourceAcV, -MagickaDiff);
				} else {
					caster->InterruptCast(false);
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
			if (bhkPickData.unkC0 || !bhkPickData.rayOutput.rootCollidable) {
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
		CasterHook::funcA = ActorCasterVtbl.write_vfunc(0xA, &CasterHook::CheckCast);
		CasterHook::func1D = ActorCasterVtbl.write_vfunc(0x1D, &CasterHook::Update);

		{
			REL::Relocation<std::uintptr_t> target1{ RELOCATION_ID(33355, 34136), OFFSET(0x51, 0x51) };  //OFFSET(0x51, 0x51)
			REL::Relocation<std::uintptr_t> target2{ RELOCATION_ID(33358, 34139), OFFSET(0xB9, 0xB9) };  // OFFSET(0xB9, 0xB9)

			auto& trampoline = SKSE::GetTrampoline();
			SKSE::AllocTrampoline(14);
			trampoline.write_call<5>(target1.address(), InterruptCastHelper);
			trampoline.write_call<5>(target2.address(), InterruptCastHelper);
		}
	}
}


