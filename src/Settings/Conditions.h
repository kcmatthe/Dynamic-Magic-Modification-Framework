#pragma once

namespace Conditions
{
	struct Condition
	{
	public:
		std::string variable;
		std::string op;
		std::variant<std::monostate, RE::ActorValue, RE::MagicSystem::SpellType, RE::MagicSystem::CastingType, RE::MagicItem*, RE::Actor*, RE::BGSPerk*, RE::MagicSystem::CastingSource, std::string, float> value;
		std::string variableDetail;

		Condition(std::string var, std::string o, std::variant<std::monostate, RE::ActorValue, RE::MagicSystem::SpellType, RE::MagicSystem::CastingType, RE::MagicItem*, RE::Actor*, RE::BGSPerk*, RE::MagicSystem::CastingSource, std::string, float> v, std::string vd = "")
		{
			variable = var;
			op = o;
			value = v;
			variableDetail = vd;
		}

		Condition()
		{
			variable = "";
			op = "";
			value = std::monostate();
			variableDetail = "";
		}
	};

	bool CheckCondition(Condition condition, RE::MagicCaster* caster);
	std::variant<std::monostate, RE::ActorValue, RE::MagicSystem::SpellType, RE::MagicSystem::CastingType, RE::MagicItem*, RE::Actor*, RE::BGSPerk*, RE::MagicSystem::CastingSource, std::string, float> AssignVariable(std::string variable, std::variant<std::string, float> value);
}
