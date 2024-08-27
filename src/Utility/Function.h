#pragma once

#include <iostream>
#include <string>
#include <stack>
#include <cctype>
#include <cmath>
#include <map>
#include <sstream>



namespace function
{
	double applyOperation(char operation, double a, double b);

	double applyUnaryOperation(const std::string& operation, double a);

	std::map<std::string, double> AssignVariables(std::vector<std::tuple<std::string, std::string, std::string>> variables, RE::MagicCaster* caster);

	double evaluateExpression(const std::string& expression, const std::map<std::string, double>& variables);
}
