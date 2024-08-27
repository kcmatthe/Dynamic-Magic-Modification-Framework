#pragma once

#include <functional>
#include <future>
#include <vector>

namespace Message
{

	class Message
	{
		class MessageBoxResultCallback : public RE::IMessageBoxCallback
		{
			std::function<void(unsigned int)> _callback;

		public:
			~MessageBoxResultCallback() override {}
			MessageBoxResultCallback(std::function<void(unsigned int)> callback) :
				_callback(callback) {}
			void Run(RE::IMessageBoxCallback::Message message) override
			{
				_callback(static_cast<unsigned int>(message));
			}
		};

	public:
		static void Show(const std::string& bodyText, std::vector<std::string> buttonTextValues,
			std::function<void(unsigned int)> callback)
		{
			//logger::info("Show (5)");
			auto* factoryManager = RE::MessageDataFactoryManager::GetSingleton();
			auto* uiStringHolder = RE::InterfaceStrings::GetSingleton();
			auto* factory = factoryManager->GetCreator<RE::MessageBoxData>(uiStringHolder->messageBoxData);  // "MessageBoxData" <--- can we just use this string?
			auto* messagebox = factory->Create();
			RE::BSTSmartPointer<RE::IMessageBoxCallback> messageCallback = RE::make_smart<MessageBoxResultCallback>(callback);
			messagebox->callback = messageCallback;
			messagebox->bodyText = bodyText;
			for (auto text : buttonTextValues) messagebox->buttonText.push_back(text.c_str());
			messagebox->QueueMessage();
		}
	};

	static void ShowMessageBox(const std::string& bodyText, std::vector<std::string> buttonTextValues, std::function<void(unsigned int)> callback)
	{
		//logger::info("ShowMessageBox (4)");
		Message::Show(bodyText, buttonTextValues, callback);
	}
	/////////////////////////////////////////////

	unsigned int GetNewMessageBoxId();
	unsigned int ShowArrayNonBlocking(std::string bodyText, std::vector<std::string> buttonTexts);
	unsigned int ShowNonBlocking(std::string bodyText, std::string button1,
		std::string button2 = "", std::string button3 = "", std::string button4 = "",
		std::string button5 = "", std::string button6 = "", std::string button7 = "",
		std::string button8 = "", std::string button9 = "", std::string button10 = "");
	void Delete(unsigned int messageBoxId);
	std::string GetResultText(unsigned int messageBoxId, bool deleteResultOnAccess = true);
	int GetResultIndex(unsigned int messageBoxId, bool deleteResultOnAccess = true);
	bool IsMessageResultAvailable(unsigned int messageBoxId);

	int ShowMessage(std::string bodyText, std::string button1,
		std::string button2 = "", std::string button3 = "", std::string button4 = "",
		std::string button5 = "", std::string button6 = "", std::string button7 = "",
		std::string button8 = "", std::string button9 = "", std::string button10 = "");
}
