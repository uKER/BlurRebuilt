#ifndef EDITOR_USERCMD_VERIFY_XT_VALUE_CMD_H
#define EDITOR_USERCMD_VERIFY_XT_VALUE_CMD_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Command/ICommand.h>
#include <EditorLib/Misc/Status.h>

namespace EditorLib
{
	class C_VerificationRecord;
}

namespace EditorUserCmd
{

class C_VerifyXtValueCmd : public EditorLib::I_Command
{
public:
	C_VerifyXtValueCmd(U32 type);
	virtual ~C_VerifyXtValueCmd();

	virtual EditorLib::C_Status Do();

	virtual bool IsUndoable() const;

	virtual void GetResults(EditorLib::ResultsArray& results) const;

	static EditorLib::I_Command*		Create(U32 type);
	static EditorLib::C_Syntax*			CreateSyntax();
	static const char*					GetName();

private:
	class C_XtValueFieldVisitor : public LvFieldDom
	{
	public:
		C_XtValueFieldVisitor(EditorLib::C_VerificationRecord& record);
		~C_XtValueFieldVisitor();

		virtual bool Visit(LfArray* field, Xt::DomArray* dom);
		virtual bool Visit(LfHandle* field, Xt::DomField* dom);
		virtual bool Visit(LfWeakPtr* field, Xt::DomRef* dom);
	private:
		void GetRefLocation(Btl::String& location, Xt::DomElement* element);

		EditorLib::C_VerificationRecord& m_Record;
	};

	class C_XtValueObjectVisitor : public LvObjects
	{
	public:
		C_XtValueObjectVisitor(C_XtValueFieldVisitor& fieldVisitor);
		virtual ~C_XtValueObjectVisitor();

		virtual bool Visit(struct LtStruct* type, Xt::DomBlock* dom);

	private:
		C_XtValueFieldVisitor& m_FieldVisitor;
	};

	EditorLib::C_Status Verify(Xt::DomElement* pElement);
	EditorLib::C_Status VerifyRecord(Xt::DomRecord* pRecord);
};

}

#endif