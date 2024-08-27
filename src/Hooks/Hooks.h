#pragma once

namespace Hooks
{
	struct GetChargeTimeVHook
	{
	public:
		static void Install();

		static float GetScrollChargeTime(RE::ScrollItem* spell);
		static float GetSpellChargeTime(RE::SpellItem* spell);
		static float GetEnchantChargeTime(RE::EnchantmentItem* spell);

		static inline REL::Relocation<decltype(GetScrollChargeTime)> funcScroll64;
		static inline REL::Relocation<decltype(GetEnchantChargeTime)> funcEnchant64;
		static inline REL::Relocation<decltype(GetSpellChargeTime)> func64;

	};

	struct CasterHook
	{
	public:
		static void RequestCastImpl(RE::MagicCaster* caster);
		static inline REL::Relocation<decltype(RequestCastImpl)> func3;

		static void StartChargeImpl(RE::MagicCaster* caster);
		static inline REL::Relocation<decltype(StartChargeImpl)> func4;

		static void StartReadyImpl(RE::MagicCaster* caster);
		static inline REL::Relocation<decltype(StartReadyImpl)> func5;

		static void StartCastImpl(RE::MagicCaster* caster);
		static inline REL::Relocation<decltype(StartCastImpl)> func6;

		static void FinishCastImpl(RE::MagicCaster* caster);
		static inline REL::Relocation<decltype(FinishCastImpl)> func7;

		static void InterruptCastImpl(RE::ActorMagicCaster* caster, bool refund);
		static inline REL::Relocation<decltype(InterruptCastImpl)> func8;

		static void InterruptCastHelper(RE::ActorMagicCaster* caster, RE::MagicSystem::CannotCastReason reason);

		static void SpellCast(RE::ActorMagicCaster* caster, bool a_doCast, std::uint32_t a_arg2, RE::MagicItem* a_spell);
		static inline REL::Relocation<decltype(SpellCast)> func9;

		static void SpellCastImpl(RE::ActorMagicCaster* caster, bool a_success, std::uint32_t a_targetCount, RE::SpellItem* a_spell);

		static bool CheckCast(RE::ActorMagicCaster* caster, RE::MagicItem* spell, bool dualCast, float* alchStrength, RE::MagicSystem::CannotCastReason* a_reason, bool useBaseValueForCost);
		static inline REL::Relocation<decltype(CheckCast)> funcA;

		static void Update(RE::ActorMagicCaster* caster, float time);
		static inline REL::Relocation<decltype(Update)> func1D;

		static void Update_Impl(RE::ActorMagicCaster* a_this, float a_delta);

		static void Install();
	};

	
	 
}


