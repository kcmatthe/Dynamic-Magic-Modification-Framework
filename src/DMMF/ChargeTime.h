#pragma once

namespace ChargeTime
{
	void CalculateNewChargeTime(RE::MagicCaster* caster, float origin, bool log = false);

	void AddChargeMultipliersOnCast(RE::MagicCaster* caster);
	void AddChargeModifiersOnCast(RE::MagicCaster* caster);
	void AddChargeOverridesOnCast(RE::MagicCaster* caster);

}

