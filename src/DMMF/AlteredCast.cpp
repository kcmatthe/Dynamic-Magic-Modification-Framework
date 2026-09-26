#include "AlteredCast.h"

namespace Cast
{
	namespace
	{
		std::mutex castMutex;
		std::unordered_map<RE::MagicCaster*, AlteredCast> casts;
	}

	AlteredCast& GetCastInstance(RE::MagicCaster* caster)
	{
		if (!caster) {
			logger::warn("GetCastInstance called with null caster ");
		}

		std::scoped_lock lock(castMutex);

		auto it = casts.find(caster);
		if (it != casts.end()) {
			logger::trace("Found existing cast instance");
			return it->second;
		}

		logger::debug("Creating AlteredCast for MagicCaster {:p}", static_cast<void*>(caster));
		auto [insertedIt, inserted] = casts.emplace(caster, AlteredCast{ caster });
		return insertedIt->second;
	}

	AlteredCast* FindCastInstance(RE::MagicCaster* caster)
	{
		if (!caster) {
			return nullptr;
		}

		std::scoped_lock lock(castMutex);

		auto it = casts.find(caster);
		if (it == casts.end()) {
			return nullptr;
		}
		return &it->second;
	}

	void ResetCastInstance(RE::MagicCaster* caster)
	{
		if (!caster) {
			return;
		}

		std::scoped_lock lock(castMutex);

		auto it = casts.find(caster);
		if (it != casts.end()) {
			it->second.ResetForNewCast();
		}
	}

	void EraseCastInstance(RE::MagicCaster* caster)
	{
		if (!caster) {
			return;
		}

		std::scoped_lock lock(castMutex);

		casts.erase(caster);
	}
}
