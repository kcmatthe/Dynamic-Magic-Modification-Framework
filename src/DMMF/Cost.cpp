#include "Settings/Config.h"
#include "Utility/Function.h"
#include "AlteredCast.h"
#include "Cost.h"

namespace Cost
{
	using namespace config;
	using namespace Cast;
	using namespace Conditions;

	void CalculateNewCost(RE::MagicCaster* caster, bool log)
	{
		RE::MagicItem* spell = caster->currentSpell;
		float origin = caster->currentSpellCost;
		float newCost = origin;

		
		auto it = GetCastInstance(caster);
		auto currentCast = *it;
		if (!currentCast.cost) {
			currentCast.cost = new AlteredCost();
		}
		auto cost = currentCast.cost;
		auto override = cost->override;
		auto excluded = cost->excluded;

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
		*it = currentCast;
	}

	void AddCostModifiersOnCast(RE::MagicCaster* caster)
	{
		auto spell = caster->currentSpell;

		logger::debug("Going through {} cost modifiers", cModifiers.size());
		for (Modifier* modifier : cModifiers) {
			std::vector<bool> bools = {};
			bool conditionsMet = false;
			for (Condition condition : modifier->conditions) {
				auto tempBool = CheckCondition(condition, caster);
				logger::debug("Individual condition check was {}", tempBool);
				bools.push_back(tempBool);
			}
			if (modifier->condOp == "or") {
				logger::debug("The comparative operator was 'or'");
				auto boolIt = std::find(bools.begin(), bools.end(), true);

				if (boolIt != bools.end()) {
					conditionsMet = true;
				} else {
					conditionsMet = false;
				}
			}
			if (modifier->condOp == "and") {
				logger::debug("The comparative operator was 'and'");
				auto boolIt = std::find(bools.begin(), bools.end(), false);

				if (boolIt != bools.end()) {
					conditionsMet = false;
				} else {
					conditionsMet = true;
				}
			}
			if (conditionsMet) {
				auto it = GetCastInstance(caster);
				auto currentCast = *it;
				if (!currentCast.cost) {
					currentCast.cost = new AlteredCost();
				}
				currentCast.cost->modifiers.push_back(modifier->value);
				*it = currentCast;
				logger::debug("Conditions were met; a cost modifier of {} added", modifier->value);
			} else {
				logger::debug("Conditions were not met; no cost modifier added");
			}
		}
		logger::debug("Going through {} cost function modifiers", cModifiersF.size());
		for (ModifierF* modifier : cModifiersF) {
			std::vector<bool> bools = {};
			bool conditionsMet = false;
			for (Condition condition : modifier->conditions) {
				auto tempBool = CheckCondition(condition, caster);
				logger::debug("Individual condition check was {}", tempBool);
				bools.push_back(tempBool);
			}
			if (modifier->condOp == "or") {
				logger::debug("The comparative operator was 'or'");
				auto boolIt = std::find(bools.begin(), bools.end(), true);

				if (boolIt != bools.end()) {
					conditionsMet = true;
				} else {
					conditionsMet = false;
				}
			}
			if (modifier->condOp == "and") {
				logger::debug("The comparative operator was 'and'");
				auto boolIt = std::find(bools.begin(), bools.end(), false);

				if (boolIt != bools.end()) {
					conditionsMet = false;
				} else {
					conditionsMet = true;
				}
			}
			if (conditionsMet) {
				auto mod = function::evaluateExpression(modifier->function.function, function::AssignVariables(modifier->function.variables, caster));

				auto it = GetCastInstance(caster);
				auto currentCast = *it;
				if (!currentCast.cost) {
					currentCast.cost = new AlteredCost();
				}
				currentCast.cost->modifiers.push_back(mod);
				*it = currentCast;
				logger::debug("Conditions were met; a cost function modifier of {} added", mod);
			} else {
				logger::debug("Conditions were not met; no cost function modifier added");
			}
		}
	}

	void AddCostMultipliersOnCast(RE::MagicCaster* caster)
	{
		auto spell = caster->currentSpell;

		logger::debug("Going through {} cost multipliers", cMultipliers.size());
		for (Multiplier* multiplier : cMultipliers) {
			std::vector<bool> bools = {};
			bool conditionsMet = false;
			for (Condition condition : multiplier->conditions) {
				auto tempBool = CheckCondition(condition, caster);
				logger::debug("Individual condition check was {}", tempBool);
				bools.push_back(tempBool);
			}
			if (multiplier->condOp == "or") {
				logger::debug("The comparative operator was 'or'");
				auto boolIt = std::find(bools.begin(), bools.end(), true);

				if (boolIt != bools.end()) {
					conditionsMet = true;
				} else {
					conditionsMet = false;
				}
			}
			if (multiplier->condOp == "and") {
				logger::debug("The comparative operator was 'and'");
				auto boolIt = std::find(bools.begin(), bools.end(), false);

				if (boolIt != bools.end()) {
					conditionsMet = false;
				} else {
					conditionsMet = true;
				}
			}
			if (conditionsMet) {
				auto it = GetCastInstance(caster);
				auto currentCast = *it;
				if (!currentCast.cost) {
					currentCast.cost = new AlteredCost();
				}
				currentCast.cost->multipliers.push_back(multiplier->value);
				*it = currentCast;
				logger::debug("Conditions were met; a cost multiplier of {} added", multiplier->value);
			} else {
				logger::debug("Conditions were not met; no cost multiplier added");
			}
		}
		logger::debug("Going through {} cost function multipliers", cMultipliersF.size());
		for (MultiplierF* multiplier : cMultipliersF) {
			std::vector<bool> bools = {};
			bool conditionsMet = false;
			for (Condition condition : multiplier->conditions) {
				auto tempBool = CheckCondition(condition, caster);
				logger::debug("Individual condition check was {}", tempBool);
				bools.push_back(tempBool);
			}
			if (multiplier->condOp == "or") {
				logger::debug("The comparative operator was 'or'");
				auto boolIt = std::find(bools.begin(), bools.end(), true);

				if (boolIt != bools.end()) {
					conditionsMet = true;
				} else {
					conditionsMet = false;
				}
			}
			if (multiplier->condOp == "and") {
				logger::debug("The comparative operator was 'and'");
				auto boolIt = std::find(bools.begin(), bools.end(), false);

				if (boolIt != bools.end()) {
					conditionsMet = false;
				} else {
					conditionsMet = true;
				}
			}
			if (conditionsMet) {
				auto mult = function::evaluateExpression(multiplier->function.function, function::AssignVariables(multiplier->function.variables, caster));

				auto it = GetCastInstance(caster);
				auto currentCast = *it;
				if (!currentCast.cost) {
					currentCast.cost = new AlteredCost();
				}
				currentCast.cost->multipliers.push_back(mult);
				*it = currentCast;
				logger::debug("Conditions were met; a cost function multiplier of {} added", mult);
			} else {
				logger::debug("Conditions  were not met; no cost function multiplier added");
			}
		}
	}

	void AddCostOverridesOnCast(RE::MagicCaster* caster)
	{
		auto spell = caster->currentSpell;

		logger::debug("Going through {} cost overrides", cOverrides.size());
		for (auto override : cOverrides) {
			std::vector<bool> bools = {};
			bool conditionsMet = false;
			for (Condition condition : override->conditions) {
				auto tempBool = CheckCondition(condition, caster);
				logger::debug("Individual condition check was {}", tempBool);
				bools.push_back(tempBool);
			}
			if (override->condOp == "or") {
				logger::debug("The comparative operator was 'or'");
				auto boolIt = std::find(bools.begin(), bools.end(), true);

				if (boolIt != bools.end()) {
					conditionsMet = true;
				} else {
					conditionsMet = false;
				}
			}
			if (override->condOp == "and") {
				logger::debug("The comparative operator was 'and'");
				auto boolIt = std::find(bools.begin(), bools.end(), false);

				if (boolIt != bools.end()) {
					conditionsMet = false;
				} else {
					conditionsMet = true;
				}
			}
			if (conditionsMet) {
				auto it = GetCastInstance(caster);
				auto currentCast = *it;
				if (!currentCast.cost) {
					currentCast.cost = new AlteredCost();
				}
				currentCast.cost->overrideValue = override->value;
				currentCast.cost->override = override->override;
				currentCast.cost->excluded = override->excluded;
				*it = currentCast;
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
