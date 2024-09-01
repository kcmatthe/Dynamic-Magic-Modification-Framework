#pragma once

namespace Spell
{
	void GetPluginData();

	void AddReplacementSpellOnCast(RE::MagicCaster* caster);
	void ReplaceSpell(RE::MagicCaster* caster);
	void ModifyMagnitude(RE::MagicCaster* caster);
}
