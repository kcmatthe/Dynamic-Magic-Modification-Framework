#pragma once

namespace MagicNode
{
	struct NodeData
	{
		float elapsed{ 0.0f };
		int value{ 1 };
		double interval{ 0.0 };
		bool active{ false };

		RE::NiPointer<RE::NiAVObject> magicNode;
		float baseScale{ 1.0f };

		RE::NiAVObject* newArt = nullptr;
		RE::NiAVObject* oldArt = nullptr;

		bool cachedFirstPerson{ false };
	};

	static std::unordered_map<RE::MagicCaster*, NodeData> nodes;

	void SetNodeScale(RE::Actor* actor, const char* nodeName, float scale, bool firstPerson);
	void CacheMagicNode(RE::MagicCaster* caster, NodeData& cd);
	void ApplyChargeScale(NodeData& cd);
	void ResetChargeScale(NodeData& cd);
	bool IsFirstPerson();

	void UpdateNode(RE::MagicCaster* caster, RE::NiAVObject* newArt);

	void UpdateCastingArt(RE::ActorMagicCaster* caster, RE::MagicItem* spell);
	void UpdateSpellSounds(RE::ActorMagicCaster* caster, RE::MagicItem* spell);
	void UpdateSpellVisualsAndSounds(RE::ActorMagicCaster* caster, RE::MagicItem* spell);
}
