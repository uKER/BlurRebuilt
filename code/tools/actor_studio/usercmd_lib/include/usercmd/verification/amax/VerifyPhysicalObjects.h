#ifndef EDITOR_USERCMD_VERIFY_PHYSICAL_OBJECTS_CMD_H
#define EDITOR_USERCMD_VERIFY_PHYSICAL_OBJECTS_CMD_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Command/ICommand.h>
#include <EditorLib/Misc/Status.h>

namespace EditorUserCmd
{

class C_VerifyPhysicalObjectsCmd : public EditorLib::I_Command
{
public:
	C_VerifyPhysicalObjectsCmd(U32 type);
	virtual ~C_VerifyPhysicalObjectsCmd();

	virtual EditorLib::C_Status Do();

	virtual bool IsUndoable() const;

	virtual void GetResults(EditorLib::ResultsArray& results) const;

	static EditorLib::I_Command*		Create(U32 type);
	static EditorLib::C_Syntax*			CreateSyntax();
	static const char*					GetName();

private:
	EditorLib::C_Status Verify(Xt::DomElement* pElement);
	EditorLib::C_Status VerifyRecord(Xt::DomRecord* pRecord);

	void CachePhysicalObjects();
	XtId FindPhysicalObjectDefinition(XtId dynamicDefinition);

	Btl::OpenHashMap<Btl::String, XtId, Btl::HashKeyOpBtlString> m_PhysicalObjectNameMapping;
	Btl::OpenHashMap<XtId, XtId> m_PhysicalObjectModelMapping;

	Btl::OpenHashMap<XtId, XtId> m_ObjectMapping;
};

}

#endif