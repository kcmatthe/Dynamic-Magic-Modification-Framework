#include "KeyPress.h"

namespace KeyPress
{
	void InputEventSink::RegisterInput()
	{
		auto inputMgr = RE::BSInputDeviceManager::GetSingleton();
		InputEventSink* inputEventSink = new InputEventSink;
		inputMgr->AddEventSink(inputEventSink);
		logger::info("Input event sink registered");
	}
}
