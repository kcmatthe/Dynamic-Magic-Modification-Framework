#include "AlteredCast.h"

namespace Cast
{
	//Finds the correct instance so that when multiple actors are casting at the same time, multipliers and modifiers are only applied to the correct spells and correct casters
	std::vector<AlteredCast>::iterator GetCastInstance(RE::MagicCaster* caster)
	{
		auto targetCast = [&caster](const AlteredCast& ac) {
			return ac.caster == caster;
		};

		auto it = std::find_if(casts.begin(), casts.end(), targetCast);

		if (it != casts.end()) {
			return it;
		} else {
			logger::debug("AlteredCast object did not exist for this cast instance; Creating new object.");
			float n = caster->currentSpell->GetChargeTime();
			
			auto newCast = AlteredCast::AlteredCast(caster, nullptr, nullptr, nullptr);
			casts.push_back(newCast);

			return casts.end() - 1;
		}
	}
}
