#include "Settings/Config.h"
#include "Utility/Function.h"
#include "Magnitude.h"
#include "Settings/Conditions.h"

namespace Magnitude
{
	using namespace config;
	using namespace Cast;
	using namespace Conditions;

	//Magnitude

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
			magnitude->newBaseMag = origin;
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
		magnitude->updatedMag = newMag;

		return newMag;

	}

	void AddMagModifiersOnCast(RE::MagicCaster* caster)
	{
		logger::debug("Going through {} mag modifiers", mModifiers.size());
		for (Modifier* modifier : mModifiers) {
			const bool conditionsMet = Conditions::EvaluateConditionsList(modifier, caster);

			if (conditionsMet) {
				auto& currentCast = Cast::GetCastInstance(caster);

				if (!currentCast.magnitude) {
					currentCast.magnitude = std::make_unique<AlteredMagnitude>();
				}

				currentCast.magnitude->modifiers.push_back(modifier->value);
				logger::debug("Conditions were met; a mag modifier of {} added", modifier->value);
			} else {
				logger::debug("Conditions were not met; no mag modifier added");
			}
		}
		logger::debug("Going through {} mag function modifiers", mModifiersF.size());
		for (ModifierF* modifier : mModifiersF) {
			const bool conditionsMet = Conditions::EvaluateConditionsList(modifier, caster);

			if (conditionsMet) {
				auto mult = function::evaluateExpression(modifier->function.function, function::AssignVariables(modifier->function.variables, caster));

				auto& currentCast = Cast::GetCastInstance(caster);

				if (!currentCast.magnitude) {
					currentCast.magnitude = std::make_unique<AlteredMagnitude>();
				}

				currentCast.magnitude->modifiers.push_back(mult);
				logger::debug("Conditions were met; a mag function modifier of {} added", mult);
			} else {
				logger::debug("Conditions were not met; no mag function modifier added");
			}
		}
	}

	void AddMagMultipliersOnCast(RE::MagicCaster* caster)
	{
		logger::debug("Going through {} mag multipliers", mMultipliers.size());
		for (Multiplier* multiplier : mMultipliers) {
			const bool conditionsMet = Conditions::EvaluateConditionsList(multiplier, caster);

			if (conditionsMet) {
				auto& currentCast = Cast::GetCastInstance(caster);

				if (!currentCast.magnitude) {
					currentCast.magnitude = std::make_unique<AlteredMagnitude>();
				}

				currentCast.magnitude->multipliers.push_back(multiplier->value);
				logger::debug("Conditions were met; a mag multiplier of {} added", multiplier->value);
			} else {
				logger::debug("Conditions were not met; no mag multiplier added");
			}
		}

		logger::debug("Going through {} mag function multipliers", mMultipliersF.size());
		for (MultiplierF* multiplier : mMultipliersF) {

			const bool conditionsMet = Conditions::EvaluateConditionsList(multiplier, caster);

			if (conditionsMet) {
				auto mult = function::evaluateExpression(multiplier->function.function, function::AssignVariables(multiplier->function.variables, caster));

				auto& currentCast = Cast::GetCastInstance(caster);

				if (!currentCast.magnitude) {
					currentCast.magnitude = std::make_unique<AlteredMagnitude>();
				}

				currentCast.magnitude->multipliers.push_back(mult);
				logger::debug("Conditions were met; a mag function multiplier of {} added", mult);
			} else {
				logger::debug("Conditions  were not met; no mag function multiplier added");
			}
		}
	}

	void AddMagOverridesOnCast(RE::MagicCaster* caster)
	{
		logger::debug("Going through {} mag overrides", mOverrides.size());
		for (auto override : mOverrides) {
			const bool conditionsMet = Conditions::EvaluateConditionsList(override, caster);

			if (conditionsMet) {
				auto& currentCast = Cast::GetCastInstance(caster);

				if (!currentCast.magnitude) {
					currentCast.magnitude = std::make_unique<AlteredMagnitude>();
				}

				currentCast.magnitude->overrideValue = override->value;
				currentCast.magnitude->override = override->override;
				currentCast.magnitude->excluded = override->excluded;

				if (currentCast.magnitude->override) {
					if (currentCast.magnitude->excluded) {
						logger::debug("Conditions were met; mag override of {} set and exclusion set to {}", currentCast.magnitude->overrideValue, currentCast.magnitude->excluded);
					} else {
						logger::debug("Conditions were met; mag override of {} set.", currentCast.magnitude->overrideValue);
					}
				} else {
					if (currentCast.magnitude->excluded) {
						logger::debug("Conditions were met; Exclusion set to {}", currentCast.magnitude->excluded);
					}
				}
			} else {
				logger::debug("Conditions were not met; no mag override set");
			}
		}
	}

	//Duration

	float CalculateNewDur(float origin, AlteredDuration* duration)
	{
		float newDur;

		auto override = duration->override;
		auto excluded = duration->excluded;

		if (duration->newBaseDur >= 0) {
			newDur = duration->newBaseDur;
			logger::debug("Base duration set to {}", newDur);
		} else {
			logger::debug("Base duration was less than 0; setting to origin ({}).", origin);
			newDur = origin;
			duration->newBaseDur = origin;
		}

		if (excluded) {
			if (override) {
				newDur = duration->overrideValue;
				logger::info("Exclusion detected and override detected; returning override value of {}", newDur);
				return newDur;
			} else {
				logger::info("Exclusion detected, returning original duration of {}", origin);  //this is giving a weird value
				return origin;
			}
		} else {
			logger::debug("No exclusion was detected.");
			if (override) {
				newDur = duration->overrideValue;
				logger::debug("Override detected; base duration set to {}. This may be further modified.", newDur);
			}
		}

		for (auto multiplier : duration->multipliers) {
			newDur = newDur * multiplier;
			logger::debug("duration multiplied by {}", multiplier);
		}

		for (auto modifier : duration->modifiers) {
			newDur = newDur + modifier;
			logger::debug("duration modified by {}", modifier);
		}

		logger::debug("New duration is {}", newDur);
		duration->updatedDur = newDur;

		return newDur;
	}

	void AddDurModifiersOnCast(RE::MagicCaster* caster)
	{
		logger::debug("Going through {} duration modifiers", dModifiers.size());
		for (Modifier* modifier : dModifiers) {
			const bool conditionsMet = Conditions::EvaluateConditionsList(modifier, caster);

			if (conditionsMet) {
				auto& currentCast = Cast::GetCastInstance(caster);

				if (!currentCast.duration) {
					currentCast.duration = std::make_unique<AlteredDuration>();
				}

				currentCast.duration->modifiers.push_back(modifier->value);
				logger::debug("Conditions were met; a duration modifier of {} added", modifier->value);
			} else {
				logger::debug("Conditions were not met; no duration modifier added");
			}
		}
		logger::debug("Going through {} duration function modifiers", dModifiersF.size());
		for (ModifierF* modifier : dModifiersF) {
			const bool conditionsMet = Conditions::EvaluateConditionsList(modifier, caster);

			if (conditionsMet) {
				auto mult = function::evaluateExpression(modifier->function.function, function::AssignVariables(modifier->function.variables, caster));

				auto& currentCast = Cast::GetCastInstance(caster);

				if (!currentCast.duration) {
					currentCast.duration = std::make_unique<AlteredDuration>();
				}

				currentCast.duration->modifiers.push_back(mult);
				logger::debug("Conditions were met; a duration function modifier of {} added", mult);
			} else {
				logger::debug("Conditions were not met; no duration function modifier added");
			}
		}
	}

	void AddDurMultipliersOnCast(RE::MagicCaster* caster)
	{
		logger::debug("Going through {} duration multipliers", dMultipliers.size());
		for (Multiplier* multiplier : dMultipliers) {
			const bool conditionsMet = Conditions::EvaluateConditionsList(multiplier, caster);

			if (conditionsMet) {
				auto& currentCast = Cast::GetCastInstance(caster);

				if (!currentCast.duration) {
					currentCast.duration = std::make_unique<AlteredDuration>();
				}

				currentCast.duration->multipliers.push_back(multiplier->value);
				logger::debug("Conditions were met; a duration multiplier of {} added", multiplier->value);
			} else {
				logger::debug("Conditions were not met; no duration multiplier added");
			}
		}

		logger::debug("Going through {} duration function multipliers", dMultipliersF.size());
		for (MultiplierF* multiplier : dMultipliersF) {
			const bool conditionsMet = Conditions::EvaluateConditionsList(multiplier, caster);

			if (conditionsMet) {
				auto mult = function::evaluateExpression(multiplier->function.function, function::AssignVariables(multiplier->function.variables, caster));

				auto& currentCast = Cast::GetCastInstance(caster);

				if (!currentCast.duration) {
					currentCast.duration = std::make_unique<AlteredDuration>();
				}

				currentCast.duration->multipliers.push_back(mult);
				logger::debug("Conditions were met; a duration function multiplier of {} added", mult);
			} else {
				logger::debug("Conditions  were not met; no duration function multiplier added");
			}
		}
	}

	void AddDurOverridesOnCast(RE::MagicCaster* caster)
	{
		logger::debug("Going through {} duration overrides", dOverrides.size());
		for (auto override : dOverrides) {
			const bool conditionsMet = Conditions::EvaluateConditionsList(override, caster);

			if (conditionsMet) {
				auto& currentCast = Cast::GetCastInstance(caster);

				if (!currentCast.duration) {
					currentCast.duration = std::make_unique<AlteredDuration>();
				}

				currentCast.duration->overrideValue = override->value;
				currentCast.duration->override = override->override;
				currentCast.duration->excluded = override->excluded;

				if (currentCast.duration->override) {
					if (currentCast.duration->excluded) {
						logger::debug("Conditions were met; duration override of {} set and exclusion set to {}", currentCast.duration->overrideValue, currentCast.duration->excluded);
					} else {
						logger::debug("Conditions were met; duration override of {} set.", currentCast.duration->overrideValue);
					}
				} else {
					if (currentCast.duration->excluded) {
						logger::debug("Conditions were met; Exclusion set to {}", currentCast.duration->excluded);
					}
				}
			} else {
				logger::debug("Conditions were not met; no duration override set");
			}
		}
	}
}
