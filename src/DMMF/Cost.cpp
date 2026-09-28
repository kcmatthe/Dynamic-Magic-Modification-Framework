#include "Settings/Config.h"
#include "Settings/Conditions.h"
#include "Utility/Function.h"
#include "AlteredCast.h"
#include "Cost.h"
#include "PluginData.h"

namespace Cost
{
	using namespace config;
	using namespace Cast;
	using namespace Conditions;
	using namespace PluginData;

	void CalculateNewCost(RE::MagicCaster* caster, bool log)
	{
		RE::MagicItem* spell = caster->currentSpell;
		float origin = caster->currentSpellCost;
		float newCost = origin;

		auto& currentCast = Cast::GetCastInstance(caster);

		if (!currentCast.cost) {
			currentCast.cost = std::make_unique<AlteredCost>();
		}

		auto* cost = currentCast.cost.get();
		bool override = cost->override;
		bool excluded = cost->excluded;

		if (spell) {
			auto type = spell->GetSpellType();
			auto casting = spell->GetCastingType();


			if (cost->newBaseCost >= 0) {
				newCost = cost->newBaseCost;
				if (log) {
					logger::debug("Base cost set to {}", newCost);
				}
			} else {
				if (log) {
					logger::debug("Base cost was less than 0; setting to origin.");
				}
				newCost = origin;
			}

			if (excluded) {
				if (override) {
					newCost = cost->overrideValue;
					if (log) {
						logger::info("Exclusion detected and override detected; returning override value of {}", newCost);
					}
					//return newCost;
					currentCast.cost->updatedCost = newCost;
				} else {
					if (log) {
						logger::info("Exclusion detected, returning original cost of {}", origin);  //this is giving a weird value
					}
					//return origin;
					currentCast.cost->updatedCost = origin;
				}
			} else {
				if (log) {
					logger::debug("No exclusion was detected.");
				}
				if (override) {
					newCost = cost->overrideValue;
					if (log) {
						logger::debug("Override detected; base cost set to {}. This may be further modified.", newCost);
					}
				}
			}

			for (auto multiplier : cost->multipliers) {
				newCost = newCost * multiplier;
				if (log) {
					logger::debug("Cost multiplied by {}", multiplier);
				}
			}

			for (auto modifier : cost->modifiers) {
				newCost = newCost + modifier;
				if (log) {
					logger::debug("Cost modified by {}", modifier);
				}
			}

			if (log) {
			//	logger::info("New cost is {}", newCost);
			}
			//return newCost;
			currentCast.cost->updatedCost = newCost;
		} else {
			logger::warn("Not a valid spell");
			//return origin;
			currentCast.cost->updatedCost = origin;
		}
	}

	void AddCostModifiersOnCast(RE::MagicCaster* caster)
	{
		logger::debug("Going through {} cost modifiers", cModifiers.size());
		for (Modifier* modifier : cModifiers) {
			const bool conditionsMet = Conditions::EvaluateConditionsList(modifier, caster);

			if (conditionsMet) {
				auto& currentCast = Cast::GetCastInstance(caster);

				if (!currentCast.cost) {
					currentCast.cost = std::make_unique<AlteredCost>();
				}
				currentCast.cost->modifiers.push_back(modifier->value);

				logger::debug("Conditions were met; a cost modifier of {} added", modifier->value);
			} else {
				logger::debug("Conditions were not met; no cost modifier added");
			}
		}
		logger::debug("Going through {} cost function modifiers", cModifiersF.size());
		for (ModifierF* modifier : cModifiersF) {
			const bool conditionsMet = Conditions::EvaluateConditionsList(modifier, caster);

			if (conditionsMet) {
				auto mod = function::evaluateExpression(modifier->function.function, function::AssignVariables(modifier->function.variables, caster));

				auto& currentCast = Cast::GetCastInstance(caster);

				if (!currentCast.cost) {
					currentCast.cost = std::make_unique<AlteredCost>();
				}
				currentCast.cost->modifiers.push_back(mod);

				logger::debug("Conditions were met; a cost function modifier of {} added", mod);
			} else {
				logger::debug("Conditions were not met; no cost function modifier added");
			}
		}
	}

	void EvaluateMultMagicEffects(RE::MagicCaster* caster)
	{
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

		if (!currentCast.cost) {
			currentCast.cost = std::make_unique<AlteredCost>();
		}

		for (auto& activeEffect : activeEffects) {
			if (!activeEffect) {
				continue;
			}

			if (activeEffect->effect->baseEffect->HasKeyword(PositiveCostMultKYWD)) {
				auto effectMult = 1 - (activeEffect->magnitude / 100);
				currentCast.cost->multipliers.push_back(effectMult);
				logger::debug("CostMult effect active; pushing back mult of {}", effectMult);
			}
			if (activeEffect->effect->baseEffect->HasKeyword(NegativeCostMultKYWD)) {
				auto effectMult = 1 + (activeEffect->magnitude / 100);
				currentCast.cost->multipliers.push_back(effectMult);
				logger::debug("CostMult effect active; pushing back mult of {}", effectMult);
			}
		}
	}

	void AddCostMultipliersOnCast(RE::MagicCaster* caster)
	{
		auto spell = caster->currentSpell;

		logger::debug("Going through {} cost multipliers", cMultipliers.size());
		for (Multiplier* multiplier : cMultipliers) {
			const bool conditionsMet = Conditions::EvaluateConditionsList(multiplier, caster);

			if (conditionsMet) {
				auto& currentCast = Cast::GetCastInstance(caster);

				if (!currentCast.cost) {
					currentCast.cost = std::make_unique<AlteredCost>();
				}
				currentCast.cost->multipliers.push_back(multiplier->value);

				logger::debug("Conditions were met; a cost multiplier of {} added", multiplier->value);
			} else {
				logger::debug("Conditions were not met; no cost multiplier added");
			}
		}
		logger::debug("Going through {} cost function multipliers", cMultipliersF.size());
		for (MultiplierF* multiplier : cMultipliersF) {
			const bool conditionsMet = Conditions::EvaluateConditionsList(multiplier, caster);

			if (conditionsMet) {
				auto mult = function::evaluateExpression(multiplier->function.function, function::AssignVariables(multiplier->function.variables, caster));

				auto& currentCast = Cast::GetCastInstance(caster);

				if (!currentCast.cost) {
					currentCast.cost = std::make_unique<AlteredCost>();
				}
				currentCast.cost->multipliers.push_back(mult);

				logger::debug("Conditions were met; a cost function multiplier of {} added", mult);
			} else {
				logger::debug("Conditions  were not met; no cost function multiplier added");
			}
		}

		EvaluateMultMagicEffects(caster);
	}

	void AddCostOverridesOnCast(RE::MagicCaster* caster)
	{
		logger::debug("Going through {} cost overrides", cOverrides.size());
		for (auto override : cOverrides) {
			const bool conditionsMet = Conditions::EvaluateConditionsList(override, caster);

			if (conditionsMet) {
				auto& currentCast = Cast::GetCastInstance(caster);

				if (!currentCast.cost) {
					currentCast.cost = std::make_unique<AlteredCost>();
				}
				currentCast.cost->overrideValue = override->value;
				currentCast.cost->override = override->override;
				currentCast.cost->excluded = override->excluded;

				if (currentCast.cost->override) {
					if (currentCast.cost->excluded) {
						logger::debug("Conditions were met; cost override of {} set and exclusion set to {}", currentCast.cost->overrideValue, currentCast.cost->excluded);
					} else {
						logger::debug("Conditions were met; cost override of {} set.", currentCast.cost->overrideValue);
					}
				} else {
					if (currentCast.cost->excluded) {
						logger::debug("Conditions were met; Cos exclusion set to {}", currentCast.cost->excluded);
					}
				}
			} else {
				logger::debug("Conditions were not met; no cost override set");
			}
		}
	}

	

}
