#pragma once

namespace DMMF
{
	//Charge Time Functions
	typedef int (*MyFloatFunc)(RE::MagicCaster*, float);
	void SetChargeTime(RE::MagicCaster* caster, float time)
	{
		static auto dmmf = REX::W32::GetModuleHandleW(L"DynamicMagicModificationFramework");
		MyFloatFunc func = (MyFloatFunc)GetProcAddress(dmmf, "SetChargeTime");
		if (func) {
			logger::debug("running set charge time func");
			func(caster, time);
		}
	}
	float GetChargeTime(RE::MagicCaster* caster)
	{
		static auto dmmf = REX::W32::GetModuleHandleW(L"DynamicMagicModificationFramework");
		static auto func = reinterpret_cast<float (*)(RE::MagicCaster*)>(GetProcAddress(dmmf, "GetChargeTime"));
		if (func) {
			logger::debug("running get charge time func");
			return func(caster);
		}
		return -1;
	}
	void AddChargeTimeMultiplier(RE::MagicCaster* caster, float mult)
	{
		static auto dmmf = REX::W32::GetModuleHandleW(L"DynamicMagicModificationFramework");
		MyFloatFunc func = (MyFloatFunc)GetProcAddress(dmmf, "AddChargeTimeMultiplier");
		if (func) {
			logger::debug("running add charge time multiplier func");
			func(caster, mult);
		}
	}
	void AddChargeTimeModifier(RE::MagicCaster* caster, float mod)
	{
		static auto dmmf = REX::W32::GetModuleHandleW(L"DynamicMagicModificationFramework");
		MyFloatFunc func = (MyFloatFunc)GetProcAddress(dmmf, "AddChargeTimeModifier");
		if (func) {
			logger::debug("running add charge time modifier func");
			func(caster, mod);
		}
	}

	//Cost Functions
	void SetCost(RE::MagicCaster* caster, float time)
	{
		static auto dmmf = REX::W32::GetModuleHandleW(L"DynamicMagicModificationFramework");
		MyFloatFunc func = (MyFloatFunc)GetProcAddress(dmmf, "SetChargeTime");
		if (func) {
			logger::debug("running set cost func");
			func(caster, time);
		}
	}
	float GetCost(RE::MagicCaster* caster)
	{
		static auto dmmf = REX::W32::GetModuleHandleW(L"DynamicMagicModificationFramework");
		static auto func = reinterpret_cast<float (*)(RE::MagicCaster*)>(GetProcAddress(dmmf, "GetCost"));
		if (func) {
			logger::debug("running get cost func");
			return func(caster);
		}
		return -1;
	}
	void AddCostMultiplier(RE::MagicCaster* caster, float mult)
	{
		static auto dmmf = REX::W32::GetModuleHandleW(L"DynamicMagicModificationFramework");
		MyFloatFunc func = (MyFloatFunc)GetProcAddress(dmmf, "AddCostMultiplier");
		if (func) {
			logger::debug("running add cost multiplier func");
			func(caster, mult);
		}
	}
	void AddCostModifier(RE::MagicCaster* caster, float mod)
	{
		static auto dmmf = REX::W32::GetModuleHandleW(L"DynamicMagicModificationFramework");
		MyFloatFunc func = (MyFloatFunc)GetProcAddress(dmmf, "AddCostModifier");
		if (func) {
			logger::debug("running add cost modifier func");
			func(caster, mod);
		}
	}

	typedef int (*MySpellFunc)(RE::MagicCaster*, RE::MagicItem*);
	void SetSpell(RE::MagicCaster* caster, RE::MagicItem* spell) {
		static auto dmmf = REX::W32::GetModuleHandleW(L"DynamicMagicModificationFramework");
		MySpellFunc func = (MySpellFunc)GetProcAddress(dmmf, "SetSpell");
		if (func) {
			logger::debug("running set spell func");
			func(caster, spell);
		}
	}

	typedef int (*MyAVFunc)(RE::MagicCaster*, RE::ActorValue);
	void SetResource(RE::MagicCaster* caster, RE::ActorValue av)
	{
		static auto dmmf = REX::W32::GetModuleHandleW(L"DynamicMagicModificationFramework");
		MyAVFunc func = (MyAVFunc)GetProcAddress(dmmf, "SetResource");
		if (func) {
			logger::debug("running set resource func");
			func(caster, av);
		}
	}

	RE::ActorValue GetResource(RE::MagicCaster* caster)
	{
		static auto dmmf = REX::W32::GetModuleHandleW(L"DynamicMagicModificationFramework");
		static auto func = reinterpret_cast<RE::ActorValue (*)(RE::MagicCaster*)>(GetProcAddress(dmmf, "GetResource"));
		if (func) {
			logger::debug("running get resource func");
			return func(caster);
		}
		return RE::ActorValue::kMagicka;
	}
	

	#define DMMF_EXPORTS

	#ifdef DMMF_EXPORTS
	#	define DLLEXPORT __declspec(dllexport)
	#else
	#	define DLLEXPORT __declspec(dllimport)
	#endif

	
}
