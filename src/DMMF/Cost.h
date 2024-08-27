#pragma once

namespace Cost
{
	void CalculateNewCost(RE::MagicCaster* caster, bool log = false);

	void AddCostMultipliersOnCast(RE::MagicCaster* caster);
	void AddCostModifiersOnCast(RE::MagicCaster* caster);
	void AddCostOverridesOnCast(RE::MagicCaster* caster);

}
