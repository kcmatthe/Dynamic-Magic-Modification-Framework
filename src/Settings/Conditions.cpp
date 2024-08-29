
#include "Conditions.h"
#include "Utility/Utility.h"

namespace Conditions
{
	std::variant<std::monostate, RE::ActorValue, RE::EffectSetting*, RE::MagicSystem::SpellType, RE::MagicSystem::CastingType, RE::MagicItem*, RE::Actor*, RE::BGSPerk*, RE::MagicSystem::CastingSource, std::string, float, bool> AssignVariable(std::string variable, std::variant<std::string, float, bool> value)
	{
		//logger::info("Assigning variable");

		if (auto svalue = std::get_if<std::string>(&value)) {
			if (variable == "form" || variable == "Form") {
				auto form = Utility::TES::GetFormFromEditorID<RE::MagicItem>(svalue->c_str());

				if (form) {
					return form;
				} else {
					return 0.0f;
				}
			
			}  else if (variable == "hand" || variable == "Hand") {
				if (*svalue == "left" || *svalue == "Left") {
					return RE::MagicSystem::CastingSource::kLeftHand;
				} else if (*svalue == "right" || *svalue == "Right") {
					return RE::MagicSystem::CastingSource::kRightHand;
				}
			} else if (variable == "perk" || variable == "Perk") {
				auto form = Utility::TES::GetFormFromEditorID<RE::BGSPerk>(svalue->c_str());

				if (form) {
					return form;
				} else {
					return 0.0f;
				}
			} else if (variable == "keyword" || variable == "Keyword") {
				return *svalue;
			} else if (variable == "caster" || variable == "Caster") {
				if (*svalue == "player" || *svalue == "Player") {
					RE::Actor* PlayerRef = RE::PlayerCharacter::GetSingleton();
					return PlayerRef;
				} else {
					auto actor = Utility::TES::GetFormFromEditorID<RE::Actor>(svalue->c_str());

					if (actor) {
						return actor;
					} else {
						return 0.0f;
					}
				}

			} else if (variable == "castType" || variable == "CastType") {
				if (*svalue == "concentration" || *svalue == "Concentration") {
					return RE::MagicSystem::CastingType::kConcentration;
				} else if (*svalue == "FireForget" || *svalue == "fireforget") {
					return RE::MagicSystem::CastingType::kFireAndForget;
				}
			} else if (variable == "school" || variable == "School") {
				if (*svalue == "Alteration" || *svalue == "alteration") {
					return RE::ActorValue::kAlteration;
				} else if (*svalue == "Conjuration" || *svalue == "conjuration") {
					return RE::ActorValue::kConjuration;
				} else if (*svalue == "Destruction" || *svalue == "destruction") {
					return RE::ActorValue::kDestruction;
				} else if (*svalue == "Illusion" || *svalue == "illusion") {
					return RE::ActorValue::kIllusion;
				} else if (*svalue == "Restoration" || *svalue == "restoration") {
					return RE::ActorValue::kRestoration;
				}
			} else if (variable == "magicType" || variable == "MagicType") {
				if (*svalue == "Spell" || *svalue == "spell") {
					return RE::MagicSystem::SpellType::kSpell;
				} else if (*svalue == "Scroll" || *svalue == "scroll") {
					return RE::MagicSystem::SpellType::kScroll;
				} else if (*svalue == "Staff" || *svalue == "staff") {
					return RE::MagicSystem::SpellType::kStaffEnchantment;
				}
			}  else if (variable == "casterHasEffect" || variable == "CasterHasEffect") {
				auto form = Utility::TES::GetFormFromEditorID<RE::EffectSetting>(svalue->c_str());

				if (form) {
					return form;
				} else {
					return 0.0f;
				}
			}
		} else if (auto fvalue = std::get_if<float>(&value)) {
			return *fvalue; //should always be able to just return the float, no interpretation necessary.
			/* if (variable == "skill" || variable == "Skill") {
				//logger::info("Assigned to {}", *fvalue);
				return *fvalue;
			} else if (variable == "difficulty" || variable == "Difficulty") {
				//logger::info("Assigned to {}", *fvalue);
				return *fvalue;
			} else if (variable == "global" || variable == "Global") { //need to think about this, what am I assigning here again?
				return *fvalue;
			}
			*/
		} else if (auto bvalue = std::get_if<bool>(&value)) {
			return *bvalue; //should be able to just return the bool, no interpretation necessary
		}
	}

	bool CheckCondition(Condition condition, RE::MagicCaster* caster)
	{
		//logger::info("check condition");
		std::variant<std::monostate, RE::ActorValue, RE::EffectSetting*, RE::MagicSystem::SpellType, RE::MagicSystem::CastingType, RE::MagicItem*, RE::Actor*, RE::BGSPerk*, RE::MagicSystem::CastingSource, std::string, float, bool> argument;
		auto variable = condition.variable;
		auto op = condition.op;
		auto value = condition.value;
		auto variableDetail = condition.variableDetail;

		//logger::info("variable is {}, and operator is {}", variable, op);

		if (caster) {
			//logger::info("Getting costliest");
			auto costliest = caster->currentSpell->GetCostliestEffectItem()->baseEffect;
			if (costliest) {
				//logger::info("Getting av");
				auto av = costliest->GetMagickSkill();
				//logger::info("Getting difficulty");
				float difficulty = costliest->GetMinimumSkillLevel();
				//logger::info("Getting actor");
				auto* actor = caster->GetCasterAsActor();
				//logger::info("Getting skill");
				float skill = 0;
				if (actor && av != RE::ActorValue::kNone) {
					skill = actor->AsActorValueOwner()->GetActorValue(av);
				}
				//logger::info("Getting type");
				auto type = caster->currentSpell->GetSpellType();
				//logger::info("Getting casting");
				auto casting = caster->currentSpell->GetCastingType();
				auto hand = caster->GetCastingSource();

				if (variable == "form" || variable == "Form") {
					argument = caster->currentSpell;
				} else if (variable == "school" || variable == "School") {
					argument = av;
				} else if (variable == "skill" || variable == "Skill") {
					argument = skill;
				} else if (variable == "difficulty" || variable == "Difficulty") {
					argument = difficulty;
				} else if (variable == "magicType" || variable == "MagicType") {
					argument = type;
				} else if (variable == "castType" || variable == "CastType") {
					argument = casting;
				} else if (variable == "global" || variable == "Global") {
					auto global = Utility::TES::GetFormFromEditorID<RE::TESGlobal>(variableDetail);
					if (global) {
						argument = global->value;
					} else {
						return false;
					}
				} else if (variable == "caster" || variable == "Caster") {
					argument = caster->GetCasterAsActor();
				} else if (variable == "perk" || variable == "Perk") {
					auto perk = std::get_if<RE::BGSPerk*>(&value);
					if (op == "==") {
						if (actor && actor->HasPerk(*perk)) {
							return true;
						} else {
							return false;
						}
					} else if (op == "!=") {
						if (actor && !actor->HasPerk(*perk)) {
							return true;
						} else {
							return false;
						}
					}
				} else if (variable == "keyword" || variable == "Keyword") {
					if (op == "==") {
						if (caster->currentSpell->HasKeywordString(std::get_if<std::string>(&value)->c_str())) {
							return true;
						} else {
							return false;
						}
					} else if (op == "!=") {
						if (!caster->currentSpell->HasKeywordString(std::get_if<std::string>(&value)->c_str())) {
							return true;
						} else {
							return false;
						}
					}
				} else if (variable == "hand" || variable == "Hand") {
					argument = hand;
				} else if (variable == "isSneaking" || variable == "IsSneaking") {
					if (actor) {
						argument = actor->IsSneaking();
					}
				} else if (variable == "isDualCasting" || variable == "IsDualCasting") {
					argument = caster->GetIsDualCasting();
				} /* else if (variable == "isDetected" || variable == "IsDetected") { //seems trickier than anticipated
					if (actor) {
						argument = actor->RequestDetectionLevel();
						RE::PlayerCharacter;
					}
				}*/ else if (variable == "casterHasEffect" || variable == "CasterHasEffect") {
					if (auto activeEffect = actor->AsMagicTarget()->GetActiveEffectList()) {
						bool hasIt = false;
						for (auto& ae : *activeEffect) {
							if (!ae) {
								break;
							}
							if (!ae->effect) {
								continue;
							}
							if (!ae->effect->baseEffect) {
								continue;
							}
							if (ae->effect->baseEffect == *std::get_if<RE::EffectSetting*>(&value)) {
								hasIt = true;
							}
						}
						argument = hasIt;
					}
				}

				if (op == ">") {
					return argument > value;  //greater than
				} else if (op == "<") {
					return argument < value;  //less than
				} else if (op == ">=") {
					return argument >= value;  //greater than or equal to
				} else if (op == "<=") {
					return argument <= value;  //less than or equal to
				} else if (op == "==") {
					return argument == value;  //equal to
				} else if (op == "!=") {
					return argument != value;  //does not equal
				} else {
					return false;
				}
			} else {
				return false;
			}
		} else {
			return false;
		}
	}

}
