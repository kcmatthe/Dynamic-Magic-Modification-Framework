#include "Settings/Config.h"
#include "Settings/Conditions.h"
#include "Utility/Function.h"
#include "Utility/Utility.h"
#include "AlteredCast.h"
#include "ChargeTime.h"
#include "PluginData.h"


namespace ChargeTime
{
	using namespace config;
	using namespace Cast;
	using namespace Conditions;
	using namespace PluginData;
	

	void EvaluateMultMagicEffects(RE::MagicCaster* caster) {
		if (!caster) {
			return;
		}
		auto spell = caster->currentSpell;
		if (!spell) {
			return;
		}

		auto actor = caster->GetCasterAsActor();
		
		if (!actor) {
			return;
		}

		auto& activeEffects = *actor->AsMagicTarget()->GetActiveEffectList();

		if (!&activeEffects) {
			return;
		}
		auto& currentCast = Cast::GetCastInstance(caster);

		if (!currentCast.charge) {
			currentCast.charge = std::make_unique<AlteredCharge>();
		}

		for (auto& activeEffect : activeEffects) {
			if (!activeEffect) {
				continue;
			}

			if (activeEffect->effect->baseEffect->HasKeyword(PositiveCastTimeMultKYWD)) {
				auto effectMult = 1 - (activeEffect->magnitude / 100);
				currentCast.charge->multipliers.push_back(effectMult);
				logger::debug("CastingTimeMult effect active; pushing back mult of {}", effectMult);
			}
			if (activeEffect->effect->baseEffect->HasKeyword(NegativeCastTimeMultKYWD)) {
				auto effectMult = 1 + (activeEffect->magnitude / 100);
				currentCast.charge->multipliers.push_back(effectMult);
				logger::debug("CastingTimeMult effect active; pushing back mult of {}", effectMult);
			}

			/* if (activeEffect->effect->baseEffect == castingTimeMult) {
				auto effectMult = 1 - (activeEffect->magnitude / 100);
				currentCast.charge->multipliers.push_back(effectMult);
				logger::debug("CastingTimeMult effect active; pushing back mult of {}", effectMult);
			}
			if (activeEffect->effect->baseEffect == castingTimeMultConst) {
				auto effectMult = 1 - (activeEffect->magnitude / 100);
				currentCast.charge->multipliers.push_back(effectMult);
				logger::debug("CastingTimeMultConst effect active; pushing back mult of {}", effectMult);
			}*/
		}

	}

	void CalculateNewChargeTime(RE::MagicCaster* caster, float origin) {

		RE::MagicItem* spell = (caster->currentSpell);

		auto& currentCast = Cast::GetCastInstance(caster);

		if (!currentCast.charge) {
			currentCast.charge = std::make_unique<AlteredCharge>();
		}

		auto charge = currentCast.charge.get();
		auto override = charge->override;
		auto excluded = charge->excluded;

		if (!spell) {
			logger::warn("Not a valid spell");
			currentCast.charge->updatedTime = origin;
		}

		auto type = spell->GetSpellType();
		auto casting = spell->GetCastingType();
		float newTime = origin;

		if (charge->newBaseTime >= 0) {
			newTime = charge->newBaseTime;
			logger::debug("Base time set to {}", newTime);
		} else {
			logger::debug("Base time was less than 0; setting to origin.");
			newTime = origin;
		}

		if (excluded) {
			if (override) {
				newTime = charge->overrideValue;
				logger::debug("Exclusion detected and override detected; returning override value of {}", newTime);
				currentCast.charge->updatedTime = newTime;
			} else {
				logger::debug("Exclusion detected, returning original charge time of {}", origin);  
				currentCast.charge->updatedTime = origin;
			}
		} else {
			logger::debug("No exclusion was detected.");
			if (override) {
				newTime = charge->overrideValue;
				logger::debug("Override detected; base time set to {}. This may be further modified.", newTime);
			}
		}

		for (auto multiplier : charge->multipliers) {
			newTime = newTime * multiplier;
			logger::debug("Charge time multiplied by {}", multiplier);
		}
			
		for (auto modifier : charge->modifiers) {
			newTime = newTime + modifier;
			logger::debug("Charge time modified by {}", modifier);
		}
			
		currentCast.charge->updatedTime = newTime;
	}
	
	void AddChargeModifiersOnCast(RE::MagicCaster* caster) 
	{
		logger::debug("Going through {} charge time modifiers", ctModifiers.size());
		for (Modifier* modifier : ctModifiers) {
			
			const bool conditionsMet = Conditions::EvaluateConditionsList(modifier, caster);

			if (conditionsMet) {
				auto& currentCast = Cast::GetCastInstance(caster);

				if (!currentCast.charge) {
					currentCast.charge = std::make_unique<AlteredCharge>();
				}
				currentCast.charge->modifiers.push_back(modifier->value);

				logger::debug("Conditions were met; a charge time modifier of {} added", modifier->value);
			} else {
				logger::debug("Conditions were not met; no charge time modifier added");
			}
		}
		logger::debug("Going through {} charge time function modifiers", ctModifiersF.size());
		for (ModifierF* modifier : ctModifiersF) {
			
			const bool conditionsMet = Conditions::EvaluateConditionsList(modifier, caster);

			if (conditionsMet) {
				auto mult = function::evaluateExpression(modifier->function.function, function::AssignVariables(modifier->function.variables, caster));

				auto& currentCast = Cast::GetCastInstance(caster);

				if (!currentCast.charge) {
					currentCast.charge = std::make_unique<AlteredCharge>();
				}
				currentCast.charge->modifiers.push_back(mult);

				logger::debug("Conditions were met; a charge time function modifier of {} added", mult);
			} else {
				logger::debug("Conditions were not met; no charge time function modifier added");
			}
		}
		
	}

	void AddChargeMultipliersOnCast(RE::MagicCaster* caster)
	{
		logger::debug("Going through {} charge time multipliers", ctMultipliers.size());
		for (Multiplier* multiplier : ctMultipliers) {
			
			const bool conditionsMet = Conditions::EvaluateConditionsList(multiplier, caster);

			if (conditionsMet) {
				auto& currentCast = Cast::GetCastInstance(caster);

				if (!currentCast.charge) {
					currentCast.charge = std::make_unique<AlteredCharge>();
				}
				currentCast.charge->multipliers.push_back(multiplier->value);

				logger::debug("Conditions were met; a charge time multiplier of {} added", multiplier->value);
			} else {
				logger::debug("Conditions were not met; no charge time multiplier added");
			}
		}
		logger::debug("Going through {} charge time function multipliers", ctMultipliersF.size());
		for (MultiplierF* multiplier : ctMultipliersF) {
			
			const bool conditionsMet = Conditions::EvaluateConditionsList(multiplier, caster);

			if (conditionsMet) {
				auto mult = function::evaluateExpression(multiplier->function.function, function::AssignVariables(multiplier->function.variables, caster));

				auto& currentCast = Cast::GetCastInstance(caster);

				if (!currentCast.charge) {
					currentCast.charge = std::make_unique<AlteredCharge>();
				}
				currentCast.charge->multipliers.push_back(mult);

				logger::debug("Conditions were met; a charge time function multiplier of {} added", mult);
			} else {
				logger::debug("Conditions  were not met; no charge time function multiplier added");
			}
		}
		EvaluateMultMagicEffects(caster);

	}

	void AddChargeOverridesOnCast(RE::MagicCaster* caster) {
		logger::debug("Going through {} charge time overrides", ctOverrides.size());
		for (auto override : ctOverrides) {
			
			const bool conditionsMet = Conditions::EvaluateConditionsList(override, caster);

			if (conditionsMet) {
				auto& currentCast = Cast::GetCastInstance(caster);

				if (!currentCast.charge) {
					currentCast.charge = std::make_unique<AlteredCharge>();
				}
				currentCast.charge->overrideValue = override->value;
				currentCast.charge->override = override->override;
				currentCast.charge->excluded = override->excluded;

				if (currentCast.charge->override){
					if (currentCast.charge->excluded) {
						logger::debug("Conditions were met; charge time override of {} seconds set and exclusion set to {}", currentCast.charge->overrideValue, currentCast.charge->excluded);
					} else {
						logger::debug("Conditions were met; charge time override of {} seconds set.", currentCast.charge->overrideValue);
					}
				} else {
					if (currentCast.charge->excluded) {
						logger::debug("Conditions were met; Charge time exclusion set to {}", currentCast.charge->excluded);
					}
				}
			} else {
				logger::debug("Conditions were not met; no charge time override set");
			}
		}
		
	}

	
}
