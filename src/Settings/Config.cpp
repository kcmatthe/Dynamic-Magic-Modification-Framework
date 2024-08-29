#pragma once

#include "config.h"
#include "Utility/Utility.h"
#include <ClibUtil/distribution.hpp>
#include <string>

#include "Conditions.h"

namespace config
{

	using namespace Conditions;

	bool GetOverrides(rapidjson::Document& config, std::string configLocation)
	{
		if (config.HasMember("Overrides")) {
			const rapidjson::Value& overrides = config["Overrides"];
			if (overrides.IsArray()) {
				logger::info("About to iterate through all {} overrides in the json", overrides.Size());
				for (int i = 0; i < overrides.Size(); i++) {
					//Variables for overrides
					bool exclude = false;        //whether or not to exclude
					bool bOverride = false;      //whether or not to override
					float value = -1;			 //override value
					const char* parameter = "";  //parameter that is being overriden
					const char* condOp = "and";  //comparative operator
					float numCond = 0;           //number of conditions

					Override* override = new Override();

					if (overrides[i].HasMember("excluded") && overrides[i]["excluded"].IsBool()) {
						exclude = (overrides[i]["excluded"].GetBool());
						override->excluded = exclude;
					} else {
						exclude = false;
						logger::debug("No valid exclusion for spell index {}. Exclude set to false", i);
						override->excluded = exclude;
					}
	
					if (overrides[i].HasMember("value") && overrides[i]["value"].IsFloat()) {
						value = overrides[i]["value"].GetFloat();
						bOverride = true;
						override->override = bOverride;
						override->value = value;
					} else {
						logger::info("No override value provided, if exclusion is false, this entry will be ignored.");
						bOverride = false;
						override->override = bOverride;
					}
					
					if (overrides[i].HasMember("conditions") && overrides[i]["conditions"].IsArray()) {
						const rapidjson::Value& conditions = overrides[i]["conditions"];
						numCond = conditions.Size();
						logger::debug("About to iterate through all {} conditions in the override", numCond);
						for (int j = 0; j < numCond; j++) {
							const char* variable = "";
							const char* variableDetail = "";
							const char* op = "";
							std::variant<std::string, float, bool> condValue;

							if (conditions[j].HasMember("condition")) {
								if (conditions[j]["condition"].IsString()) {
									variable = conditions[j]["condition"].GetString();
									auto variable_s = std::string(variable);
									if (variable_s == "global" || variable_s == "Global") {
										if (conditions[j].HasMember("conditionDetail")) {
											if (conditions[j]["conditionDetail"].IsString()) {
												variableDetail = conditions[j]["conditionDetail"].GetString();
											}
										}
									}
								} else {
									variable = "";
									logger::info("No valid variable");
								}

								if (conditions[j].HasMember("operator") && conditions[j]["operator"].IsString()) {
									op = conditions[j]["operator"].GetString();

								} else {
									op = "";
									logger::info("No valid operator");
								}

								if (conditions[j].HasMember("value")) {
									if (conditions[j]["value"].IsString()) {
										condValue = overrides[i]["conditions"][j]["value"].GetString();
									} else if (conditions[j]["value"].IsFloat()) {
										condValue = overrides[i]["conditions"][j]["value"].GetFloat();
									} else if (conditions[j]["value"].IsBool()) {
										condValue = overrides[i]["conditions"][j]["value"].GetBool();
									} else {
										logger::info("No valid condition value");
									}
								} else {
									logger::info("No valid condition value");
								}

								if (!std::holds_alternative<std::string>(condValue) || (std::holds_alternative<std::string>(condValue) && *std::get_if<std::string>(&condValue) != "")) {
									auto assignedVariable = AssignVariable(variable, condValue);
									auto newCondition = Condition(variable, op, assignedVariable, variableDetail);
									override->conditions.push_back(newCondition);
									if (std::holds_alternative<std::string>(condValue)) {
										auto cvString = *std::get_if<std::string>(&condValue);
										logger::info("Pushed back a new condition: {} {} {}", variable, op, cvString);
									} else if (std::holds_alternative<std::string>(condValue)) {
										auto cvFloat = *std::get_if<float>(&condValue);
										logger::info("Pushed back a new condition: {} {} {}", variable, op, cvFloat);
									} else if (std::holds_alternative<std::string>(condValue)) {
										auto cvBool = *std::get_if<bool>(&condValue);
										logger::info("Pushed back a new condition: {} {} {}", variable, op, cvBool);
									}
								}
								
							} else {
								logger::info("No valid condition");
							}
						}

					} else {
						logger::info("There are no conditions for this override.");
						override->conditions = {};
					}
						
						
					if (overrides[i].HasMember("conditionsOperator") && overrides[i]["conditionsOperator"].IsString()) {
						condOp = overrides[i]["conditionsOperator"].GetString();
					} else {
						condOp = "and";
					}
					override->condOp = condOp;

					if (overrides[i].HasMember("parameter") && overrides[i]["parameter"].IsString()) {
						parameter = overrides[i]["parameter"].GetString();
						auto parameter_s = std::string(parameter);
						if (parameter_s == "time" || parameter_s == "Time") {
							ctOverrides.push_back(override);
						} else if (parameter_s == "Cost" || parameter_s == "cost") {
							cOverrides.push_back(override);
						} else if (parameter_s == "Magnitude" || parameter_s == "magnitude") {
							mOverrides.push_back(override);
						} else {
							logger::info("No valid parameter.");
						}
						logger::info("Read override for parameter: {} exclude: {}, override: {}, value: {}", parameter_s, exclude, bOverride, value);
					} else {
						logger::info("No valid parameter.");
					}
					
				}
			} else {
				logger::info("{} did not contain a valid array: overrides", configLocation);
			}
		} else {
			logger::info("{} did not contain a valid object: overrides", configLocation);
		}
		return true;
	}

	bool GetMultipliers(rapidjson::Document& config, std::string configLocation) {
		if (config.HasMember("Multipliers")) {
			const rapidjson::Value& multipliers = config["Multipliers"];

			if (multipliers.IsArray()) {
				logger::info("About to iterate through all {} multipliers in the json", multipliers.Size());
				for (int i = 0; i < multipliers.Size(); i++) {
					Multiplier* newMultiplier = new Multiplier();     //New Multiplier struct with regular float values
					MultiplierF* newMultiplierF = new MultiplierF();  //New Multiplier struct with functions

					//variables for multipler features
					const char* condOp = "and";   //comparative operator
					float numCond = 0;            //number of conditions
					const char* valueType = "";   //type of multiplier value (float or function)
					const char* parameter = "";   //what is being multiplied (i.e. charge time)
					const char* function = "";    //function string
					const char* varName = "";     //variable alias (i.e. x,y,z or a,b,c)
					const char* varSource = "";  //variable meaning (i.e. skill level or spell difficulty)
					const char* sourceDef = "";
					float mult = -1;              //multiplier value

					bool isFunction = false;
					if (multipliers[i].HasMember("valueType") && multipliers[i]["valueType"].IsString()) {
						valueType = multipliers[i]["valueType"].GetString();
						auto valueType_s = std::string(valueType);

						if (valueType_s == "float" || valueType_s == "function") {
							if (valueType_s == "float") {
								if (multipliers[i].HasMember("value") && multipliers[i]["value"].IsFloat()) {
									mult = multipliers[i]["value"].GetFloat();
									isFunction = false;
									newMultiplier->value = mult;
								}
							} else if (valueType_s == "function") {
								if (multipliers[i].HasMember("function") && multipliers[i]["function"].IsString()) {
									function = multipliers[i]["function"].GetString();
									isFunction = true;
									newMultiplierF->function.function = function;

									if (multipliers[i].HasMember("variables") && multipliers[i]["variables"].IsArray()) {
										for (int j = 0; j < multipliers[i]["variables"].Size(); j++) {
											if (multipliers[i]["variables"][j].HasMember("name") && multipliers[i]["variables"][j]["name"].IsString()) {
												varName = multipliers[i]["variables"][j]["name"].GetString();
												if (multipliers[i]["variables"][j].HasMember("source") && multipliers[i]["variables"][j]["source"].IsString()) {
													varSource = multipliers[i]["variables"][j]["source"].GetString();
												}
												if (multipliers[i]["variables"][j].HasMember("definition") && multipliers[i]["variables"][j]["definition"].IsString()) {
													sourceDef = multipliers[i]["variables"][j]["definition"].GetString();
												}
											}
											auto varName_s = std::string(varName);
											auto varSource_s = std::string(varSource);
											auto sourceDef_s = std::string(sourceDef);
											if (varName_s != "" && varSource_s != "") {
												std::tuple<std::string, std::string, std::string> variable;
												variable = { varName, varSource, sourceDef };
												newMultiplierF->function.variables.push_back(variable);
											}
										}

									} else {
										logger::info("No valid variables");
									}
								}
							}

							if (multipliers[i].HasMember("conditions") && multipliers[i]["conditions"].IsArray()) {
								const rapidjson::Value& conditions = multipliers[i]["conditions"];
								numCond = conditions.Size();
								logger::debug("About to iterate through all {} conditions in the multiplier", numCond);
								for (int j = 0; j < numCond; j++) {
									const char* variable = "";
									const char* variableDetail = "";
									const char* op = "";
									std::variant<std::string, float, bool> condValue;

									if (conditions[j].HasMember("condition")) {
										if (conditions[j]["condition"].IsString()) {
											variable = conditions[j]["condition"].GetString();
											auto variable_s = std::string(variable);
											if (variable_s == "global" || variable_s == "Global") {
												if (conditions[j].HasMember("conditionDetail")) {
													if (conditions[j]["conditionDetail"].IsString()) {
														variableDetail = conditions[j]["conditionDetail"].GetString();
													}
												}
											}
										} else {
											variable = "";
											logger::info("No valid variable");
										}

										if (conditions[j].HasMember("operator") && conditions[j]["operator"].IsString()) {
											op = conditions[j]["operator"].GetString();
										} else {
											op = "";
											logger::info("No valid operator");
										}

										if (conditions[j].HasMember("value")) {
											if (conditions[j]["value"].IsString()) {
												condValue = multipliers[i]["conditions"][j]["value"].GetString();
											} else if (conditions[j]["value"].IsFloat()) {
												condValue = multipliers[i]["conditions"][j]["value"].GetFloat();
											} else if (conditions[j]["value"].IsBool()) {
												condValue = multipliers[i]["conditions"][j]["value"].GetBool();
											}
											else {
												logger::info("No valid condition value");
											}
										} else {
											logger::info("No valid condition value");
										}

										if (!std::holds_alternative<std::string>(condValue) || (std::holds_alternative<std::string>(condValue) && *std::get_if<std::string>(&condValue) != "")) {
											auto assignedVariable = Conditions::AssignVariable(variable, condValue);
											auto newCondition = Conditions::Condition(variable, op, assignedVariable, variableDetail);
											if (isFunction) {
												newMultiplierF->conditions.push_back(newCondition);
											} else {
												newMultiplier->conditions.push_back(newCondition);
											}
											if (std::holds_alternative<std::string>(condValue)) {
												auto cvString = *std::get_if<std::string>(&condValue);
												logger::info("Pushed back a new condition: {} {} {}", variable, op, cvString);
											} else if (std::holds_alternative<std::string>(condValue)) {
												auto cvFloat = *std::get_if<float>(&condValue);
												logger::info("Pushed back a new condition: {} {} {}", variable, op, cvFloat);
											} else if (std::holds_alternative<std::string>(condValue)) {
												auto cvBool = *std::get_if<bool>(&condValue);
												logger::info("Pushed back a new condition: {} {} {}", variable, op, cvBool);
											}
										}

									} else {
										logger::info("No valid condition");
									}
								}

							} else {
								if (isFunction) {
									newMultiplierF->conditions = {};
								} else {
									newMultiplier->conditions = {};
								}
							}

							if (multipliers[i].HasMember("conditionsOperator") && multipliers[i]["conditionsOperator"].IsString()) {
								condOp = multipliers[i]["conditionsOperator"].GetString();
							} else {
								condOp = "and";
							}

							if (multipliers[i].HasMember("parameter") && multipliers[i]["parameter"].IsString()) {
								parameter = multipliers[i]["parameter"].GetString();
								auto parameter_s = std::string(parameter);
								if (parameter_s == "Time" || parameter_s == "time") {
									if (isFunction) {
										newMultiplierF->condOp = condOp;
										ctMultipliersF.push_back(newMultiplierF);
										logger::info("A multiplier effecting parameter: {} with the function: '{}' added with {} conditions", parameter_s, function, numCond);
									} else {
										newMultiplier->condOp = condOp;
										ctMultipliers.push_back(newMultiplier);
										logger::info("A multiplier of {} effecting parameter: {} added with {} conditions", mult, parameter_s, numCond);
									}
								} else if (parameter_s == "Cost" || parameter_s == "cost") {
									if (isFunction) {
										newMultiplierF->condOp = condOp;
										cMultipliersF.push_back(newMultiplierF);
										logger::info("A multiplier effecting parameter: {} with the function: '{}' added with {} conditions", parameter_s, function, numCond);
									} else {
										newMultiplier->condOp = condOp;
										cMultipliers.push_back(newMultiplier);
										logger::info("A multiplier of {} effecting parameter: {} added with {} conditions", mult, parameter_s, numCond);
									}
								} else if (parameter_s == "Magnitude" || parameter_s == "magnitude") {
									if (isFunction) {
										newMultiplierF->condOp = condOp;
										mMultipliersF.push_back(newMultiplierF);
										logger::info("A multiplier effecting parameter: {} with the function: '{}' added with {} conditions", parameter_s, function, numCond);
									} else {
										newMultiplier->condOp = condOp;
										mMultipliers.push_back(newMultiplier);
										logger::info("A multiplier of {} effecting parameter: {} added with {} conditions", mult, parameter_s, numCond);
									}
								} else {
									logger::info("No valid parameter.");
								}
							} else {
								logger::info("No valid parameter.");
							}

						} else {
							logger::info("The value type was not a valid type");
						}
					} else {
						logger::info("No object 'valueType' found");
					}
				}
			} else {
				logger::info("{} did not contain a valid array: Multipliers", configLocation);
			}
		} else {
			logger::info("{} did not contain a valid object: Multipliers", configLocation);
		}
		return true;
	}

	bool GetModifiers(rapidjson::Document& config, std::string configLocation)
	{
		if (config.HasMember("Modifiers")) {
			const rapidjson::Value& modifiers = config["Modifiers"];

			if (modifiers.IsArray()) {
				logger::info("About to iterate through all {} modifiers in the json", modifiers.Size());
				for (int i = 0; i < modifiers.Size(); i++) {
					Modifier* newModifier = new Modifier();
					ModifierF* newModifierF = new ModifierF();

					const char* condOp = "and";
					float numCond = 0;
					const char* valueType = "";
					const char* function = "";
					const char* parameter = "";  
					const char* varName = "";
					const char* varSource = "";
					const char* sourceDef = "";
					float mult = -1;

					bool isFunction = false;
					if (modifiers[i].HasMember("valueType") && modifiers[i]["valueType"].IsString()) {
						valueType = modifiers[i]["valueType"].GetString();
						auto valueType_s = std::string(valueType);
				
						if (valueType_s == "float" || valueType_s == "function") {
							if (valueType_s == "float") {
								if (modifiers[i].HasMember("value") && modifiers[i]["value"].IsFloat()) {
									mult = modifiers[i]["value"].GetFloat();
									isFunction = false;
									newModifier->value = mult;
								}
							} else if (valueType_s == "function") {
								if (modifiers[i].HasMember("function") && modifiers[i]["function"].IsString()) {
									function = modifiers[i]["function"].GetString();
									isFunction = true;
									newModifierF->function.function = function;

									if (modifiers[i].HasMember("variables") && modifiers[i]["variables"].IsArray()) {
										for (int j = 0; j < modifiers[i]["variables"].Size(); j++) {
											if (modifiers[i]["variables"][j].HasMember("name") && modifiers[i]["variables"][j]["name"].IsString()) {
												varName = modifiers[i]["variables"][j]["name"].GetString();
												if (modifiers[i]["variables"][j].HasMember("source") && modifiers[i]["variables"][j]["source"].IsString()) {
													varSource = modifiers[i]["variables"][j]["source"].GetString();
												}
												if (modifiers[i]["variables"][j].HasMember("definition") && modifiers[i]["variables"][j]["definition"].IsString()) {
													sourceDef = modifiers[i]["variables"][j]["definition"].GetString();
												}
											}
											auto varName_s = std::string(varName);
											auto varSource_s = std::string(varSource);
											auto sourceDef_s = std::string(sourceDef);
											if (varName_s != "" && varSource_s != "") {
												std::tuple<std::string, std::string, std::string> variable;
												variable = { varName, varSource, sourceDef };
												newModifierF->function.variables.push_back(variable);
											}
										}

									} else {
										logger::info("No valid variables");
									}
								}
							}

							if (modifiers[i].HasMember("conditions") && modifiers[i]["conditions"].IsArray()) {
								const rapidjson::Value& conditions = modifiers[i]["conditions"];
								numCond = conditions.Size();
								logger::debug("About to iterate through all {} conditions in the multiplier", numCond);
								for (int j = 0; j < numCond; j++) {
									const char* variable = "";
									const char* variableDetail = "";
									const char* op = "";
									std::variant<std::string, float, bool> condValue;

									if (conditions[j].HasMember("condition")) {
										if (conditions[j]["condition"].IsString()) {
											variable = conditions[j]["condition"].GetString();
											auto variable_s = std::string(variable);
											if (variable_s == "global" || variable_s == "Global") {
												if (conditions[j].HasMember("conditionDetail")) {
													if (conditions[j]["conditionDetail"].IsString()) {
														variableDetail = conditions[j]["conditionDetail"].GetString();
													}
												}
											}
										} else {
											variable = "";
											logger::info("No valid variable");
										}

										if (conditions[j].HasMember("operator") && conditions[j]["operator"].IsString()) {
											op = conditions[j]["operator"].GetString();
										} else {
											op = "";
											logger::info("No valid operator");
										}

										if (conditions[j].HasMember("value")) {
											if (conditions[j]["value"].IsString()) {
												condValue = modifiers[i]["conditions"][j]["value"].GetString();
											} else if (conditions[j]["value"].IsFloat()) {
												condValue = modifiers[i]["conditions"][j]["value"].GetFloat();
											} else if (conditions[j]["value"].IsBool()) {
												condValue = modifiers[i]["conditions"][j]["value"].GetBool();
											} else {
												logger::info("No valid condition value");
											}
										} else {
											logger::info("No valid condition value");
										}

										if (!std::holds_alternative<std::string>(condValue) || (std::holds_alternative<std::string>(condValue) && *std::get_if<std::string>(&condValue) != "")) {
											auto assignedVariable = Conditions::AssignVariable(variable, condValue);
											auto newCondition = Conditions::Condition(variable, op, assignedVariable, variableDetail);
											if (isFunction) {
												newModifierF->conditions.push_back(newCondition);
											} else {
												newModifier->conditions.push_back(newCondition);
											}
											if (std::holds_alternative<std::string>(condValue)) {
												auto cvString = *std::get_if<std::string>(&condValue);
												logger::info("Pushed back a new condition: {} {} {}", variable, op, cvString);
											} else if (std::holds_alternative<std::string>(condValue)) {
												auto cvFloat = *std::get_if<float>(&condValue);
												logger::info("Pushed back a new condition: {} {} {}", variable, op, cvFloat);
											} else if (std::holds_alternative<std::string>(condValue)) {
												auto cvBool = *std::get_if<bool>(&condValue);
												logger::info("Pushed back a new condition: {} {} {}", variable, op, cvBool);
											}
										}

									} else {
										logger::info("No valid condition");
									}
								}

							} else {
								if (isFunction) {
									newModifierF->conditions = {};
								} else {
									newModifier->conditions = {};
								}
							}

							if (modifiers[i].HasMember("conditionsOperator") && modifiers[i]["conditionsOperator"].IsString()) {
								condOp = modifiers[i]["conditionsOperator"].GetString();
							} else {
								condOp = "and";
							}
							if (modifiers[i].HasMember("parameter") && modifiers[i]["parameter"].IsString()) {
								parameter = modifiers[i]["parameter"].GetString();
								std:: string parameter_s = std::string(parameter);
								if (parameter_s == "Time" || parameter_s == "time") {
									if (isFunction) {
										newModifierF->condOp = condOp;
										cModifiersF.push_back(newModifierF);
										logger::info("A modifier effecting parameter: {} with the function: '{}' added with {} conditions", parameter_s, function, numCond);
									} else {
										newModifier->condOp = condOp;
										cModifiers.push_back(newModifier);
										logger::info("A modifier of {} effecting parameter: {} added with {} conditions", mult, parameter_s, numCond);
									}
								} else if (parameter_s == "Cost" || parameter_s == "cost") {
									if (isFunction) {
										newModifierF->condOp = condOp;
										cModifiersF.push_back(newModifierF);
										logger::info("A modifier effecting parameter: {} with the function: '{}' added with {} conditions", parameter_s, function, numCond);
									} else {
										newModifier->condOp = condOp;
										cModifiers.push_back(newModifier);
										logger::info("A modifier of {} effecting parameter: {} added with {} conditions", mult, parameter_s, numCond);
									}
								} else if (parameter_s == "Magnitude" || parameter_s == "magnitude") {
									if (isFunction) {
										newModifierF->condOp = condOp;
										cModifiersF.push_back(newModifierF);
										logger::info("A modifier effecting parameter: {} with the function: '{}' added with {} conditions", parameter_s, function, numCond);
									} else {
										newModifier->condOp = condOp;
										cModifiers.push_back(newModifier);
										logger::info("A modifier of {} effecting parameter: {} added with {} conditions", mult, parameter_s, numCond);
									}
								} else {
									logger::info("No valid parameter.");
								}

							} else {
									logger::info("No valid parameter.");
							}
						} else {
							logger::info("The value type was not a valid type");
						}
					} else {
						logger::info("No object 'valueType' found");
					}
				}
			} else {
				logger::info("{} did not contain a valid array 'Modifiers'", configLocation);
			}
		} else {
			logger::info("{} did not contain a valid object 'Modifiers'", configLocation);
		}
		return true;
	}

	bool GetResources(rapidjson::Document& config, std::string configLocation) {
		if (config.HasMember("Resources")) {
			const rapidjson::Value& resources = config["Resources"];
			if (resources.IsArray()) {
				logger::info("About to iterate through all {} resources in the json", resources.Size());
				for (int i = 0; i < resources.Size(); i++) {
					//Variables for Spells, Scrolls, and Staves
					const char* res;
					const char* condOp = "and";  //comparative operator
					float numCond = 0;           //number of conditions
					if (resources[i].HasMember("resource") && resources[i]["resource"].IsString()) {
						res = (resources[i]["resource"].GetString());
					} else {
						res = "magicka";
						logger::info("No valid resource for index {}. Resource set to magicka", i);
					}

					CastingResource* resource = new CastingResource();
					auto res_s = std::string(res);
					if (res_s == "magicka") {
						resource->resource = RE::ActorValue::kMagicka;
					} else if (res_s == "health") {
						resource->resource = RE::ActorValue::kHealth;
					} else if (res_s == "stamina") {
						resource->resource = RE::ActorValue::kStamina;
					}

					if (resources[i].HasMember("conditions") && resources[i]["conditions"].IsArray()) {
						const rapidjson::Value& conditions = resources[i]["conditions"];
						numCond = conditions.Size();
						logger::debug("About to iterate through all {} conditions in the resource", numCond);
						for (int j = 0; j < numCond; j++) {
							const char* variable = "";
							const char* variableDetail = "";
							const char* op = "";
							std::variant<std::string, float, bool> condValue;

							if (conditions[j].HasMember("condition")) {
								if (conditions[j]["condition"].IsString()) {
									variable = conditions[j]["condition"].GetString();
									auto variable_s = std::string(variable);
									if (variable_s == "global" || variable_s == "Global") {
										if (conditions[j].HasMember("conditionDetail")) {
											if (conditions[j]["conditionDetail"].IsString()) {
												variableDetail = conditions[j]["conditionDetail"].GetString();
											}
										}
									}
								} else {
									variable = "";
									logger::info("No valid variable");
								}

								if (conditions[j].HasMember("operator") && conditions[j]["operator"].IsString()) {
									op = conditions[j]["operator"].GetString();

								} else {
									op = "";
									logger::info("No valid operator");
								}

								if (conditions[j].HasMember("value")) {
									if (conditions[j]["value"].IsString()) {
										condValue = resources[i]["conditions"][j]["value"].GetString();
									} else if (conditions[j]["value"].IsFloat()) {
										condValue = resources[i]["conditions"][j]["value"].GetFloat();
									} else if (conditions[j]["value"].IsBool()) {
										condValue = resources[i]["conditions"][j]["value"].GetBool();
									} else {
										logger::info("No valid condition value");
									}
								} else {
									logger::info("No valid condition value");
								}

								if (!std::holds_alternative<std::string>(condValue) || (std::holds_alternative<std::string>(condValue) && *std::get_if<std::string>(&condValue) != "")) {
									auto assignedVariable = AssignVariable(variable, condValue);
									auto newCondition = Condition(variable, op, assignedVariable, variableDetail);
									resource->conditions.push_back(newCondition);
									if (std::holds_alternative<std::string>(condValue)) {
										auto cvString = *std::get_if<std::string>(&condValue);
										logger::info("Pushed back a new condition: {} {} {}", variable, op, cvString);
									} else if (std::holds_alternative<std::string>(condValue)) {
										auto cvFloat = *std::get_if<float>(&condValue);
										logger::info("Pushed back a new condition: {} {} {}", variable, op, cvFloat);
									} else if (std::holds_alternative<std::string>(condValue)) {
										auto cvBool = *std::get_if<bool>(&condValue);
										logger::info("Pushed back a new condition: {} {} {}", variable, op, cvBool);
									}
								}

							} else {
								logger::info("No valid condition");
							}
						}

					} else {
						resource->conditions = {};
					}

					if (resources[i].HasMember("conditionsOperator") && resources[i]["conditionsOperator"].IsString()) {
						condOp = resources[i]["conditionsOperator"].GetString();
					} else {
						condOp = "and";
					}
					resource->condOp = condOp;

					//Add to resources vector
					Resources.push_back(resource);
					logger::info("Added a resource ({}) with {} conditions", res, numCond);
				}
			} else {
				logger::info("{} did not contain a valid array: resources", configLocation);
			}
		} else {
			logger::info("{} did not contain a valid object: resources", configLocation);
		}
		return true;
	}

	bool GetReplacements(rapidjson::Document& config, std::string configLocation)
	{
		if (config.HasMember("Replacements")) {
			const rapidjson::Value& replacements = config["Replacements"];
			if (replacements.IsArray()) {
				logger::info("About to iterate through all {} replacements in the json", replacements.Size());
				for (int i = 0; i < replacements.Size(); i++) {
					//Variables for Spells, Scrolls, and Staves
					const char* res = "";
					const char* condOp = "and";  //comparative operator
					float numCond = 0;           //number of conditions
					if (replacements[i].HasMember("replacement") && replacements[i]["replacement"].IsString()) {
						res = (replacements[i]["replacement"].GetString());
					} else {
						res = "";
						logger::info("No valid resource for index {}. Resource set to magicka", i);
					}

					ReplacementSpell* replacement = new ReplacementSpell();
					auto res_s = std::string(res);
					auto form = Utility::TES::GetFormFromEditorID<RE::MagicItem>(res_s);
					if (form) {
						replacement->replacement = form;
					} else {
						logger::warn("{} was not a valid editorID", res_s);
						replacement->replacement = nullptr;
					}

					if (replacements[i].HasMember("conditions") && replacements[i]["conditions"].IsArray()) {
						const rapidjson::Value& conditions = replacements[i]["conditions"];
						numCond = conditions.Size();
						logger::debug("About to iterate through all {} conditions in the override", numCond);
						for (int j = 0; j < numCond; j++) {
							const char* variable = "";
							const char* variableDetail = "";
							const char* op = "";
							std::variant<std::string, float, bool> condValue;

							if (conditions[j].HasMember("condition")) {
								if (conditions[j]["condition"].IsString()) {
									variable = conditions[j]["condition"].GetString();
									auto variable_s = std::string(variable);
									if (variable_s == "global" || variable_s == "Global") {
										if (conditions[j].HasMember("conditionDetail")) {
											if (conditions[j]["conditionDetail"].IsString()) {
												variableDetail = conditions[j]["conditionDetail"].GetString();
											}
										}
									}
								} else {
									variable = "";
									logger::info("No valid variable");
								}

								if (conditions[j].HasMember("operator") && conditions[j]["operator"].IsString()) {
									op = conditions[j]["operator"].GetString();

								} else {
									op = "";
									logger::info("No valid operator");
								}

								if (conditions[j].HasMember("value")) {
									if (conditions[j]["value"].IsString()) {
										condValue = replacements[i]["conditions"][j]["value"].GetString();
									} else if (conditions[j]["value"].IsFloat()) {
										condValue = replacements[i]["conditions"][j]["value"].GetFloat();
									} else if (conditions[j]["value"].IsBool()) {
										condValue = replacements[i]["conditions"][j]["value"].GetBool();
									} else {
										logger::info("No valid condition value");
									}
								} else {
									logger::info("No valid condition value");
								}

								if (!std::holds_alternative<std::string>(condValue) || (std::holds_alternative<std::string>(condValue) && *std::get_if<std::string>(&condValue) != "")) {
									auto assignedVariable = AssignVariable(variable, condValue);
									auto newCondition = Condition(variable, op, assignedVariable, variableDetail);
									replacement->conditions.push_back(newCondition);
									if (std::holds_alternative<std::string>(condValue)) {
										auto cvString = *std::get_if<std::string>(&condValue);
										logger::info("Pushed back a new condition: {} {} {}", variable, op, cvString);
									} else if (std::holds_alternative<std::string>(condValue)) {
										auto cvFloat = *std::get_if<float>(&condValue);
										logger::info("Pushed back a new condition: {} {} {}", variable, op, cvFloat);
									} else if (std::holds_alternative<std::string>(condValue)) {
										auto cvBool = *std::get_if<bool>(&condValue);
										logger::info("Pushed back a new condition: {} {} {}", variable, op, cvBool);
									}
								}

							} else {
								logger::info("No valid condition");
							}
						}

					} else {
						replacement->conditions = {};
					}

					if (replacements[i].HasMember("conditionsOperator") && replacements[i]["conditionsOperator"].IsString()) {
						condOp = replacements[i]["conditionsOperator"].GetString();
					} else {
						condOp = "and";
					}
					replacement->condOp = condOp;

					//Add to replacements vector
					Replacements.push_back(replacement);
					logger::info("Added a replacement ({}) with {} conditions", res, numCond);
				}
			} else {
				logger::info("{} did not contain a valid array: replacements", configLocation);
			}
		} else {
			logger::info("{} did not contain a valid object: replacements", configLocation);
		}
		return true;
	}

	bool GetFromJSON()
	{
		
		logger::info("Attempting to access _DMMF.json's");
		std::vector<std::filesystem::path> configPaths = clib_util::distribution::get_configs_paths(R"(Data\SKSE\Plugins\DynamicMagicModificationFramework\)", "_DMMF"sv, ".json");  //change to be _DMMF instead
		std::filesystem::path configPath{ "Data/SKSE/Plugins/DynamicMagicModificationFramework/"sv };

		logger::info("Iterating through {} json files", configPaths.size());
		for (auto configLocation : configPaths) {
			logger::info("\n\n");
			logger::info("Attempting to access {}", configLocation.string());
			FILE* fp = _wfopen(configLocation.c_str(), L"r");

			char readBuffer[65536]{};
			rapidjson::FileReadStream is{ fp, readBuffer, sizeof(readBuffer) };
			rapidjson::Document config;

			config.ParseStream(is);
			fclose(fp);

			if (config.HasParseError()) {
				logger::error("There was a parse error for {}", configLocation.string());
			} else {
				logger::info("There was no parse error for {}", configLocation.string());
			}
			
			bool gotOverrides = GetOverrides(config, configLocation.string());
			bool gotMultipliers = GetMultipliers(config, configLocation.string());
			bool gotModifiers = GetModifiers(config, configLocation.string());
			bool gotResources = GetResources(config, configLocation.string());
			bool gotReplacements = GetReplacements(config, configLocation.string());
		}
		return true;
	}

	
}

