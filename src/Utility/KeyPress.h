#pragma once
#include "Hooks/Hooks.h"

namespace KeyPress
{

	inline bool isKeyPressed = false;

	struct KeyStateCache
	{
		
		std::array<std::atomic_bool, 256> keyDown{};
		std::array<std::atomic_bool, 16> mouseDown{};
		std::array<std::atomic_bool, 32> gamepadDown{};

		bool IsKeyboardDown(std::uint32_t key) const
		{
			return key < keyDown.size() ? keyDown[key].load(std::memory_order_relaxed) : false;
		}

		void SetKeyboardDown(std::uint32_t key, bool down)
		{
			if (key < keyDown.size()) {
				keyDown[key].store(down, std::memory_order_relaxed);
				//logger::trace("Key {} down={}", key, down);
			}
		}
	};

	inline KeyStateCache g_keyState;

	class InputEventSink final :
		public RE::BSTEventSink<RE::InputEvent*>
	{
	public:
		virtual RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* eventPtr, RE::BSTEventSource<RE::InputEvent*>*) override
		{
			if (!eventPtr || !*eventPtr || !RE::Main::GetSingleton()->GetRuntimeData().gameActive) {
				return RE::BSEventNotifyControl::kContinue;
			}

			auto* event = *eventPtr;
			if (event->eventType == RE::INPUT_EVENT_TYPE::kButton) {

				auto* buttonEvent = event->AsButtonEvent();
				auto code = buttonEvent->GetIDCode();
				const auto device = buttonEvent->device.get();
				auto userEvent = RE::UserEvents::GetSingleton();
			
				auto ui = RE::UI::GetSingleton();

				if (ui && (ui->GameIsPaused() || ui->IsMenuOpen(RE::MainMenu::MENU_NAME))) {
					return RE::BSEventNotifyControl::kContinue;
				}

				bool down = buttonEvent->IsPressed();
				g_keyState.SetKeyboardDown(code, down);

				
				
				
			}

			return RE::BSEventNotifyControl::kContinue;
		}


		static void RegisterInput();

		
	};



	
}
