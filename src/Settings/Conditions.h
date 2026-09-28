#include <algorithm>
#include <cctype>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

#undef CompareString
namespace Conditions
{
	struct Condition
	{
	public:
		std::string variable;
		std::string op;
		std::variant<std::monostate, std::string, int, float, bool> value;
		std::string variableDetail;

		Condition(std::string var,
			std::string o,
			std::variant<std::monostate, std::string, int, float, bool> v,
			std::string vd = "") :
			variable(std::move(var)),
			op(std::move(o)),
			value(std::move(v)),
			variableDetail(std::move(vd))
		{}

		Condition() = default;
	};

	namespace detail
	{
		inline std::string ToLower(std::string_view str)
		{
			std::string out;
			out.reserve(str.size());

			for (char c : str) {
				out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
			}
			return out;
		}

		inline bool IsEqualityOperator(std::string_view op)
		{
			return op == "==" || op == "!=";
		}

		inline bool CompareBool(bool lhs, std::string_view op, bool rhs)
		{
			if (op == "==") {
				return lhs == rhs;
			}
			if (op == "!=") {
				return lhs != rhs;
			}
			return false;
		}

		inline bool CompareString(std::string_view lhs, std::string_view op, std::string_view rhs)
		{
			if (op == "==") {
				return lhs == rhs;
			}
			if (op == "!=") {
				return lhs != rhs;
			}
			return false;
		}

		inline bool CompareNumeric(float lhs, std::string_view op, float rhs)
		{
			if (op == "==") {
				return lhs == rhs;
			}
			if (op == "!=") {
				return lhs != rhs;
			}
			if (op == "<") {
				return lhs < rhs;
			}
			if (op == ">") {
				return lhs > rhs;
			}
			if (op == "<=") {
				return lhs <= rhs;
			}
			if (op == ">=") {
				return lhs >= rhs;
			}
			return false;
		}

		inline std::optional<std::string> GetStringValue(const Condition& condition)
		{
			if (const auto* value = std::get_if<std::string>(&condition.value)) {
				return ToLower(*value);
			}
			return std::nullopt;
		}

		inline std::optional<float> GetFloatValue(const Condition& condition)
		{
			if (const auto* value = std::get_if<float>(&condition.value)) {
				return *value;
			}
			if (const auto* value = std::get_if<int>(&condition.value)) {
				return static_cast<float>(*value);
			}
			return std::nullopt;
		}

		inline std::optional<bool> GetBoolValue(const Condition& condition)
		{
			if (const auto* value = std::get_if<bool>(&condition.value)) {
				return *value;
			}
			return std::nullopt;
		}

		inline std::optional<int> GetIntValue(const Condition& condition)
		{
			// Direct int
			if (const auto* value = std::get_if<int>(&condition.value)) {
				return *value;
			}

			// Float -> int 
			if (const auto* value = std::get_if<float>(&condition.value)) {
				return static_cast<int>(*value);
			}
			return std::nullopt;
		}

		inline RE::Actor* GetCasterActor(RE::MagicCaster* caster)
		{
			if (!caster) {
				return nullptr;
			}

			return caster->GetCasterAsActor();
		}

		inline RE::MagicItem* GetCurrentMagicItem(RE::MagicCaster* caster)
		{
			if (!caster) {
				return nullptr;
			}

			return caster->currentSpell;
		}

		inline std::string GetMagicTypeString(RE::MagicItem* magicItem)
		{
			if (!magicItem) {
				return "";
			}

			switch (magicItem->GetSpellType()) {
			case RE::MagicSystem::SpellType::kSpell:
				return "spell";
			case RE::MagicSystem::SpellType::kStaffEnchantment:
				return "staff";
			case RE::MagicSystem::SpellType::kScroll:
				return "scroll";
			default:
				return "";
			}
		}

		inline std::string GetCastingTypeString(RE::MagicItem* magicItem)
		{
			if (!magicItem) {
				return "";
			}

			switch (magicItem->GetCastingType()) {
			case RE::MagicSystem::CastingType::kFireAndForget:
				return "fireforget";
			case RE::MagicSystem::CastingType::kConcentration:
				return "concentration";
			default:
				return "";
			}
		}

		inline std::string GetSchoolString(RE::MagicItem* magicItem)
		{
			if (!magicItem) {
				return "";
			}

			switch (magicItem->GetAssociatedSkill()) {
			case RE::ActorValue::kAlteration:
				return "alteration";
			case RE::ActorValue::kConjuration:
				return "conjuration";
			case RE::ActorValue::kDestruction:
				return "destruction";
			case RE::ActorValue::kIllusion:
				return "illusion";
			case RE::ActorValue::kRestoration:
				return "restoration";
			default:
				return "";
			}
		}

		inline std::string GetHandString(RE::MagicCaster* caster)
		{
			if (!caster) {
				return "";
			}

			switch (caster->GetCastingSource()) {
			case RE::MagicSystem::CastingSource::kLeftHand:
				return "left";
			case RE::MagicSystem::CastingSource::kRightHand:
				return "right";
			default:
				return "";
			}
		}

	}


	bool EvaluateCondition(const Condition& condition, RE::MagicCaster* caster);
	
	template <typename T>
	bool EvaluateConditionsList(const T* modifier, RE::MagicCaster* caster)
	{
		bool conditionsMet = true;

		if (!modifier) {
			logger::warn("Attempted to evaluate conditions on a null modifier");
			return false;
		}

		if (!caster) {
			logger::warn("Attempted to evaluate conditions on a null caster");
			return false;
		}

		if (modifier->condOp == "or") {
			logger::trace("The comparative operator was 'or'");

			conditionsMet = false;

			for (const Condition& condition : modifier->conditions) {
				const bool tempBool = EvaluateCondition(condition, caster);
				logger::trace("Individual condition check was {}", tempBool);

				if (tempBool) {
					conditionsMet = true;
					break;
				}
			}
		} else if (modifier->condOp == "and") {
			logger::trace("The comparative operator was 'and'");

			conditionsMet = true;

			for (const Condition& condition : modifier->conditions) {
				const bool tempBool = EvaluateCondition(condition, caster);
				logger::trace("Individual condition check was {}", tempBool);

				if (!tempBool) {
					conditionsMet = false;
					break;
				}
			}
		} else {
			logger::warn("Unknown comparative operator '{}'", modifier->condOp);
			conditionsMet = false;
		}

		return conditionsMet;
	}
}
