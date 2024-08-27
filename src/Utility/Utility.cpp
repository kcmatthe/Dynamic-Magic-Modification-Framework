#undef GetModuleHandle

#include "Utility.h"
#include <Windows.h>
#include <stdarg.h>
#include <winbase.h>
#include <iostream>
#include <stdio.h>
#include <chrono>
#include <ctime>
#include <string>
//#include <SKSE/Impl/WinAPI.h>

namespace Utility
{
	namespace Format
	{
		/* template <class... Arguments>
		std::string Print(std::string_view format, Arguments... arguments)
		{
			std::vector<char> buffer(std::snprintf(nullptr, 0, format.data(), arguments...) + 1);

			std::snprintf(buffer.data(), buffer.size(), format.data(), arguments...);

			return std::string(buffer.data(), buffer.size() - 1);
		}*/
	}
	namespace Utility
	{
		template <class T>
		void RemoveFromVectorByValue(std::vector<T>& v, T value)
		{
			v.erase(std::remove(v.begin(), v.end(), value), v.end());
		}
	}
	namespace Wait
	{
		int Wait::Cooldown(int val)
		{
			Active = true;
			while (Active == true) {
				std::this_thread::sleep_for(std::chrono::milliseconds(val));

				Active = false;
			}
			return 0;
		}

		int Wait::Timer(int val)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(val));
			return 0;
		}

		void DelayedFunction(auto function, int delay)
		{
			std::thread t([=]() {
				std::this_thread::sleep_for(std::chrono::milliseconds(delay));
				function();
			});
			t.detach();
		}
	}

	namespace Time
	{
			void Timer::Start()
			{
				start = std::chrono::high_resolution_clock::now();
			}

			void Timer::Stop()
			{
				end = std::chrono::high_resolution_clock::now();
			}

			double Timer::GetElapsedTime() const
			{
				std::chrono::duration<double> elapsed = end - start;
				return elapsed.count();
			}
	}

	namespace Math
	{
		float randomFraction()
		{
			return (float)(rand()) / (float)(RAND_MAX);
		}

		int randomInt(int a, int b)
		{
			if (a > b)
				return randomInt(b, a);
			if (a == b)
				return a;
			return a + (rand() % (b - a));
		}

		float randomFloat(int a, int b)
		{
			if (a > b)
				return randomFloat(b, a);
			if (a == b)
				return a;

			return (float)randomInt(a, b) + randomFraction();
		}

		void initializeRand()
		{
			srand(time(0));
		}

		template <typename T>
		std::string IntToHex(T i)
		{
			std::stringstream stream;
			stream << ""
				   << std::setfill('0') << std::setw(sizeof(T) * 2)
				   << std::hex << i;
			return stream.str();
		}
	}

	namespace TES
	{
		using namespace std;

		void sendConsoleCommandRef(std::string a_command, RE::TESObjectREFRPtr refPtr)
		{
			const auto scriptFactory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::Script>();
			const auto script = scriptFactory ? scriptFactory->Create() : nullptr;
			if (script) {
				script->SetCommand(a_command);
				script->CompileAndRun(refPtr.get());
				delete script;
			}
		}

		void sendConsoleCommand(std::string a_command)
		{
			const auto scriptFactory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::Script>();
			const auto script = scriptFactory ? scriptFactory->Create() : nullptr;
			if (script) {
				const auto selectedRef = RE::Console::GetSelectedRef();
				script->SetCommand(a_command);
				script->CompileAndRun(selectedRef.get());
				delete script;
			}
		}

		//GetEditorID from PO3 Tweaks

		inline std::string GetEditorID(const RE::FormID formID)
		{
			static auto tweaks = REX::W32::GetModuleHandleW(L"po3_Tweaks");
			static auto func = reinterpret_cast<const char* (*)(std::uint32_t)>(GetProcAddress(tweaks, "GetFormEditorID"));
			if (func) {
				return func(formID);
			}
			return "fail";
		}

		bool compareEditorID(RE::SpellItem* a, RE::SpellItem* b)
		{
			return GetEditorID(a->formID) < GetEditorID(b->formID);
		}

		void TogglePlayerControls(bool a_enable)
		{
			RE::ControlMap::GetSingleton()->ToggleControls(controlFlags, a_enable);

			if (const auto pcControls = RE::PlayerControls::GetSingleton()) {
				pcControls->readyWeaponHandler->SetInputEventHandlingEnabled(a_enable);
				pcControls->sneakHandler->SetInputEventHandlingEnabled(a_enable);
				pcControls->autoMoveHandler->SetInputEventHandlingEnabled(a_enable);
				pcControls->shoutHandler->SetInputEventHandlingEnabled(a_enable);
				pcControls->attackBlockHandler->SetInputEventHandlingEnabled(a_enable);
			}
		}

		void ModGlobal(RE::TESGlobal* global, float value)
		{
			float initial = global->value;
			global->value = initial + value;
		}

		void SetGlobal(RE::TESGlobal* global, float value)
		{
			global->value = value;
		}

		//From DbSkseFunctions (Dylbill)

		

		RE::BSFixedString GetFormName(RE::TESForm* akForm)
		{
			if (!akForm) {
				return "null";
			}

			RE::TESObjectREFR* ref = akForm->As<RE::TESObjectREFR>();
			if (ref) {
				return ref->GetDisplayFullName();
			} else {
				return akForm->GetName();
			}
		}

		std::string GetDescription(RE::TESForm* akForm)
		{
			logger::info("{} called", __func__);

			if (!akForm) {
				logger::warn("{}: akForm doesn't exist", __func__);
				return "";
			}

			auto* description = akForm->As<RE::TESDescription>();

			if (!description) {
				logger::error("{}: couldn't get description for form [{}] ID [{:x}]", __func__, GetFormName(akForm), akForm->GetFormID());
				return "";
			}

			RE::BSString descriptionString;
			description->GetDescription(descriptionString, akForm);

			return static_cast<std::string>(descriptionString);
		}
	}		
}
