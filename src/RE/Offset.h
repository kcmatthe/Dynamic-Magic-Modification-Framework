#pragma once

#include "REL/Relocation.h" //maybe helps?

namespace RE
{
	namespace Offset
	{
		namespace ActiveEffect
		{
			constexpr auto AdjustForPerks = REL::ID(34053);
			constexpr auto DoStandardCustomSkillUsage = REL::ID(34100);
		}

		namespace ActiveEffectFactory
		{
			constexpr auto CheckTargetLevelMagnitude = REL::ID(34048);
		}

	}
}
