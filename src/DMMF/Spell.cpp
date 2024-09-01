#include "Utility/CreateForm.h"
#include "Utility/Utility.h"
#include "Settings/Config.h"
#include "AlteredCast.h"
#include "Magnitude.h"
#include "Spell.h"


namespace Spell
{
	using namespace config;
	using namespace Cast;
	using namespace Conditions;

	void GetPluginData()
	{
		
	}

	//These seem to work. Just need to figure out if it is worth it to delete forms as we go. Probably is worth it but idk the best way to do so.

	//Creates a new MagicItem form; allows each copied / modified spell to be its own item/reference to prevent weird overlaps when multiple actors get spells replaced
	RE::MagicItem* CreateNewMagicItemCopy(RE::MagicItem* spell) {
		if (!spell) {
			return nullptr;
		}

		auto* newSpell = AddNewForm(spell)->As<RE::MagicItem>();
		if (newSpell) {
			logger::debug("new form id", newSpell->GetFormID());
		}

		
		return newSpell;
	}

	//Creats a new EffectSetting item / reference; EffectSetting does not havea default constructor so this allows new baseEffects to be made so you don't have to use and edit an existing one.
	RE::EffectSetting* CreateNewEffectSettingCopy(RE::EffectSetting* spell)
	{
		if (!spell) {
			return nullptr;
		}

		auto* newEffect = AddNewForm(spell)->As<RE::EffectSetting>();
		if (newEffect) {
			logger::debug("new form id", newEffect->GetFormID());
		}
		
		return newEffect;
	}

	void AddReplacementSpellOnCast(RE::MagicCaster* caster) { 
		auto spell = caster->currentSpell;

		logger::debug("Going through {} replacement spells", Replacements.size());
		for (auto replacement : Replacements) {
			std::vector<bool> bools = {};
			bool conditionsMet = false;
			for (Condition condition : replacement->conditions) {
				auto tempBool = CheckCondition(condition, caster);
				logger::debug("Individual condition check was {}", tempBool);
				bools.push_back(tempBool);
			}
			if (replacement->condOp == "or") {
				logger::debug("The comparative operator was 'or'");
				auto boolIt = std::find(bools.begin(), bools.end(), true);

				if (boolIt != bools.end()) {
					conditionsMet = true;
				} else {
					conditionsMet = false;
				}
			}
			if (replacement->condOp == "and") {
				logger::debug("The comparative operator was 'and'");
				auto boolIt = std::find(bools.begin(), bools.end(), false);

				if (boolIt != bools.end()) {
					conditionsMet = false;
				} else {
					conditionsMet = true;
				}
			}
			if (conditionsMet) {
				auto it = Cast::GetCastInstance(caster);
				auto currentCast = *it;
				if (!currentCast.spell) {
					currentCast.spell = new AlteredSpell();
				}
				currentCast.spell->modified = replacement->replacement;
				*it = currentCast;
				logger::debug("Conditions were met; Replacement set to {}.", currentCast.spell->modified->fullName.c_str());
			} else {
				logger::debug("Conditions were not met; no replacement set");
			}
		}
	}

	void CopyAndAddMagicEffect(RE::Effect* from, RE::MagicItem* to) {
		logger::debug("Copying effect {}", from->baseEffect->fullName.c_str());
		auto newEffect = new RE::Effect();

		//Copy EffectItem
		newEffect->effectItem.magnitude = from->effectItem.magnitude;
		newEffect->effectItem.area = from->effectItem.area;
		newEffect->effectItem.duration = from->effectItem.duration;

		//Copy Cost
		newEffect->cost = from->cost;

		//Copy padding
		newEffect->pad0C = from->pad0C;
		newEffect->pad1C = from->pad1C;

		//Copy Conditions
		newEffect->conditions = from->conditions;

		//Copy BaseEffect

		//newEffect->baseEffect = GenericEffect;
		newEffect->baseEffect = CreateNewEffectSettingCopy(from->baseEffect);

		if (newEffect->baseEffect) {
			newEffect->baseEffect->data = from->baseEffect->data;
			newEffect->baseEffect->data.projectileBase = from->baseEffect->data.projectileBase;
			newEffect->baseEffect->data.associatedForm = from->baseEffect->data.associatedForm;
			newEffect->baseEffect->effectSounds = from->baseEffect->effectSounds;
			newEffect->baseEffect->conditions = from->baseEffect->conditions;
		} else {
			logger::info("Generic Effect was null");
		}

		std::string oldName = from->baseEffect->fullName.c_str();
		auto newName = "Copy of " + oldName;
		newEffect->baseEffect->SetFullName(newName.c_str());
		logger::debug("Pushed back effect {}", newEffect->baseEffect->fullName.c_str());

		//Push back new effect
		to->effects.push_back(newEffect);
	}

	bool _copySpell(RE::SpellItem* from, RE::SpellItem* to)
	{
		to->effects.clear();
		logger::debug("After clearing, {} effects attached to generic spell", to->effects.size());
		
		//Go through each effect from the original spell and copy it into the new spell if it is NOT the costliest effect
		for (auto item : from->effects) {
			if (item != from->GetCostliestEffectItem()) {
				CopyAndAddMagicEffect(item, to);
			}
		}
		
		//Copy the costliest effect last - this ensures it's projectile, effect shader, etc. are priotized
		//Normally the costiliest effect should always take priority, but it seems when adding effects mid game like this, the effect added most recently takes priority
		auto costliest = from->GetCostliestEffectItem();
		CopyAndAddMagicEffect(costliest, to);
		
		to->data = from->data;

		std::string oldName = from->fullName.c_str();
		auto newName = "Copy of " + oldName;
		to->SetFullName(newName.c_str());
		
		return true;
	}

	RE::MagicItem* CopyCasterSpell(RE::MagicCaster* caster)
	{
		logger::debug("copying original spell into temp spell");
		auto from = caster->currentSpell;
		RE::MagicItem* to = nullptr;
		to = CreateNewMagicItemCopy(from);
		auto castingSource = caster->GetCastingSource();
		auto actor = caster->GetCasterAsActor();
		auto Player = RE::PlayerCharacter::GetSingleton();
	
		if (to) {
			_copySpell(from->As<RE::SpellItem>(), to->As<RE::SpellItem>());
		} 

		return to;
	}

	void ReplaceSpell(RE::MagicCaster* caster)
	{
		RE::MagicItem* spell = caster->currentSpell;
		RE::MagicItem* newSpell;
		if (spell) {
			auto type = spell->GetSpellType();
			auto casting = spell->GetCastingType();

			auto it = GetCastInstance(caster);
			auto currentCast = *it;
			if (!currentCast.spell) {
				currentCast.spell = new AlteredSpell();
			}
			newSpell = currentCast.spell->modified;
			if (newSpell) {
				caster->currentSpell = newSpell;
			} else {
				caster->currentSpell = spell;
			}
		}
	}

	void ModifyMagnitude(RE::MagicCaster* caster) {
		logger::debug("Modifying Magnitude");
		auto spell = caster->currentSpell;

		//AlternateSpell newSpell = ModifyEffectMagnitude(caster);
		auto it = GetCastInstance(caster);
		auto currentCast = *it;
		auto magnitude = currentCast.magnitude;
		if (!currentCast.spell) {
			currentCast.spell = new AlteredSpell();
		}
		if (magnitude) {
			if (spell) {
				currentCast.spell->original = spell;
				currentCast.spell->modified = CopyCasterSpell(caster);
				if (magnitude->costliestOnly) {
					logger::debug("Only altering costliest effect");
					currentCast.spell->modified->GetCostliestEffectItem()->effectItem.magnitude = Magnitude::CalculateNewMag(spell->GetCostliestEffectItem()->effectItem.magnitude, magnitude);
				} else {
					for (auto effect : currentCast.spell->modified->effects) {
						effect->effectItem.magnitude = Magnitude::CalculateNewMag(effect->effectItem.magnitude, magnitude);
					}
				}
			}
		}
		*it = currentCast;
	}
}
