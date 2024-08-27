#include "Papyrus.h"
#include "Settings/Settings.h"
#include "Utility/Logger.h"


namespace Papyrus
{
	void OnConfigClose(RE::TESQuest*)
	{
		Settings::ReadSettings();
	}

	bool Register(RE::BSScript::IVirtualMachine* a_vm)
	{
		a_vm->RegisterFunction("OnConfigClose", "_DMMF_MCM", OnConfigClose);

		return true;
	}
}
