#pragma once

#include "GameData.h"

namespace GameData
{
	RE::BSTArray<RE::ScrollItem*> getScrolls()
	{
		auto& scrolls = RE::TESDataHandler::GetSingleton()->GetFormArray<RE::ScrollItem>();
		logger::info("Returning {} scrolls.", scrolls.size());
		return scrolls;
	}

	RE::BSTArray<RE::TESObjectBOOK*> getSpellBooks()
	{
		auto& books = RE::TESDataHandler::GetSingleton()->GetFormArray<RE::TESObjectBOOK>();
		logger::info("Getting {} books.", books.size());
		RE::BSTArray<RE::TESObjectBOOK*> spellBooks;
		for (auto* book : books) {
			if (book->TeachesSpell() && book != RE::TESForm::LookupByEditorID("MGRDestructionFinal")->As<RE::TESObjectBOOK>() ){
				spellBooks.emplace_back(book);
			}
		}
		logger::info("Returning {} spell tomes.", spellBooks.size());
		return spellBooks;
	}

	RE::BSTArray<RE::IngredientItem*> getIngredients() {
		
		auto& ingredients = RE::TESDataHandler::GetSingleton()->GetFormArray<RE::IngredientItem>();
		logger::info("Getting {} ingredients.", ingredients.size());
	}

	RE::BSTArray<RE::EnchantmentItem*> getEnchantments()
	{
		auto& enchantments = RE::TESDataHandler::GetSingleton()->GetFormArray<RE::EnchantmentItem>();
		logger::info("Getting {} ingredients.", enchantments.size());
	}

	RE::BSTArray<RE::SpellItem*> getSpells() {
		
		auto spellBooks = getSpellBooks();
		logger::info("Getting {} spells from spell books", spellBooks.size());
		RE::BSTArray<RE::SpellItem*> spells;
		for (auto* book : spellBooks) {
			spells.emplace_back(book->GetSpell());
		}
		logger::info("Returning {} spells.", spells.size());
		return spells;
	}

	RE::BSTArray<RE::TESObjectWEAP*> getStaves()
	{
		auto& weapons = RE::TESDataHandler::GetSingleton()->GetFormArray<RE::TESObjectWEAP>();
		logger::info("Getting {} weapons.", weapons.size());
		RE::BSTArray<RE::TESObjectWEAP*> staves;
		for (auto* weapon : weapons) {
			if (weapon->IsStaff()) {
				staves.emplace_back(weapon);
			}
		}
		logger::info("Returning {} staves.", staves.size());
		return staves;
	}

	bool hasEffect(RE::TESForm* form, RE::EffectSetting* akEffect)
	{
		if (form->Is(RE::FormType::Spell)) {
			auto* spell = static_cast<RE::SpellItem*>(form);
			auto& effects = spell->effects;
			logger::info("Comparing {} to {} effect(s) of spell {}", akEffect->fullName.c_str(), effects.size(), spell->fullName.c_str());
			for (auto& effect : effects) {
				if (effect->baseEffect == akEffect) {
					return true;
				}
			}
		}
		if (form->Is(RE::FormType::Scroll)) {
			auto* scroll = static_cast<RE::ScrollItem*>(form);
			auto& effects = scroll->effects;
			logger::info("Comparing {} to {} effect(s) of scroll {}", akEffect->fullName.c_str(), effects.size(), scroll->fullName.c_str());
			for (auto& effect : effects) {
				if (effect->baseEffect == akEffect) {
					return true;
				}
			}
		}
		if (form->Is(RE::FormType::Enchantment)) {
			auto* enchant = static_cast<RE::EnchantmentItem*>(form);
			auto& effects = enchant->effects;
			logger::info("Comparing {} to {} effect(s) of enchantment {}", akEffect->fullName.c_str(), effects.size(), enchant->fullName.c_str());
			for (auto& effect : effects) {
				if (effect->baseEffect == akEffect) {
					return true;
				}
			}
		}
		return false;
	}

	bool hasEffectSpell(RE::SpellItem* spell, RE::EffectSetting* akEffect)
	{
		auto& effects = spell->effects;
		logger::info("Comparing {} to {} effect(s) of spell {}", akEffect->fullName.c_str(), effects.size(), spell->fullName.c_str());
		for (auto& effect : effects) {
			if (effect->baseEffect == akEffect) {
				return true;
			}
		}
		return false;
	}

	bool hasEffectScroll(RE::ScrollItem* scroll, RE::EffectSetting* akEffect)
	{
		auto& effects = scroll->effects;
		logger::info("Comparing {} to {} effect(s) of scroll {}", akEffect->fullName.c_str(), effects.size(), scroll->fullName.c_str());
		for (auto& effect : effects) {
			if (effect->baseEffect == akEffect) {
				return true;
			}
		}
		return false;
	}

	bool hasEffectEnchantment(RE::EnchantmentItem* enchant, RE::EffectSetting* akEffect)
	{
		auto& effects = enchant->effects;
		logger::info("Comparing {} to {} effect(s) of enchantment {}", akEffect->fullName.c_str(), effects.size(), enchant->fullName.c_str());
		for (auto& effect : effects) {
			if (effect->baseEffect == akEffect) {
				return true;
			}
		}
		return false;
	}
}
