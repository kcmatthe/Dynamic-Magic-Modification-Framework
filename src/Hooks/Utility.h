#ifndef UTILITY_H
#define UTILITY_H

//#include <nlohmann/json.hpp>
#include <unordered_set>

RE::ActorValue LookupActorValueByName(const char* av_name);
std::int32_t QueryStat(const char* name);
int RandomInt(int min, int max);
RE::BSTArray<RE::ObjectRefHandle> GetUndiscoveredMapMarkers();
bool HasEffect(RE::Actor* actor, RE::EffectSetting* effect);
float GetWarmthRating(RE::Actor* actor);
bool IsMarkerInPlayerWorld(RE::TESWorldSpace* a_refWorld);
RE::TESWorldSpace* GetRootWorld(RE::TESWorldSpace* a_World);
RE::BGSLocation* GetRootLocation(RE::BGSLocation* a_loc);
std::string GetPlayerRootWorldName();
bool IsPlayersMount(const RE::Actor* actor);

#endif  // UTILITY_H
