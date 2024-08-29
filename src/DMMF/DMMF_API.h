#pragma once

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


	

