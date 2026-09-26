#include "Utility/CreateForm.h"
#include "Utility/Utility.h"
#include "Settings/Config.h"
#include "Settings/Conditions.h"
#include "Settings/Settings.h"
#include "AlteredCast.h"
#include "Magnitude.h"
#include "Spell.h"

#include "Utility/MagicNode.h"
#include "Hooks/Hooks.h"


namespace Spell
{
	using namespace config;
	using namespace Cast;
	using namespace Conditions;


	void AddReplacementSpellOnCast(RE::MagicCaster* caster)
	{
		auto spell = caster->currentSpell;

		logger::debug("Going through {} replacement spells", Replacements.size());
		for (auto replacement : Replacements) {
			std::vector<bool> bools = {};
			bool conditionsMet = false;
			for (Condition condition : replacement->conditions) {
				auto tempBool = EvaluateCondition(condition, caster);
				logger::trace("Individual condition check was {}", tempBool);
				bools.push_back(tempBool);
			}
			if (replacement->condOp == "or") {
				logger::trace("The comparative operator was 'or'");
				auto boolIt = std::find(bools.begin(), bools.end(), true);

				if (boolIt != bools.end()) {
					conditionsMet = true;
				} else {
					conditionsMet = false;
				}
			}
			if (replacement->condOp == "and") {
				logger::trace("The comparative operator was 'and'");
				auto boolIt = std::find(bools.begin(), bools.end(), false);

				if (boolIt != bools.end()) {
					conditionsMet = false;
				} else {
					conditionsMet = true;
				}
			}
			if (conditionsMet) {
				auto& currentCast = Cast::GetCastInstance(caster);

				if (!currentCast.spell) {
					currentCast.spell = std::make_unique<AlteredSpell>();
				}

				currentCast.spell->modified = replacement->replacement;
				logger::debug("Conditions were met; Replacement set to {}.", currentCast.spell->modified->fullName.c_str());
			} else {
				logger::debug("Conditions were not met; no replacement set");
			}
		}
	}


	void ReplaceSpell(RE::ActorMagicCaster* caster)
	{
		RE::MagicItem* spell = caster->currentSpell;
		RE::MagicItem* newSpell;
		if (spell) {
			auto type = spell->GetSpellType();
			auto casting = spell->GetCastingType();
			auto delivery = spell->GetDelivery();

			auto& currentCast = Cast::GetCastInstance(caster);

			if (!currentCast.spell) {
				currentCast.spell = std::make_unique<AlteredSpell>();
			}

			newSpell = currentCast.spell->modified;
			if (newSpell) {
				//this could be a setting option (only allow matching casting / delivery
				if ((casting == newSpell->GetCastingType() || Settings::bAllowUnmatchedCastType) && (delivery == newSpell->GetDelivery() || Settings::bAllowUnmatchedDeliveryType)){
					caster->currentSpell = newSpell;
					
					if (Settings::bReplaceVFX) {
						MagicNode::UpdateSpellVisualsAndSounds(caster, newSpell);
					}

				} else {
					logger::info("Casting Types or Delivery Types do not match, returning original spell");
					caster->currentSpell = spell;
				}
			} else {
				caster->currentSpell = spell;
			}
		}
		
	}

	
}
