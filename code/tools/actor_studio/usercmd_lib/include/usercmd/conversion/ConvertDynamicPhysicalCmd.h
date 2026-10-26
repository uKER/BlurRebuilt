#ifndef EDITOR_USERCMD_CONVERT_DYNAMIC_PHYSICAL_CMD_H
#define EDITOR_USERCMD_CONVERT_DYNAMIC_PHYSICAL_CMD_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Command/ICommand.h>
#include <EditorLib/Misc/Status.h>

namespace EditorUserCmd
{

class C_ConvertDynamicPhysicalCmd : public EditorLib::I_Command
{
public:
	C_ConvertDynamicPhysicalCmd(U32 type);
	virtual ~C_ConvertDynamicPhysicalCmd();

	virtual EditorLib::C_Status Do();

	virtual bool IsUndoable() const;

	virtual void GetResults(EditorLib::ResultsArray& results) const;

	static EditorLib::I_Command*		Create(U32 type);
	static EditorLib::C_Syntax*			CreateSyntax();
	static const char*					GetName();

private:
	bool IsInteresting(EditorLib::I_DgNode* pNode);
	bool Convert(Xt::DomElement* pElement, int& dynamicObjectCount, int& convertedCount);
	
	void CachePhysicalGroups();
	void CachePhysicalObjects();
	XtId FindPhysicalGroupDefinition(XtId dynamicDefinition);
	XtId FindPhysicalObjectDefinition(XtId dynamicDefinition);

	Btl::OpenHashMap<Btl::String, XtId, Btl::HashKeyOpBtlString> m_PhysicalGroupNameMapping;

	Btl::OpenHashMap<Btl::String, XtId, Btl::HashKeyOpBtlString> m_PhysicalObjectNameMapping;
	Btl::OpenHashMap<XtId, XtId> m_PhysicalObjectModelMapping;

	Btl::OpenHashMap<XtId, XtId> m_ObjectMapping;
};

}

#endif