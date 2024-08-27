#include "Settings/Config.h"
#include "Utility/Function.h"
#include "Magnitude.h"

namespace Magnitude
{
	using namespace config;
	using namespace Cast;
	using namespace Conditions;

	float CalculateNewMag(float origin, AlteredMagnitude* magnitude)
	{
		float newMag;
		
		auto override = magnitude->override;
		auto excluded = magnitude->excluded;
		
		if (magnitude->newBaseMag >= 0) {
			newMag = magnitude->newBaseMag;

			logger::debug("Base magnitude set to {}", newMag);

		} else {

			logger::debug("Base magnitude was less than 0; setting to origin ({}).", origin);
			
			newMag = origin;
		}

		if (excluded) {
			if (override) {
				newMag = magnitude->overrideValue;

				logger::info("Exclusion detected and override detected; returning override value of {}", newMag);

				return newMag;
			} else {

				logger::info("Exclusion detected, returning original magnitude of {}", origin);  //this is giving a weird value

				return origin;
			}
		} else {

			logger::debug("No exclusion was detected.");

			if (override) {
				newMag = magnitude->overrideValue;

				logger::debug("Override detected; base magnitude set to {}. This may be further modified.", newMag);

			}
		}

		for (auto multiplier : magnitude->multipliers) {
			newMag = newMag * multiplier;

			logger::debug("magnitude multiplied by {}", multiplier);
			
		}

		for (auto modifier : magnitude->modifiers) {
			newMag = newMag + modifier;

			logger::debug("magnitude modified by {}", modifier);
		}

		logger::debug("New magnitude is {}", newMag);

		return newMag;

	}

	void AddMagModifiersOnCast(RE::MagicCaster* caster)
	{
		auto spell = caster->currentSpell;

		logger::debug("Going through {} modifiers", mModifiers.size());
		for (Modifier* modifier : mModifiers) {
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
				if (!currentCast.magnitude) {
					currentCast.magnitude = new AlteredMagnitude();
				}
				currentCast.magnitude->modifiers.push_back(modifier->value);
				*it = currentCast;
				logger::debug("Conditions were met; a modifier of {} added", modifier->value);
			} else {
				logger::debug("Conditions were not met; no modifier added");
			}
		}
		logger::debug("Going through {} function modifiers", mModifiersF.size());
		for (ModifierF* modifier : mModifiersF) {
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
				auto mult = function::evaluateExpression(modifier->function.function, function::AssignVariables(modifier->function.variables, caster));

				auto it = GetCastInstance(caster);
				auto currentCast = *it;
				if (!currentCast.magnitude) {
					currentCast.magnitude = new AlteredMagnitude();
				}
				currentCast.magnitude->modifiers.push_back(mult);
				*it = currentCast;
				logger::debug("Conditions were met; a function modifier of {} added", mult);
			} else {
				logger::debug("Conditions were not met; no function modifier added");
			}
		}
	}

	void AddMagMultipliersOnCast(RE::MagicCaster* caster)
	{
		auto spell = caster->currentSpell;

		logger::debug("Going through {} multipliers", mMultipliers.size());
		for (Multiplier* multiplier : mMultipliers) {
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
				if (!currentCast.magnitude) {
					currentCast.magnitude = new AlteredMagnitude();
				}
				currentCast.magnitude->multipliers.push_back(multiplier->value);
				*it = currentCast;
				logger::debug("Conditions were met; a multiplier of {} added", multiplier->value);
			} else {
				logger::debug("Conditions were not met; no multiplier added");
			}
		}
		logger::debug("Going through {} function multipliers", mMultipliersF.size());
		for (MultiplierF* multiplier : mMultipliersF) {
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
				if (!currentCast.magnitude) {
					currentCast.magnitude = new AlteredMagnitude();
				}
				currentCast.magnitude->multipliers.push_back(mult);
				*it = currentCast;
				logger::debug("Conditions were met; a function multiplier of {} added", mult);
			} else {
				logger::debug("Conditions  were not met; no function multiplier added");
			}
		}
	}

	void AddMagOverridesOnCast(RE::MagicCaster* caster)
	{
		auto spell = caster->currentSpell;

		logger::debug("Going through {} overrides", mOverrides.size());
		for (auto override : mOverrides) {
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
				if (!currentCast.magnitude) {
					currentCast.magnitude = new AlteredMagnitude();
				}
				currentCast.magnitude->overrideValue = override->value;
				currentCast.magnitude->override = override->override;
				currentCast.magnitude->excluded = override->excluded;
				*it = currentCast;
				if (currentCast.magnitude->override) {
					if (currentCast.magnitude->excluded) {
						logger::debug("Conditions were met; override of {} set and exclusion set to {}", currentCast.magnitude->overrideValue, currentCast.magnitude->excluded);
					} else {
						logger::debug("Conditions were met; override of {} set.", currentCast.magnitude->overrideValue);
					}
				} else {
					if (currentCast.magnitude->excluded) {
						logger::debug("Conditions were met; Exclusion set to {}", currentCast.magnitude->excluded);
					}
				}
			} else {
				logger::debug("Conditions were not met; no override set");
			}
		}
	}
}
