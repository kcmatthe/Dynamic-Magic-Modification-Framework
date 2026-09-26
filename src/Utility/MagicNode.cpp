#include "MagicNode.h"

namespace MagicNode
{
	void SetNodeScale(
		RE::Actor* actor,
		const char* nodeName,
		float scale,
		bool firstPerson)
	{
		if (!actor) {
			return;
		}

		// Papyrus "wiseman" = PlayerCharacter
		auto root = actor->Get3D(firstPerson);
		if (!root) {
			return;
		}

		auto node = root->GetObjectByName(nodeName);
		if (!node) {
			return;
		}

		node->local.scale = scale;
		RE::NiUpdateData updateData;
		node->Update(updateData);
	}

	bool IsFirstPerson()
	{
		auto camera = RE::PlayerCamera::GetSingleton();
		if (!camera) {
			return false;
		}

		return camera->IsInFirstPerson();
	}

	void CacheMagicNode(RE::MagicCaster* caster, NodeData& cd)
	{
		if (!caster) {
			return;
		}
		auto actor = caster->GetCasterAsActor();
		if (!actor) {
			return;
		}

		const bool rightHand =
			caster->GetCastingSource() == RE::MagicSystem::CastingSource::kRightHand;

		const char* nodeName =
			rightHand ? "NPC R MagicNode [RMag]" : "NPC L MagicNode [LMag]";

		bool firstPerson = false;
		if (actor == RE::PlayerCharacter::GetSingleton()) {
			firstPerson = IsFirstPerson();
		}

		auto root = actor->Get3D(firstPerson);
		if (!root) {
			return;
		}

		auto node = root->GetObjectByName(nodeName);
		if (!node) {
			return;
		}

		cd.magicNode = RE::NiPointer<RE::NiAVObject>(node);
		cd.baseScale = node->local.scale;
		cd.oldArt = caster->currentSpell->GetCostliestEffectItem()->baseEffect->data.castingArt->As<RE::NiAVObject>();
	}

	void UpdateNode(RE::MagicCaster* caster, RE::NiAVObject* newArt) {

		logger::info("Updating node");
		if (!caster) {
			return;
		}
		auto actor = caster->GetCasterAsActor();
		if (!actor) {
			return;
		}

		const bool rightHand =
			caster->GetCastingSource() == RE::MagicSystem::CastingSource::kRightHand;

		const char* nodeName =
			rightHand ? "NPC R MagicNode [RMag]" : "NPC L MagicNode [LMag]";

		bool firstPerson = false;
		if (actor == RE::PlayerCharacter::GetSingleton()) {
			firstPerson = IsFirstPerson();
		}

		auto root = actor->Get3D(firstPerson);
		if (!root) {
			return;
		}

		auto node = root->GetObjectByName(nodeName);
		if (!node) {
			return;
		}
		logger::info("Attaching new child");
		node->AsNode()->AttachChild(newArt);
		RE::NiUpdateData updateData;
		node->Update(updateData);
	}

	void ApplyChargeScale(NodeData& cd)
	{
		if (!cd.magicNode) {
			return;
		}

		float scaleMult = 1.0f + 0.2f * (cd.value - 1);
		if (scaleMult > 3) {
			scaleMult = 3;
		}
		if (scaleMult < 0.25) {
			scaleMult = 0.25;
		}
		cd.magicNode->local.scale = cd.baseScale * scaleMult;
		RE::NiUpdateData updateData;
		cd.magicNode->Update(updateData);
	}

	void ResetChargeScale(NodeData& cd)
	{
		if (!cd.magicNode) {
			return;
		}

		cd.magicNode->local.scale = cd.baseScale;
		RE::NiUpdateData updateData;
		cd.magicNode->Update(updateData);
	}

	void UpdateSpellVisualsAndSounds(RE::ActorMagicCaster* caster, RE::MagicItem* spell)
	{
		UpdateSpellSounds(caster, spell);

		UpdateCastingArt(caster, spell);
	}

	void UpdateCastingArt(RE::ActorMagicCaster* caster, RE::MagicItem* spell) {
		logger::debug("UpdateCastingArt Called");

		int slot = static_cast<int>(caster->castingSource);
		auto cachedSpell = caster->GetCasterAsActor()->GetActorRuntimeData().selectedSpells[slot];
		
		if (spell && spell->GetCostliestEffectItem() && spell->GetCostliestEffectItem()->baseEffect) {
			auto art = spell->GetCostliestEffectItem()->baseEffect->data.castingArt;
			//caster->GetCasterAsActor()->GetActorRuntimeData().selectedSpells[slot] = spell; //this doesn't actually seem to be needed
		
			//caster->ResetNodes(); //this causes issues; casting art just disappears entirely
			caster->RemoveCastingArt(false); //was true; doesn't seem to make a difference
			
			if (art)
				caster->ApplyCastingArt(art);
		
			//caster->GetCasterAsActor()->GetActorRuntimeData().selectedSpells[slot] = cachedSpell; /
		}
	}

	void UpdateSpellSounds(RE::ActorMagicCaster* caster, RE::MagicItem* spell) {

		logger::debug("Resetting sounds to normal");

		for (int i = 0; i <= 4; i++) {
			auto sound = static_cast<RE::MagicSystem::SoundID>(i);
			caster->PrepareSound(sound, spell);
		}
	}
}
