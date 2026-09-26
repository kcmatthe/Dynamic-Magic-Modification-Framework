#include "Settings/Config.h"
#include "AlteredCast.h"
#include "Resource.h"
#include "Settings/Conditions.h"
#include "PluginData.h"

namespace Resource
{
	using namespace config;
	using namespace Conditions;
	using namespace PluginData;

	RE::ActorValue UpdateCastingResource(RE::MagicCaster* caster)
	{
		auto& currentCast = Cast::GetCastInstance(caster);


		auto resource = currentCast.resource;

		if (resource != RE::ActorValue::kNone) {
			return resource;
		} else {
			logger::warn("Resource was kNone, returning Magicka.");
			return RE::ActorValue::kMagicka;
		}
	}

	RE::ActorValue UpdateSecondaryResource(RE::MagicCaster* caster)
	{
		auto& currentCast = Cast::GetCastInstance(caster);

		auto resource = currentCast.secondaryResource;

		if (resource != RE::ActorValue::kNone) {
			return resource;
		} else {
			logger::warn("Secondary resource was kNone");
			return resource;
		}
	}

	void EvaluateResourceMagicEffects(RE::MagicCaster* caster)
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

		for (auto& activeEffect : activeEffects) {
			if (!activeEffect) {
				continue;
			}

			if (activeEffect->effect->baseEffect->HasKeyword(HealthResourceKYWD)) {
				currentCast.resource = RE::ActorValue::kHealth;
				logger::debug("Health Resource Keyword; Setting resource to Health");
			}
			if (activeEffect->effect->baseEffect->HasKeyword(StaminaResourceKYWD)) {
				currentCast.resource = RE::ActorValue::kStamina;
				logger::debug("Stamina Resource Keyword; Setting resource to Stamina");
			}
			if (activeEffect->effect->baseEffect->HasKeyword(MagickaResourceKYWD)) {
				currentCast.resource = RE::ActorValue::kMagicka;
				logger::debug("Stamina Resource Keyword; Setting resource to Stamina");
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

	void AddResourceOnCast(RE::MagicCaster* caster)
	{
		auto spell = caster->currentSpell;

		logger::debug("Going through {} resources", Resources.size());
		for (auto resource : Resources) {
			std::vector<bool> bools = {};
			bool conditionsMet = false;
			for (Condition condition : resource->conditions) {
				auto tempBool = EvaluateCondition(condition, caster);
				logger::trace("Individual condition check was {}", tempBool);
				bools.push_back(tempBool);
			}
			if (resource->condOp == "or") {
				logger::trace("The comparative operator was 'or'");
				auto boolIt = std::find(bools.begin(), bools.end(), true);

				if (boolIt != bools.end()) {
					conditionsMet = true;
				} else {
					conditionsMet = false;
				}
			}
			if (resource->condOp == "and") {
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

				currentCast.resource = resource->resource;
				currentCast.secondaryResource = resource->secondaryResource;
				currentCast.secondaryMult = resource->secondaryMult;
				
				logger::debug("Conditions were met; Casting resource updated.");
			} else {
				logger::debug("Conditions were not met; no resource set");
			}
		}

		EvaluateResourceMagicEffects(caster);
	}
}
