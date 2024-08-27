#pragma once

#include <fstream>
#include <iostream>
#include <rapidjson/document.h>
#include <rapidjson/reader.h>
#include "rapidjson/filereadstream.h"
#include "rapidjson/stringbuffer.h"
#include <rapidjson/writer.h>
#include <cstdio>

#include "Conditions.h"

namespace config
{
	

	struct Override
	{
	public:
		bool excluded = false;
		bool override = false;
		float value;
		std::vector<Conditions::Condition> conditions;
		std::string condOp;

		Override(bool e, bool o, float val, std::vector<Conditions::Condition> c, std::string co)
		{
			excluded = e;
			override = o;
			value = val;
			conditions = c;
			condOp = co;
		}

		Override() {
			excluded = false;
			override = false;
			value = -1;
			conditions = {};
			condOp = "and";
		}
	};

	
	
	
	

	struct Function
	{
	public:
		std::string function;
		std::vector<std::tuple<std::string, std::string, std::string>> variables;
		

		Function() {
			function = "";
			variables = {};
		}

		Function(std::string f, std::vector<std::tuple<std::string, std::string, std::string>> v)
		{
			function = f;
			variables = v;
		}
	};

	struct MultiplierF
	{
	public:
		Function function;
		std::vector<Conditions::Condition> conditions;
		std::string condOp;

		MultiplierF(Function f, std::vector<Conditions::Condition> c, std::string co)
		{
			function = f;
			conditions = c;
			condOp = co;
		}

		MultiplierF()
		{
			function = Function();
			conditions = {};
			condOp = "";
		}
	};

	struct Multiplier
	{
	public:
		
		float value;
		std::vector<Conditions::Condition> conditions;
		std::string condOp;


		Multiplier(float v, std::vector<Conditions::Condition> c, std::string co)
		{
			value = v;
			conditions = c;
			condOp = co;
		}

		Multiplier() {
			value = -1;
			conditions = {};
			condOp = "";
		}
	};

	struct ModifierF
	{
	public:
		Function function;
		std::vector<Conditions::Condition> conditions;
		std::string condOp;

		ModifierF(Function f, std::vector<Conditions::Condition> c, std::string co)
		{
			function = f;
			conditions = c;
			condOp = co;
		}

		ModifierF()
		{
			function = Function();
			conditions = {};
			condOp = "";
		}
	};

	struct Modifier
	{
	public:
		float value;
		std::vector<Conditions::Condition> conditions;
		std::string condOp;

		Modifier(float v, std::vector<Conditions::Condition> c, std::string co)
		{
			value = v;
			conditions = c;
			condOp = co;
		}

		Modifier()
		{
			value = -1;
			conditions = {};
			condOp = "";
		}
	};

	struct CastingResource
	{
	public:
		RE::ActorValue resource;
		std::vector<Conditions::Condition> conditions;
		std::string condOp;

		CastingResource(RE::ActorValue av, std::vector<Conditions::Condition> c, std::string co)
		{
			resource = av;
			conditions = c;
			condOp = co;
		}

		CastingResource()
		{
			resource = RE::ActorValue::kMagicka;
			conditions = {};
			condOp = "";
		}
	};

	struct ReplacementSpell
	{
	public:
		RE::MagicItem* replacement;
		std::vector<Conditions::Condition> conditions;
		std::string condOp;

		ReplacementSpell(RE::MagicItem* spell, std::vector<Conditions::Condition> c, std::string co)
		{
			replacement = spell;
			conditions = c;
			condOp = co;
		}

		ReplacementSpell()
		{
			replacement = nullptr;
			conditions = {};
			condOp = "";
		}
	};

	inline std::vector<Override*> cOverrides;
	inline std::vector<Multiplier*> cMultipliers;
	inline std::vector<MultiplierF*> cMultipliersF;
	inline std::vector<Modifier*> cModifiers;
	inline std::vector<ModifierF*> cModifiersF;

	inline std::vector<Override*> ctOverrides;
	inline std::vector<Multiplier*> ctMultipliers;
	inline std::vector<MultiplierF*> ctMultipliersF;
	inline std::vector<Modifier*> ctModifiers;
	inline std::vector<ModifierF*> ctModifiersF;

	inline std::vector<Override*> mOverrides;
	inline std::vector<Multiplier*> mMultipliers;
	inline std::vector<MultiplierF*> mMultipliersF;
	inline std::vector<Modifier*> mModifiers;
	inline std::vector<ModifierF*> mModifiersF;

	inline std::vector<CastingResource*> Resources;
	inline std::vector<ReplacementSpell*> Replacements;

	bool GetFromJSON();
	/*
	bool GetOverrides(rapidjson::Document& config, std::string configLocation);
	bool GetMultipliers(rapidjson::Document& config, std::string configLocation);
	bool GetModifiers(rapidjson::Document& config, std::string configLocation);
	bool GetResources(rapidjson::Document& config, std::string configLocation);
	*/
}

