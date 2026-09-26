#pragma once

#include <cstdint>
#include <vector>
#include <shared_mutex>

namespace DMMF_API
{
	inline constexpr std::uint32_t API_VERSION = 1;

	struct QueryContext
	{
		RE::Actor* actor{ nullptr };
		RE::MagicItem* magicItem{ nullptr };
		float baseValue{ 0.0f };
		std::uint32_t eventID{ 0 };
	};

	struct QueryResult
	{
		bool handled{ false };
		float time{ -1.0f };
		float timeMult{ 1.0f };
		float timeMod{ 0.0f };
		float cost{ -1.0f };
		float costMult{ 1.0f };
		float costMod{ 0.0f };
		RE::MagicItem* spell{ nullptr };
		RE::ActorValue resource{ RE::ActorValue::kNone };

		std::uint32_t priority{ 0 };
	};

	using ComputeValueFn = bool (*)(const RE::ActorMagicCaster* caster, QueryResult* result);

	struct ProviderRegistration
	{
		std::uint32_t apiVersion;
		const char* providerName;
		ComputeValueFn compute;
	};

	using RegisterProviderFn =
		bool (*)(
			const ProviderRegistration* provider);

	using UnregisterProviderFn =
		bool (*)(
			const ProviderRegistration* provider);

	using RequestValueFn =
		QueryResult (*)(
			const RE::ActorMagicCaster* caster);

	struct HostAPI
	{
		std::uint32_t apiVersion;

		RegisterProviderFn RegisterProvider;
		UnregisterProviderFn UnregisterProvider;
		RequestValueFn RequestValue;
	};
	extern const HostAPI g_api;
}

extern "C"
{
	__declspec(dllexport)
		const DMMF_API::HostAPI* RequestDMMFAPI(
			std::uint32_t requestedVersion);
}


/* old code 
#define DMMF_EXPORTS
	
#ifdef DMMF_EXPORTS
	#define DLLEXPORT __declspec(dllexport)
#else
	#define DLLEXPORT __declspec(dllimport)
#endif


extern "C" DLLEXPORT float GetChargeTime(RE::MagicCaster* caster);
	

extern "C" DLLEXPORT void SetChargeTime(RE::MagicCaster* caster, float time);

extern "C" DLLEXPORT void AddChargeTimeMultiplier(RE::MagicCaster* caster, float mult);

extern "C" DLLEXPORT void AddChargeTimeModifier(RE::MagicCaster* caster, float mod);

extern "C" DLLEXPORT float GetCost(RE::MagicCaster* caster);

extern "C" DLLEXPORT void SetCost(RE::MagicCaster* caster, float cost);

extern "C" DLLEXPORT void AddCostMultiplier(RE::MagicCaster* caster, float mult);

extern "C" DLLEXPORT void AddCostModifier(RE::MagicCaster* caster, float mod);

//extern "C" DLLEXPORT float GetMagnitude(RE::MagicCaster* caster); //Not happy with the overall implementation of magnitude modification, so I want to wait on this.

extern "C" DLLEXPORT void SetMagnitude(RE::MagicCaster* caster, float mag);

extern "C" DLLEXPORT void AddMagMultiplier(RE::MagicCaster* caster, float mult);

extern "C" DLLEXPORT void AddMagModifier(RE::MagicCaster* caster, float mod);

extern "C" DLLEXPORT void SetSpell(RE::MagicCaster* caster, RE::MagicItem* spell);	

extern "C" DLLEXPORT void SetResource(RE::MagicCaster* caster, RE::ActorValue resource);

extern "C" DLLEXPORT RE::ActorValue GetResource(RE::MagicCaster* caster);

*/
	

