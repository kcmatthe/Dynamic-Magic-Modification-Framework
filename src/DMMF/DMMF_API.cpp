
#include "AlteredCast.h"
#include "ChargeTime.h"
#include "Cost.h"
#include "Magnitude.h"

#include "DMMF_API.h"
#include <algorithm>

namespace
{
	std::vector<
		const DMMF_API::ProviderRegistration*>
		g_providers;

	std::shared_mutex g_providerLock;
}

namespace DMMF_API
{
	bool RegisterProvider_Impl(
		const ProviderRegistration* provider)
	{
		if (!provider) {
			return false;
		}

		if (provider->apiVersion != API_VERSION) {
			return false;
		}

		if (!provider->compute) {
			return false;
		}

		std::unique_lock lock(g_providerLock);

		auto it = std::find(
			g_providers.begin(),
			g_providers.end(),
			provider);

		if (it != g_providers.end()) {
			return true;
		}

		g_providers.push_back(provider);

		return true;
	}

	bool UnregisterProvider_Impl(
		const ProviderRegistration* provider)
	{
		std::unique_lock lock(g_providerLock);

		auto it = std::find(
			g_providers.begin(),
			g_providers.end(),
			provider);

		if (it == g_providers.end()) {
			return false;
		}

		g_providers.erase(it);

		return true;
	}

	 QueryResult RequestValue_Impl(
		const RE::ActorMagicCaster* caster)
	{
		QueryResult bestResult{};
		bestResult.handled = false;
		bestResult.priority = 0;
		bestResult.time = -1.0f;
		bestResult.timeMod = 0.0f;
		bestResult.timeMult = 1.0f;
		bestResult.cost = -1.0f;
		bestResult.costMod = 0.0f;
		bestResult.costMult = 1.0f;
		bestResult.resource = RE::ActorValue::kNone;
		bestResult.spell = nullptr;
	

		if (!caster) {
			return bestResult;
		}

		std::shared_lock lock(g_providerLock);

		for (auto* provider : g_providers) {
			QueryResult currentResult{};

			bool success =
				provider->compute(
					caster,
					&currentResult);

			if (!success) {
				continue;
			}

			if (!currentResult.handled) {
				continue;
			}

			if (!bestResult.handled ||
				currentResult.priority >
					bestResult.priority) {
				bestResult = currentResult;
			}
		}

		return bestResult;
	}

	const HostAPI g_api = {
		API_VERSION,
		RegisterProvider_Impl,
		UnregisterProvider_Impl,
		RequestValue_Impl
	};
}

extern "C" __declspec(dllexport)
	const DMMF_API::HostAPI* RequestDMMFAPI(
		std::uint32_t requestedVersion)
{
	if (requestedVersion !=
		DMMF_API::API_VERSION) {
		return nullptr;
	}

	return &DMMF_API::g_api;
}

/*
//Old code
//Charge Time
extern "C" DLLEXPORT float GetChargeTime(RE::MagicCaster* caster)
{
	logger::info("GetCastTime called from API");

	auto& currentCast = Cast::GetCastInstance(caster);

	if (!currentCast.charge) {
		currentCast.charge = std::make_unique<Cast::AlteredCharge>();
	}
	float origin = caster->currentSpell->GetChargeTime();
	logger::info("Original time was {} seconds", origin);
	
	ChargeTime::CalculateNewChargeTime(caster, origin);
	auto newTime = currentCast.charge->updatedTime;
	logger::info("Returning new time of {} seconds", newTime);

	return newTime;
}

extern "C" DLLEXPORT void SetChargeTime(RE::MagicCaster* caster, float time)
{
	logger::info("SetCastTime Called from API");

	logger::info("Cast time gather from other mods: {}", time);

	auto& currentCast = Cast::GetCastInstance(caster);

	if (!currentCast.charge) {
		currentCast.charge = std::make_unique<Cast::AlteredCharge>();
	}
	currentCast.charge->newBaseTime = time;


}

extern "C" DLLEXPORT void AddChargeTimeMultiplier(RE::MagicCaster* caster, float mult) {
	logger::info("AddMultiplier Called from API; adding multiplier of {}", mult);

	auto& currentCast = Cast::GetCastInstance(caster);

	if (!currentCast.charge) {
		currentCast.charge = std::make_unique<Cast::AlteredCharge>();
	}
	currentCast.charge->multipliers.push_back(mult);

}

extern "C" DLLEXPORT void AddChargeTimeModifier(RE::MagicCaster* caster, float mod) {
	logger::info("AddModifier Called from API; adding modifier of {}", mod);

	auto& currentCast = Cast::GetCastInstance(caster);

	if (!currentCast.charge) {
		currentCast.charge = std::make_unique<Cast::AlteredCharge>();
	}
	currentCast.charge->modifiers.push_back(mod);

}

//Cost
extern "C" DLLEXPORT void SetCost(RE::MagicCaster* caster, float cost)
{
	logger::info("SetCost Called from API");

	logger::info("Cast time gather from other mods: {}", cost);

	auto& currentCast = Cast::GetCastInstance(caster);

	if (!currentCast.cost) {
		currentCast.cost = std::make_unique<Cast::AlteredCost>();
	}
	currentCast.cost->newBaseCost = cost;

}

extern "C" DLLEXPORT void AddCostMultiplier(RE::MagicCaster* caster, float mult)
{
	logger::info("AddMultiplier Called from API; adding multiplier of {}", mult);

	auto& currentCast = Cast::GetCastInstance(caster);

	if (!currentCast.cost) {
		currentCast.cost = std::make_unique<Cast::AlteredCost>();
	}
	currentCast.cost->multipliers.push_back(mult);

}

extern "C" DLLEXPORT void AddCostModifier(RE::MagicCaster* caster, float mod)
{
	logger::info("AddModifier Called from API; adding modifier of {}", mod);

	auto& currentCast = Cast::GetCastInstance(caster);

	if (!currentCast.cost) {
		currentCast.cost = std::make_unique<Cast::AlteredCost>();
	}
	currentCast.cost->modifiers.push_back(mod);

}

extern "C" DLLEXPORT float GetCost(RE::MagicCaster* caster)
{
	logger::info("GetCost called from API");
	auto& currentCast = Cast::GetCastInstance(caster);

	if (!currentCast.cost) {
		currentCast.cost = std::make_unique<Cast::AlteredCost>();
	}
	float origin = caster->currentSpellCost;
	logger::info("Original cost was {} seconds", origin);
	
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

	auto& currentCast = Cast::GetCastInstance(caster);

	if (!currentCast.magnitude) {
		currentCast.magnitude = std::make_unique<Cast::AlteredMagnitude>();
	}
	currentCast.magnitude->newBaseMag = mag;

}

extern "C" DLLEXPORT void AddMagMultiplier(RE::MagicCaster* caster, float mult)
{
	logger::info("AddMultiplier Called from API; adding multiplier of {}", mult);

	auto& currentCast = Cast::GetCastInstance(caster);

	if (!currentCast.magnitude) {
		currentCast.magnitude = std::make_unique<Cast::AlteredMagnitude>();
	}
	currentCast.magnitude->multipliers.push_back(mult);

}

extern "C" DLLEXPORT void AddMagModifier(RE::MagicCaster* caster, float mod)
{
	logger::info("AddModifier Called from API; adding modifier of {}", mod);

	auto& currentCast = Cast::GetCastInstance(caster);

	if (!currentCast.magnitude) {
		currentCast.magnitude = std::make_unique<Cast::AlteredMagnitude>();
	}
	currentCast.magnitude->modifiers.push_back(mod);

}


//Spell
extern "C" DLLEXPORT void SetSpell(RE::MagicCaster * caster, RE::MagicItem * spell) {
	logger::info("SetSpell called from API; set modified to {}", spell->fullName.c_str());
	auto& currentCast = Cast::GetCastInstance(caster);

	if (!currentCast.spell) {
		currentCast.spell = std::make_unique<Cast::AlteredSpell>();
	}
	currentCast.spell->modified = spell;

}

//Resource
extern "C" DLLEXPORT void SetResource(RE::MagicCaster* caster, RE::ActorValue resource)
{
	logger::info("SetResource called from API; set resource to {}", resource);
	auto& currentCast = Cast::GetCastInstance(caster);
	currentCast.resource = resource;
}

extern "C" DLLEXPORT RE::ActorValue GetResource(RE::MagicCaster* caster)
{
	logger::info("GetResource called from API");
	auto& currentCast = Cast::GetCastInstance(caster);
	logger::info("Returning resource: {}", currentCast.resource);
	
	return currentCast.resource;
}
*/
