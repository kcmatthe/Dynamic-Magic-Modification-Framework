#include "Settings/Config.h"
#include "AlteredCast.h"
#include "Resource.h"

namespace Resource
{
	using namespace config;
	using namespace Conditions;

	RE::ActorValue UpdateCastingResource(RE::MagicCaster* caster)
	{
		auto it = Cast::GetCastInstance(caster);
		auto currentCast = *it;

		auto resource = currentCast.resource;

		return resource;
	}

	void AddResourceOnCast(RE::MagicCaster* caster)
	{
		auto spell = caster->currentSpell;

		logger::trace("Going through {} resources", Resources.size());
		for (auto resource : Resources) {
			std::vector<bool> bools = {};
			bool conditionsMet = false;
			for (Condition condition : resource->conditions) {
				auto tempBool = CheckCondition(condition, caster);
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
				auto it = Cast::GetCastInstance(caster);
				auto currentCast = *it;
				currentCast.resource = resource->resource;
				
				*it = currentCast;
				logger::trace("Conditions were met; Casting resource updated.");
			} else {
				logger::trace("Conditions were not met; no resource set");
			}
		}
	}
}
