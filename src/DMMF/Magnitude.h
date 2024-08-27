#pragma once
#include "AlteredCast.h"
namespace Magnitude
{
	float CalculateNewMag(float origin, Cast::AlteredMagnitude* magnitude);

	void AddMagMultipliersOnCast(RE::MagicCaster* caster);
	void AddMagModifiersOnCast(RE::MagicCaster* caster);
	void AddMagOverridesOnCast(RE::MagicCaster* caster);
}
