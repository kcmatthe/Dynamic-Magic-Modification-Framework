#include "MessageBox.h"

#include <algorithm>
#include <atomic>
#include <unordered_map>
#include <vector>

 //From Papyrus MessageBox SKSE NG (MrowPurr)
namespace Message {

	std::atomic<unsigned int> _nextMessageBoxId = 1;
	std::unordered_map<unsigned int, unsigned int> _messageBoxResults;
	std::unordered_map<unsigned int, std::vector<std::string>> _messageBoxButtons;

	
	
	
	unsigned int GetNewMessageBoxId() { return _nextMessageBoxId++; }

	unsigned int ShowArrayNonBlocking(std::string bodyText, std::vector<std::string> buttonTexts)
	{
		
		//logger::info("ShowArrayNonBlocking (3)");
		std::erase_if(buttonTexts, [](const std::string& text) { return text.empty(); });
		auto messageBoxId = GetNewMessageBoxId();
		_messageBoxButtons.emplace(messageBoxId, buttonTexts);
		ShowMessageBox(bodyText, buttonTexts, [messageBoxId](uint32_t result) {
			if (messageBoxId) {
				_messageBoxResults.insert_or_assign(messageBoxId, result);
			}
		});
		return messageBoxId;
	}

	unsigned int ShowNonBlocking(std::string bodyText, std::string button1,
		std::string button2, std::string button3, std::string button4,
		std::string button5, std::string button6, std::string button7,
		std::string button8, std::string button9, std::string button10)
	{

		//logger::info("ShowNonBlocking (2)");
		std::vector<std::string> buttonTexts;
		if (!button1.empty())
			buttonTexts.push_back(button1);
		if (!button2.empty())
			buttonTexts.push_back(button2);
		if (!button3.empty())
			buttonTexts.push_back(button3);
		if (!button4.empty())
			buttonTexts.push_back(button4);
		if (!button5.empty())
			buttonTexts.push_back(button5);
		if (!button6.empty())
			buttonTexts.push_back(button6);
		if (!button7.empty())
			buttonTexts.push_back(button7);
		if (!button8.empty())
			buttonTexts.push_back(button8);
		if (!button9.empty())
			buttonTexts.push_back(button9);
		if (!button10.empty())
			buttonTexts.push_back(button10);
		return ShowArrayNonBlocking(bodyText, buttonTexts);
	}
	
	
	void Delete(unsigned int messageBoxId)
	{
		if (_messageBoxResults.contains(messageBoxId))
			_messageBoxResults.erase(messageBoxId);
		if (_messageBoxButtons.contains(messageBoxId))
			_messageBoxButtons.erase(messageBoxId);
	}

	std::string GetResultText(unsigned int messageBoxId, bool deleteResultOnAccess) {
		std::string resultString;
		if (_messageBoxResults.contains(messageBoxId) && _messageBoxButtons.contains(messageBoxId)) {
			auto index = _messageBoxResults.at(messageBoxId);
			auto buttons = _messageBoxButtons.at(messageBoxId);
			if (buttons.size() > index) {
				resultString = buttons.at(index);
			}
			if (deleteResultOnAccess) {
				Delete(messageBoxId);
			}
		}
		return resultString;
	}

	int GetResultIndex(unsigned int messageBoxId, bool deleteResultOnAccess)
	{
		auto index = -1;
		if (_messageBoxResults.contains(messageBoxId)) {
			index = _messageBoxResults.at(messageBoxId);
			if (deleteResultOnAccess) {
				Delete(messageBoxId);
			}
		}
		return index;
	}

	bool IsMessageResultAvailable(unsigned int messageBoxId)
	{
		 return _messageBoxResults.contains(messageBoxId);
	}


	int ShowMessage(std::string bodyText, std::string button1,
		std::string button2, std::string button3, std::string button4,
		std::string button5, std::string button6, std::string button7,
		std::string button8, std::string button9, std::string button10) {

		//logger::info("ShowMessage (1)");
		
		int id = ShowNonBlocking(bodyText, button1, button2, button3, button4, button5, button6, button7, button8, button9, button10);
		
		return id;
	}
}
