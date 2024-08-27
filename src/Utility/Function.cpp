#include "Function.h"
#include "Utility.h"

namespace function
{
	// Function to perform the arithmetic operations
	double applyOperation(char operation, double a, double b)
	{
		double answer = 0;
		switch (operation) {
		case '+':
			answer =  a + b;
			break;
		case '-':
			answer = a - b;
			break;
		case '*':
			answer = a * b;
			break;
		case '/':
			answer = a / b;
			break;
		case '^':
			answer = pow(a, b);
			break;
		}

		logger::trace("after applying operatation {} answer is {}", operation, answer);
		return answer;
	}

	// Function to perform the unary operations like exp() and sqrt()
	double applyUnaryOperation(const std::string& operation, double a)
	{
		double answer = 0;
		if (operation == "exp")
			answer = exp(a);
		if (operation == "sqrt")
			answer = sqrt(a);
		if (operation == "abs")
			answer = abs(a);
		logger::trace("after applying operatation {} answer is {}", operation, answer);
		return answer;
	}

	//Assigns the appropriate values to the variables; currently only supports skill level and spell difficulty
	std::map<std::string, double> AssignVariables(std::vector<std::tuple<std::string, std::string, std::string>> variables, RE::MagicCaster* caster)
	{
		std::map<std::string, double> vars;
		if (caster) {
			auto costliest = caster->currentSpell->GetCostliestEffectItem()->baseEffect;
			if (costliest) {
				auto av = costliest->GetMagickSkill();
				float difficulty = costliest->GetMinimumSkillLevel();
				auto* actor = caster->GetCasterAsActor();
				float skill = 0;

				if (actor && av != RE::ActorValue::kNone) {
					skill = actor->AsActorValueOwner()->GetActorValue(av);
				}

				for (auto variable : variables) {
					float value;
					std::string name = std::get<0>(variable);

					if (std::get<1>(variable) == "difficulty") {
						value = difficulty;
						logger::trace("assigning variable {} to difficulty of {}", name, value);
					} else if (std::get<1>(variable) == "skill") {
						value = skill;
						logger::trace("assigning variable {} to skill of {}", name, value);
					} else if (std::get<1>(variable) == "global") {
						auto global = Utility::TES::GetFormFromEditorID<RE::TESGlobal>(std::get<2>(variable));
						if (global) {
							value = global->value;
							logger::trace("assigning variable {} to the global variable value of {}", name, value);
						} else {
							value = 0;
						}
					}

					vars.insert({ name, value });
				}
			}
		}

		return vars;
	}

	// Function to define the precedence of the operators (PEMDAS); right now, unary operations have low priority given that they will be handled primarily by the parentheses logic
	int precedence(std::string op)
	{
		if (op == "+" || op == "-")
			return 1;
		if (op == "*" || op == "/")
			return 2;
		if (op == "^")
			return 3;
		return 0;
	}

	//Removes all spaces from a string
	std::string removeSpaces(std::string string) {
		std::string result;
		for (char c : string) {
			if (c != ' ') {
				result += c;
			}
		}
		return result;
	}

	// Function to convert a string into a math function and evaluate it
	double evaluateExpression(const std::string& express, const std::map<std::string, double>& variables)
	{
		auto expression = removeSpaces(express); //Should remove all spaces; allows for easier parsing
		std::stack<double> values;         // stack to store values
		std::stack<std::string> ops;       // stack to store operators

		for (size_t i = 0; i < expression.length(); i++) {
			
			// If the current character is a whitespace, skip it; all spaces should have already been removed though.
			if (isspace(expression[i])) {
				continue;
			}

			// Handle numbers and negative numbers
			if (isdigit(expression[i]) || (expression[i] == '-' && (i == 0 || expression[i - 1] == '(' || ( !isdigit(expression[i - 1]) && !isalpha(expression[i-1]))))) {
				double value = 0;
				int sign = 1;

				if (expression[i] == '-') {
					sign = -1;
					i++;
				}

				bool isFloat = false;
				double factor = 1;

				// There may be more than one digit in the number
				while (i < expression.length() && (isdigit(expression[i]) || expression[i] == '.')) {
					if (expression[i] == '.') {
						isFloat = true;
					} else {
						if (isFloat) {
							factor /= 10;
							value += (expression[i] - '0') * factor;
						} else {
							value = (value * 10) + (expression[i] - '0');
						}
					}
					i++;
				}

				logger::trace("pushing value {} to value stack", (value * sign));
				values.push(value * sign);

				// The current character is not a digit, so we need to decrement i
				// because the for loop will increment it
				i--;
			} else if (expression[i] == '(') {
				ops.push(std::string(1, expression[i]));
			} else if (expression[i] == ')') {
				// Solve the entire expression inside parentheses
				while (!ops.empty() && ops.top() != "(") {
					std::string op = ops.top();
					ops.pop();

					double b = values.top();
					values.pop();
					if (op.size() > 1){
						values.push(applyUnaryOperation(op, b));
					} else {
						double a = values.top();
						char o = op[0];
						values.pop();
						values.push(applyOperation(o, a, b));
					}
				}

				if (!ops.empty())
					ops.pop();

			} else if (isalpha(expression[i])) {
				// Handling variables and unary operations exp and sqrt
				std::string name;
				while (i < expression.length() && (isalpha(expression[i]) || isdigit(expression[i]))) {
					name += expression[i];
					i++;
				}
				i--;  // Decrement to handle the last increment by the for loop
				if (name == "exp" || name == "sqrt" || name == "abs") {
					logger::trace("found unary operation {}", name);
					ops.push(name);
				} else {
					// If it's a variable, push its value to the values stack
					if (variables.find(name) != variables.end()) {
						logger::trace("found variable {} with value {}", name, variables.at(name));
						values.push(variables.at(name));
					} else {
						logger::info("Error: Undefined variable {}", name);

						return 0;
					}
				}
			} else {
				// If the current character is an operator
				while (!ops.empty() && precedence(ops.top()) >= precedence(std::string(1,expression[i]))) {
					std::string op = ops.top();
					ops.pop();

					double b = values.top();
					values.pop();

					if (values.empty()) {  // Check for unary operations	
						values.push(applyUnaryOperation(op, b));
					} else {
						double a = values.top();
						char o = op[0];
						values.pop();
						values.push(applyOperation(o, a, b));
					}
				}
				ops.push(std::string(1,expression[i]));
			}
		}

		// Entire expression has been parsed at this point, apply remaining ops to remaining values
		while (!ops.empty()) {
			logger::trace("Entire expression has been parsed. Applying reaming {} operations to {} values", ops.size(), values.size());
			double b = values.top();
			values.pop();
		
			std::string op = ops.top();
			ops.pop();

			if (values.empty()) {  // Check for unary operations
					values.push(applyUnaryOperation(op, b));
			} else {
				double a = values.top();
				char o = op[0];
				values.pop();
				values.push(applyOperation(o, a, b));
			}
		}

		// Top of 'values' contains the result, return it
		return values.top();
	}

}
