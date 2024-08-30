#include "Settings/Config.h"
#include "Utility/Function.h"
#include "AlteredCast.h"
#include "ChargeTime.h"


namespace ChargeTime
{
	using namespace config;
	using namespace Cast;
	using namespace Conditions;

	//also should look into a way that perks / enchantments can alter charge time - might be best to set up a casting time actor value - reimpliment ActorValueGenerator or just make an AV using AVG

	//could add options for other items being equipped (staff, one handed weapon, etc.)

	void CalculateNewChargeTime(RE::MagicCaster* caster, float origin, bool log) {

		RE::MagicItem* spell = (caster->currentSpell);

		auto it = GetCastInstance(caster);
		auto currentCast = *it;
		if (!currentCast.charge) {
			currentCast.charge = new AlteredCharge();
		}

		auto charge = currentCast.charge;
		auto override = charge->override;
		auto excluded = charge->excluded;

		if (spell) {
			auto type = spell->GetSpellType();
			auto casting = spell->GetCastingType();
			float newTime = origin;

			if (charge->newBaseTime >= 0) {
				newTime = charge->newBaseTime;
				if (log) {
					logger::debug("Base time set to {}", newTime);
				}
			} else {
				if (log) {
					logger::debug("Base time was less than 0; setting to origin.");
				}
				newTime = origin;
			}

			

			if (excluded) {
				if (override) {
					newTime = charge->overrideValue;
					if (log) {
						logger::info("Exclusion detected and override detected; returning override value of {}", newTime);
					}
					currentCast.charge->updatedTime = newTime;
					//return newTime;
				} else {
					if (log) {
						logger::info("Exclusion detected, returning original charge time of {}", origin);  
					}
					currentCast.charge->updatedTime = origin;
					//return origin;
				}
			} else {
				if (log) {
					logger::debug("No exclusion was detected.");
				}
				if (override) {
					newTime = charge->overrideValue;
					if (log) {
						logger::debug("Override detected; base time set to {}. This may be further modified.", newTime);
					}
				}
			}

			for (auto multiplier : charge->multipliers) {
				newTime = newTime * multiplier;
				if (log) {
					logger::debug("Charge time multiplied by {}", multiplier);
				}
			}
			

			for (auto modifier : charge->modifiers) {
				newTime = newTime + modifier;
				if (log) {
					logger::debug("Charge time modified by {}", modifier);
				}
			}
			
			if (log) {
			//	logger::info("New charge time is {} seconds", newTime);
			}
			currentCast.charge->updatedTime = newTime;
			//return newTime;
		} else {
			logger::warn("Not a valid spell");
			currentCast.charge->updatedTime = origin;
			//return origin;
		}

		*it = currentCast;
	}
	
	void AddChargeModifiersOnCast(RE::MagicCaster* caster) 
	{
		logger::debug("Going through {} charge time modifiers", ctModifiers.size());
		for (Modifier* modifier : ctModifiers) {
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
				if (!currentCast.charge) {
					currentCast.charge = new AlteredCharge();
				}
				currentCast.charge->modifiers.push_back(modifier->value);
				*it = currentCast;
				logger::debug("Conditions were met; a charge time modifier of {} added", modifier->value);
			} else {
				logger::debug("Conditions were not met; no charge time modifier added");
			}
		}
		logger::debug("Going through {} charge time function modifiers", ctModifiersF.size());
		for (ModifierF* modifier : ctModifiersF) {
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
				if (!currentCast.charge) {
					currentCast.charge = new AlteredCharge();
				}
				currentCast.charge->modifiers.push_back(mult);
				*it = currentCast;
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
				if (!currentCast.charge) {
					currentCast.charge = new AlteredCharge();
				}
				currentCast.charge->multipliers.push_back(multiplier->value);
				*it = currentCast;
				logger::debug("Conditions were met; a charge time multiplier of {} added", multiplier->value);
			} else {
				logger::debug("Conditions were not met; no charge time multiplier added");
			}
		}
		logger::debug("Going through {} charge time function multipliers", ctMultipliersF.size());
		for (MultiplierF* multiplier : ctMultipliersF) {
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
				if (!currentCast.charge) {
					currentCast.charge = new AlteredCharge();
				}
				currentCast.charge->multipliers.push_back(mult);
				*it = currentCast;
				logger::debug("Conditions were met; a charge time function multiplier of {} added", mult);
			} else {
				logger::debug("Conditions  were not met; no charge time function multiplier added");
			}
		}
		
	}

	void AddChargeOverridesOnCast(RE::MagicCaster* caster) {
		logger::debug("Going through {} charge time overrides", ctOverrides.size());
		for (auto override : ctOverrides) {
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
				if (!currentCast.charge) {
					currentCast.charge = new AlteredCharge();
				}
				currentCast.charge->overrideValue = override->value;
				currentCast.charge->override = override->override;
				currentCast.charge->excluded = override->excluded;
				*it = currentCast;
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
