#pragma once


namespace AlternateAV
{
	//These functions will be used to alter what av is used to cast spells.
	
	bool FlashTrueHUDMeter(RE::Actor* a_actor, RE::ActorValue a_actorValue, bool a_long);

	void DamageActorValue(RE::Actor* a_actor, RE::ActorValue a_actorValue, float a_value);

	void FlashHealthMeter(RE::Actor* a_actor);

	float GetActorValue(RE::Actor* a_actor, RE::ActorValue a_actorValue);

	float GetBaseActorValue(RE::Actor* a_actor, RE::ActorValue a_actorValue);

	float GetMagickaCost(RE::MagicItem* a_magicItem, RE::Actor* a_actor, bool a_dualCast);

	std::int32_t GetStepCount();

	void InterruptActor(RE::Actor* a_actor, RE::MagicSystem::CastingSource a_castingSource);

	template <typename T>
	inline static void SafeSet(T* a_ptr, T a_value)
	{
		if (a_ptr) {
			*a_ptr = a_value;
		}
	}

	void ShowCannotCastReason(RE::MagicSystem::CannotCastReason a_reason, RE::ActorValue av);
	
}
