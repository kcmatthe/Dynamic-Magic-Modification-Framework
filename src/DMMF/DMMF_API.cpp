#include "DMMF_API.h"
#include "AlteredCast.h"
#include "ChargeTime.h"
#include "Cost.h"
#include "Magnitude.h"

#include <iostream>

//Charge Time
extern "C" DLLEXPORT float GetChargeTime(RE::MagicCaster* caster)
{
	logger::info("GetCastTime called from API");

	auto it = Cast::GetCastInstance(caster);
	auto currentCast = *it;
	if (!currentCast.charge) {
		currentCast.charge = new Cast::AlteredCharge();
	}
	float origin = caster->currentSpell->GetChargeTime();
	logger::info("Original time was {} seconds", origin);
	//auto newTime = ChargeTime::CalculateNewChargeTime(caster, origin); //Rather than calculate this repeatedly, calculate once and have it be a member of AlteredCast?
	ChargeTime::CalculateNewChargeTime(caster, origin);
	auto newTime = currentCast.charge->updatedTime;
	logger::info("Returning new time of {} seconds", newTime);

	return newTime;
}

extern "C" DLLEXPORT void SetChargeTime(RE::MagicCaster* caster, float time)
{
	logger::info("SetCastTime Called from API");

	logger::info("Cast time gather from other mods: {}", time);

	auto it = Cast::GetCastInstance(caster);
	auto currentCast = *it;
	if (!currentCast.charge) {
		currentCast.charge = new Cast::AlteredCharge();
	}
	currentCast.charge->newBaseTime = time;
	*it = currentCast;

}

extern "C" DLLEXPORT void AddChargeTimeMultiplier(RE::MagicCaster* caster, float mult) {
	logger::info("AddMultiplier Called from API; adding multiplier of {}", mult);

	auto it = Cast::GetCastInstance(caster);
	auto currentCast = *it;
	if (!currentCast.charge) {
		currentCast.charge = new Cast::AlteredCharge();
	}
	currentCast.charge->multipliers.push_back(mult);
	*it = currentCast;
}

extern "C" DLLEXPORT void AddChargeTimeModifier(RE::MagicCaster* caster, float mod) {
	logger::info("AddModifier Called from API; adding modifier of {}", mod);

	auto it = Cast::GetCastInstance(caster);
	auto currentCast = *it;
	if (!currentCast.charge) {
		currentCast.charge = new Cast::AlteredCharge();
	}
	currentCast.charge->modifiers.push_back(mod);
	*it = currentCast;
}

//Cost
extern "C" DLLEXPORT void SetCost(RE::MagicCaster* caster, float cost)
{
	logger::info("SetCost Called from API");

	logger::info("Cast time gather from other mods: {}", cost);

	auto it = Cast::GetCastInstance(caster);
	auto currentCast = *it;
	if (!currentCast.cost) {
		currentCast.cost = new Cast::AlteredCost();
	}
	currentCast.cost->newBaseCost = cost;
	*it = currentCast;
}

extern "C" DLLEXPORT void AddCostMultiplier(RE::MagicCaster* caster, float mult)
{
	logger::info("AddMultiplier Called from API; adding multiplier of {}", mult);

	auto it = Cast::GetCastInstance(caster);
	auto currentCast = *it;
	if (!currentCast.cost) {
		currentCast.cost = new Cast::AlteredCost();
	}
	currentCast.cost->multipliers.push_back(mult);
	*it = currentCast;
}

extern "C" DLLEXPORT void AddCostModifier(RE::MagicCaster* caster, float mod)
{
	logger::info("AddModifier Called from API; adding modifier of {}", mod);

	auto it = Cast::GetCastInstance(caster);
	auto currentCast = *it;
	if (!currentCast.cost) {
		currentCast.cost = new Cast::AlteredCost();
	}
	currentCast.cost->modifiers.push_back(mod);
	*it = currentCast;
}

extern "C" DLLEXPORT float GetCost(RE::MagicCaster* caster)
{
	logger::info("GetCost called from API");
	auto it = Cast::GetCastInstance(caster);
	auto currentCast = *it;
	if (!currentCast.cost) {
		currentCast.cost = new Cast::AlteredCost();
	}
	float origin = caster->currentSpellCost;
	logger::info("Original cost was {} seconds", origin);
	//auto newCost = Cost::CalculateNewCost(caster, origin);  //Rather than calculate this repeatedly, calculate once and have it be a member of AlteredCast?
	Cost::CalculateNewCost(caster, origin);
	auto newCost = currentCast.cost->updatedCost;
	logger::info("Returning new cost of {}", newCost);

	return newCost;
}

//Magnitude

extern "C" DLLEXPORT void SetMagnitude(RE::MagicCaster* caster, float mag)
{
	logger::info("SetCost Called from API");

	logger::info("Cast time gather from other mods: {}", mag);

	auto it = Cast::GetCastInstance(caster);
	auto currentCast = *it;
	if (!currentCast.magnitude) {
		currentCast.magnitude = new Cast::AlteredMagnitude();
	}
	currentCast.magnitude->newBaseMag = mag;
	*it = currentCast;
}

extern "C" DLLEXPORT void AddMagMultiplier(RE::MagicCaster* caster, float mult)
{
	logger::info("AddMultiplier Called from API; adding multiplier of {}", mult);

	auto it = Cast::GetCastInstance(caster);
	auto currentCast = *it;
	if (!currentCast.magnitude) {
		currentCast.magnitude = new Cast::AlteredMagnitude();
	}
	currentCast.magnitude->multipliers.push_back(mult);
	*it = currentCast;
}

extern "C" DLLEXPORT void AddMagModifier(RE::MagicCaster* caster, float mod)
{
	logger::info("AddModifier Called from API; adding modifier of {}", mod);

	auto it = Cast::GetCastInstance(caster);
	auto currentCast = *it;
	if (!currentCast.magnitude) {
		currentCast.magnitude = new Cast::AlteredMagnitude();
	}
	currentCast.magnitude->modifiers.push_back(mod);
	*it = currentCast;
}


//Spell
extern "C" DLLEXPORT void SetSpell(RE::MagicCaster * caster, RE::MagicItem * spell) {
	logger::info("SetSpell called from API; set modified to {}", spell->fullName.c_str());
	auto it = Cast::GetCastInstance(caster);
	auto currentCast = *it;
	if (!currentCast.spell) {
		currentCast.spell = new Cast::AlteredSpell();
	}
	currentCast.spell->modified = spell;
	*it = currentCast;
}

//Resource
extern "C" DLLEXPORT void SetResource(RE::MagicCaster* caster, RE::ActorValue resource)
{
	logger::info("SetResource called from API; set resource to {}", resource);
	auto it = Cast::GetCastInstance(caster);
	auto currentCast = *it;
	currentCast.resource = resource;
	*it = currentCast;
}

extern "C" DLLEXPORT RE::ActorValue GetResource(RE::MagicCaster* caster)
{
	logger::info("GetResource called from API");
	auto it = Cast::GetCastInstance(caster);
	auto currentCast = *it;
	logger::info("Returning resource: {}", currentCast.resource);
	
	return currentCast.resource;
}

