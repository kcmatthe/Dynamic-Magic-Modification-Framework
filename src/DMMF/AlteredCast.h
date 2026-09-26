#pragma once

namespace Cast
{
	struct AlteredCharge
	{
	public:

		RE::MagicCaster* caster{ nullptr };

		float newBaseTime{ -1.0f };
		float updatedTime{ -1.0f };
		bool excluded{ false };
		bool override{ false };
		float overrideValue{ -1.0f };
		std::vector<float> multipliers;
		std::vector<float> modifiers;

		AlteredCharge() = default;

		AlteredCharge(
			RE::MagicCaster* c,
			float time,
			bool o,
			bool e,
			float val,
			std::vector<float> mult,
			std::vector<float> mod) :
			caster(c),
			newBaseTime(time),
			updatedTime(time),
			excluded(e),
			override(o),
			overrideValue(val),
			multipliers(std::move(mult)),
			modifiers(std::move(mod))
		{}
	};
	struct AlteredCost
	{
	public:

		RE::MagicCaster* caster{ nullptr };

		float newBaseCost{ -1.0f };
		float updatedCost{ -1.0f };
		bool excluded{ false };
		bool override{ false };
		float overrideValue{ -1.0f };
		std::vector<float> multipliers;
		std::vector<float> modifiers;

		AlteredCost() = default;

		AlteredCost(
			RE::MagicCaster* c,
			float cost,
			bool o,
			bool e,
			float val,
			std::vector<float> mult,
			std::vector<float> mod) :
			caster(c),
			newBaseCost(cost),
			updatedCost(cost),
			excluded(e),
			override(o),
			overrideValue(val),
			multipliers(std::move(mult)),
			modifiers(std::move(mod))
		{}
	};
	struct AlteredMagnitude
	{
	public:
		RE::MagicCaster* caster{ nullptr };

		float newBaseMag{ -1.0f };
		float updatedMag{ -1.0f };
		bool excluded{ false };
		bool override{ false };
		float overrideValue{ -1.0f };
		bool costliestOnly{ true };
		std::vector<float> multipliers;
		std::vector<float> modifiers;

		AlteredMagnitude() = default;

		AlteredMagnitude(
			RE::MagicCaster* c,
			float mag,
			bool o,
			bool e,
			float val,
			bool costliest,
			std::vector<float> mult,
			std::vector<float> mod) :
			caster(c),
			newBaseMag(mag),
			updatedMag(mag),
			excluded(e),
			override(o),
			overrideValue(val),
			costliestOnly(costliest),
			multipliers(std::move(mult)),
			modifiers(std::move(mod))
		{}
	};
	struct AlteredSpell
	{
	public:
		RE::MagicItem* modified{ nullptr };
		RE::MagicItem* original{ nullptr };

		AlteredSpell() = default;

		AlteredSpell(RE::MagicItem* m, RE::MagicItem* o) :
			modified(m),
			original(o)
		{}
	};

	struct AlteredDuration
	{
	public:
		RE::MagicCaster* caster{ nullptr };

		float newBaseDur{ -1.0f };
		float updatedDur{ -1.0f };
		bool excluded{ false };
		bool override{ false };
		float overrideValue{ -1.0f };
		bool costliestOnly{ true };
		std::vector<float> multipliers;
		std::vector<float> modifiers;

		AlteredDuration() = default;

		AlteredDuration(
			RE::MagicCaster* c,
			float dur,
			bool o,
			bool e,
			float val,
			bool costliest,
			std::vector<float> mult,
			std::vector<float> mod) :
			caster(c),
			newBaseDur(dur),
			updatedDur(dur),
			excluded(e),
			override(o),
			overrideValue(val),
			costliestOnly(costliest),
			multipliers(std::move(mult)),
			modifiers(std::move(mod))
		{}
	};

	struct AlteredCast 
	{
	public:
		RE::MagicCaster* caster{ nullptr };
		RE::MagicItem* spellItem{ nullptr };

		std::unique_ptr<AlteredCharge> charge;
		std::unique_ptr<AlteredCost> cost;
		std::unique_ptr<AlteredMagnitude> magnitude;
		std::unique_ptr<AlteredDuration> duration;
		RE::ActorValue resource{ RE::ActorValue::kMagicka };
		RE::ActorValue secondaryResource{ RE::ActorValue::kNone };
		float secondaryMult = 1;
		std::unique_ptr<AlteredSpell> spell;
		bool fired{ false };

		AlteredCast() = default;

		explicit AlteredCast(RE::MagicCaster* c) :
			caster(c)
		{}

		// Called when starting a *new* cast for this caster.
		void ResetForNewCast()
		{
			charge.reset();
			cost.reset();
			magnitude.reset();
			duration.reset();
			resource = RE::ActorValue::kMagicka;
			secondaryResource = RE::ActorValue::kNone;
			secondaryMult = 1;
			spell.reset();
			fired = false;
			spellItem = nullptr;
		}
	};
	
	// Thread-safe hopefully
	AlteredCast& GetCastInstance(RE::MagicCaster* caster);
	AlteredCast* FindCastInstance(RE::MagicCaster* caster);
	void ResetCastInstance(RE::MagicCaster* caster);
	void EraseCastInstance(RE::MagicCaster* caster);
}
