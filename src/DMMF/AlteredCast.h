#pragma once

namespace Cast
{
	struct AlteredCharge
	{
	public:
		RE::MagicCaster* caster;

		float newBaseTime;
		float updatedTime;
		bool excluded;
		bool override;
		float overrideValue;
		std::vector<float> multipliers;
		std::vector<float> modifiers;

		AlteredCharge(RE::MagicCaster* c, float time, bool o, bool e, float val, std::vector<float> mult, std::vector<float> mod)
		{
			caster = c;
			newBaseTime = time;
			override = o;
			excluded = e;
			overrideValue = val;

			multipliers = mult;
			modifiers = mod;
		}

		AlteredCharge() {
			caster = nullptr;
			newBaseTime = -1;
			excluded = false;
			override = false;
			overrideValue = -1;
			multipliers = {};
			modifiers = {};
		}
	};
	struct AlteredCost
	{
	public:
		RE::MagicCaster* caster;

		float newBaseCost;
		float updatedCost;
		bool excluded;
		bool override;
		float overrideValue;
		std::vector<float> multipliers;
		std::vector<float> modifiers;

		AlteredCost(RE::MagicCaster* c, float cost, bool o, bool e, float val, std::vector<float> mult, std::vector<float> mod)
		{
			caster = c;
			override = o;
			excluded = e;
			overrideValue = val;

			multipliers = mult;
			modifiers = mod;
		}
		
		AlteredCost()
		{
			caster = nullptr;
			newBaseCost = -1;
			excluded = false;
			override = false;
			overrideValue = -1;
			multipliers = {};
			modifiers = {};
		}
	};
	struct AlteredMagnitude
	{
	public:
		RE::MagicCaster* caster;

		float newBaseMag;
		float updatedMag;
		bool excluded;
		bool override;
		float overrideValue;
		bool costliestOnly;
		std::vector<float> multipliers;
		std::vector<float> modifiers;

		AlteredMagnitude(RE::MagicCaster* c, float mag, bool o, bool e, float val, bool costliest, std::vector<float> mult, std::vector<float> mod)
		{
			caster = c;
			override = o;
			excluded = e;
			overrideValue = val;
			costliestOnly = costliest;

			multipliers = mult;
			modifiers = mod;
		}

		AlteredMagnitude()
		{
			caster = nullptr;
			newBaseMag = -1;
			excluded = false;
			override = false;
			costliestOnly = true;
			overrideValue = -1;
			multipliers = {};
			modifiers = {};
		}
	};
	struct AlteredSpell
	{
	public:
		RE::MagicItem* modified;
		RE::MagicItem* original;

		AlteredSpell(RE::MagicItem* m, RE::MagicItem* o)
		{
			modified = m;
			original = o;
		}

		AlteredSpell()
		{
			modified = nullptr;
			original = nullptr;
		}
	};
	struct AlteredCast 
	{
	public:
		RE::MagicCaster* caster;
		AlteredCharge* charge;
		AlteredCost* cost;
		AlteredMagnitude* magnitude;
		RE::ActorValue resource;
		AlteredSpell* spell;
		

		AlteredCast(RE::MagicCaster* c, AlteredCharge* ct, AlteredCost* cc, AlteredMagnitude* mag, RE::ActorValue res = RE::ActorValue::kMagicka, AlteredSpell* repl = nullptr)
		{
			caster = c;
			charge = ct;
			cost = cc;
			magnitude = mag;
			resource = res;
			spell = repl;
		}
	};
	
	

	std::vector<AlteredCast>::iterator GetCastInstance(RE::MagicCaster* caster);

	inline std::vector<AlteredCast> casts = {};
}
