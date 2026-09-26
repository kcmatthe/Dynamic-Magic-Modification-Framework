#include "Utility.h"
#include "Hooks.h"

#include <fmt/format.h>
#include <regex>


RE::ActorValue LookupActorValueByName(const char* av_name) {
    using func_t = decltype(&LookupActorValueByName);
    REL::Relocation<func_t> func{REL::ID(27203)};
    return func(av_name);
}

std::int32_t QueryStat(const char* name) {
    std::int32_t result[4] = {0, 0, 0, 0};
    using type = bool (*)(RE::BSFixedString, std::int32_t*);
    static REL::Relocation<type> func{REL::VariantID(16120, 16362, 0x2094F0)};
    return func(name, result) ? result[0] : 0;
}

int RandomInt(int min, int max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(min, max);
    return dist(gen);
}

RE::BSTArray<RE::ObjectRefHandle> GetUndiscoveredMapMarkers() {
    RE::BSTArray<RE::ObjectRefHandle> handles;

    const auto& [formMap, formMapLock] = RE::TESForm::GetAllForms();

    for (auto& [id, form] : *formMap) {
        if (!form || !form->Is(RE::FormType::Reference)) {
            continue;
        }

        auto* ref = form->As<RE::TESObjectREFR>();
        if (!ref || !ref->GetBaseObject() || ref->IsDisabled()) {
            continue;
        }

        auto* extraMapMarker = ref->extraList.GetByType<RE::ExtraMapMarker>();
        if (extraMapMarker && extraMapMarker->mapData && ref->GetWorldspace()) {
            if (IsMarkerInPlayerWorld(ref->GetWorldspace())) {
                auto* extraMapMarker = ref->extraList.GetByType<RE::ExtraMapMarker>();
                const auto flags = extraMapMarker->mapData->flags;
                if (!(flags & RE::MapMarkerData::Flag::kVisible)) {
                    logger::trace("UNDISCOVERED MARKER -- {}", extraMapMarker->mapData->locationName.GetFullName());
                    RE::ObjectRefHandle handle;
                    handle = ref;
                    handles.push_back(handle);
                }
            }
        }
    }

    auto player = RE::PlayerCharacter::GetSingleton();
    logger::info("[Wayfarer] Found {} undiscovered map markers.", handles.size());
    return handles;
}

bool HasEffect(RE::Actor* actor, RE::EffectSetting* effect) {
    if (!actor || !effect) {
        return false;
    }

    const auto activeEffects = actor->AsMagicTarget()->GetActiveEffectList();
    if (!activeEffects) {
        return false;
    }

    for (const auto& activeEffect : *activeEffects) {
        if (activeEffect && activeEffect->effect && activeEffect->effect->baseEffect == effect) {
            return true;
        }
    }

    return false;
}

float GetWarmthRating(RE::Actor* actor) {
    using func_t = decltype(GetWarmthRating);
    REL::Relocation<func_t> func{RELOCATION_ID(25834, 26394).address()};
    return func(actor);
}

bool IsMarkerInPlayerWorld(RE::TESWorldSpace* a_refWorld) {
    auto player = RE::PlayerCharacter::GetSingleton();
    RE::TESWorldSpace* targetWorld;
    if (!player->GetWorldspace()) {
        if (player->GetPlayerRuntimeData().cachedWorldSpace != nullptr) {
            targetWorld = player->GetPlayerRuntimeData().cachedWorldSpace;
            logger::trace("Cached worldspace: {}", targetWorld->GetName());
        } else {
            if (!player->GetParentCell() || !a_refWorld->location) {
                return false;
            } else if (!player->GetParentCell()->GetLocation()) {
                return false;
            }
            auto* rootLocation = GetRootLocation(player->GetParentCell()->GetLocation());
            auto* worldRootLoc = GetRootLocation(a_refWorld->location);
            if (rootLocation == worldRootLoc) {
                return true;
            } else {
                return false;
            }
        }
    } else {
        targetWorld = player->GetWorldspace();
    }
    targetWorld = GetRootWorld(targetWorld);
    logger::trace("Is Marker in World? {} -> {}", a_refWorld->GetName(), targetWorld->GetName());
    if (a_refWorld == targetWorld) {
        logger::trace("{} == {} -> YES", a_refWorld->GetName(), targetWorld->GetName());
        return true;
    } else if (a_refWorld->parentWorld) {
        logger::trace("Entering recursion for {}, parent is {}", a_refWorld->GetName(),
                      a_refWorld->parentWorld->GetName());
        return IsMarkerInPlayerWorld(a_refWorld->parentWorld);
    }
    logger::trace("{} != {} and {}->parentWorld doesn't exist -> NO", a_refWorld->GetName(), targetWorld->GetName(),
                  a_refWorld->GetName());
    return false;
}

RE::TESWorldSpace* GetRootWorld(RE::TESWorldSpace* a_world) {
    if (!a_world->parentWorld) {
        return a_world;
    } else {
        return GetRootWorld(a_world->parentWorld);
    }
}

RE::BGSLocation* GetRootLocation(RE::BGSLocation* a_loc) {
    if (a_loc->parentLoc) {
        logger::trace("Location {} has parent.", a_loc->GetName());
        return GetRootLocation(a_loc->parentLoc);
    } else {
        logger::trace("Returning root loc: {}", a_loc->GetName());
        return a_loc;
    }
}

std::string GetPlayerRootWorldName() { 
    auto player = RE::PlayerCharacter::GetSingleton();
    auto targetWorld = player->GetWorldspace();
    if (!targetWorld) {
        targetWorld = player->GetPlayerRuntimeData().cachedWorldSpace;
    }
    targetWorld = GetRootWorld(targetWorld);
    std::string worldName = targetWorld->GetName();
    return worldName;
}

bool IsPlayersMount(const RE::Actor* actor) {
    if (!actor) {
        return false;
    }

    RE::NiPointer<RE::Actor> playerMount;
    const auto player = RE::PlayerCharacter::GetSingleton();
    if (player->GetMount(playerMount)) {
        return playerMount.get() == actor;
    }

    return false;
}
