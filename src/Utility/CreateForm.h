#pragma once
#include <unordered_set>

class FormRecord
{
private:
	FormRecord()
	{
	}
	FormRecord(RE::TESForm* _actualForm)
	{
		actualForm = _actualForm;
	}

public:
	static FormRecord* CreateNew(RE::TESForm* _actualForm, RE::FormType formType, RE::FormID formId)
	{
		if (!_actualForm) {
			return nullptr;
		}
		auto result = new FormRecord(_actualForm);
		result->formType = formType;
		result->formId = formId;
		return result;
	}
	static FormRecord* CreateReference(RE::TESForm* _actualForm)
	{
		if (!_actualForm) {
			return nullptr;
		}
		auto result = new FormRecord(_actualForm);
		result->formId = _actualForm->GetFormID();
		return result;
	}
	static FormRecord* CreateDeleted(RE::FormID formId)
	{
		auto result = new FormRecord();
		result->formId = formId;
		result->deleted = true;
		return result;
	}
	void UndeleteReference(RE::TESForm* _actualForm)
	{
		if (!_actualForm) {
			return;
		}
		deleted = false;
		actualForm = _actualForm;
		formId = _actualForm->GetFormID();
		return;
	}
	void Undelete(RE::TESForm* _actualForm, RE::FormType _formType)
	{
		if (!_actualForm) {
			return;
		}
		actualForm = _actualForm;
		formType = _formType;
		deleted = false;
	}

	bool Match(RE::TESForm* matchForm)
	{
		if (!matchForm) {
			return false;
		}
		return matchForm->GetFormID() == formId;
	}

	bool Match(RE::FormID matchForm)
	{
		return matchForm == formId;
	}
	bool HasModel()
	{
		return modelForm != nullptr;
	}
	RE::TESForm* baseForm = nullptr;
	RE::TESForm* actualForm = nullptr;
	RE::TESForm* modelForm = nullptr;
	bool deleted = false;
	RE::FormType formType;
	RE::FormID formId;
};

RE::TESForm* AddNewForm(RE::TESForm* baseItem);
void DeleteCreatedForm(RE::TESForm* form);
static void copyFormArmorModel(RE::TESForm* source, RE::TESForm* target);
static void copyFormObjectWeaponModel(RE::TESForm* source, RE::TESForm* target);
static void copyMagicEffect(RE::TESForm* source, RE::TESForm* target);
static void copyBookAppearence(RE::TESForm* source, RE::TESForm* target);
static void applyPattern(FormRecord* instance);
template <class T>
void copyComponent(RE::TESForm* from, RE::TESForm* to);
void copyAppearence(RE::TESForm* source, RE::TESForm* target);
static void applyPattern(FormRecord* instance);

//from model.cpp

inline std::vector<FormRecord*> formData;
inline std::vector<FormRecord*> formRef;

inline bool espFound = false;
inline uint32_t lastFormId = 0;  // last mod
inline uint32_t firstFormId = 0;  // last mod
inline uint32_t dynamicModId = 0;

void AddFormData(FormRecord* item);

void AddFormRef(FormRecord* item);

void EachFormData(std::function<bool(FormRecord*)> const& iteration);

void EachFormRef(std::function<bool(FormRecord*)> const& iteration);

void incrementLastFormID();

void UpdateId();

void ResetId();

void ReadFirstFormIdFromESP();
