#pragma once


namespace GameData
{
	RE::BSTArray<RE::ScrollItem*> getScrolls();
	RE::BSTArray<RE::TESObjectBOOK*> getSpellBooks();
	RE::BSTArray<RE::TESObjectWEAP*> getStaves();
	RE::BSTArray<RE::SpellItem*> getSpells();
	RE::BSTArray<RE::IngredientItem*> getIngredients();
	RE::BSTArray<RE::EnchantmentItem*> getEnchantments();

	bool hasEffect(RE::TESForm* form, RE::EffectSetting* akEffect);
	bool hasEffectSpell(RE::SpellItem* spell, RE::EffectSetting* akEffect);
	bool hasEffectScroll(RE::ScrollItem* scroll, RE::EffectSetting* akEffect);
	bool hasEffectEnchantment(RE::EnchantmentItem* enchant, RE::EffectSetting* akEffect);
}
