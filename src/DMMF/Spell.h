#pragma once

namespace Spell
{
	inline RE::MagicItem* GenericSpell = nullptr;
	inline RE::MagicItem* GenericSpell_PlayerLeft = nullptr;
	inline RE::MagicItem* GenericSpell_PlayerRight = nullptr;
	inline RE::MagicItem* GenericSpell_PlayerVoice = nullptr;
	inline RE::MagicItem* GenericSpell_NPCLeft = nullptr;
	inline RE::MagicItem* GenericSpell_NPCRight = nullptr;
	inline RE::MagicItem* GenericSpell_NPCVoice = nullptr;

	inline RE::EffectSetting* GenericEffect = nullptr;

	void GetPluginData();

	void AddReplacementSpellOnCast(RE::MagicCaster* caster);
	void ReplaceSpell(RE::MagicCaster* caster);
	void ModifyMagnitude(RE::MagicCaster* caster);
}
