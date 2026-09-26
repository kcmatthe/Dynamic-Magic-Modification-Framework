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
		
	
		//Virtual Function Hooks
		static void RequestCastImpl(RE::ActorMagicCaster* caster);
		static inline REL::Relocation<decltype(RequestCastImpl)> func3;

		static void StartChargeImpl(RE::ActorMagicCaster* caster);
		static inline REL::Relocation<decltype(StartChargeImpl)> func4;

		static void StartReadyImpl(RE::ActorMagicCaster* caster);
		static inline REL::Relocation<decltype(StartReadyImpl)> func5;

		static void StartCastImpl(RE::ActorMagicCaster* caster);
		static inline REL::Relocation<decltype(StartCastImpl)> func6;

		static void FinishCastImpl(RE::ActorMagicCaster* caster);
		static inline REL::Relocation<decltype(FinishCastImpl)> func7;

		static void InterruptCastImpl(RE::ActorMagicCaster* caster, bool refund);
		static inline REL::Relocation<decltype(InterruptCastImpl)> func8;

		static void SpellCast(RE::ActorMagicCaster* caster, bool a_doCast, std::uint32_t a_arg2, RE::MagicItem* a_spell);
		static inline REL::Relocation<decltype(SpellCast)> func9;

		static void SetCurrentSpellImpl(RE::ActorMagicCaster* caster, RE::MagicItem* spell);
		static inline REL::Relocation<decltype(SetCurrentSpellImpl)> func10;

		static void SelectSpellImpl(RE::ActorMagicCaster* caster);
		static inline REL::Relocation<decltype(SelectSpellImpl)> func11;

		static void DeselectSpellImpl(RE::ActorMagicCaster* caster);
		static inline REL::Relocation<decltype(DeselectSpellImpl)> func12;

		static bool CheckCast(RE::ActorMagicCaster* caster, RE::MagicItem* spell, bool dualCast, float* alchStrength, RE::MagicSystem::CannotCastReason* a_reason, bool useBaseValueForCost);
		static inline REL::Relocation<decltype(CheckCast)> funcA;

		static void Update(RE::ActorMagicCaster* caster, float time);
		static inline REL::Relocation<decltype(Update)> func1D;

		static void AdjustActiveEffect(RE::MagicCaster* caster, RE::ActiveEffect* effect, float power, bool onlyAdjustHostile);
		static inline REL::Relocation<decltype(AdjustActiveEffect)> func1C;

		
		//Callsite Hooks
		static void InterruptCastHelper(RE::ActorMagicCaster* caster, RE::MagicSystem::CannotCastReason reason);

	

		
		//static inline REL::Relocation<decltype(SetSpellandTimer)> _SetSpellandTimer;

		//Non-hooked functions
		static void SpellCastImpl(RE::ActorMagicCaster* caster, bool a_success, std::uint32_t a_targetCount, RE::SpellItem* a_spell);

		static void Update_Impl(RE::ActorMagicCaster* a_this, float a_delta);

		static void Install();
	};

	
	
	 
}


