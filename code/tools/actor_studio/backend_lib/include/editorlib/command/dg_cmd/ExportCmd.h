#ifndef EDITOR_LIB_EXPORT_CMD_H
#define EDITOR_LIB_EXPORT_CMD_H

#ifdef _MSC_VER
#pragma once
#endif

namespace EditorLib
{

class C_ExportCmd : public I_Command
{
public:
	C_ExportCmd (U32 type);
	virtual ~C_ExportCmd();

	virtual C_Status Do();
	virtual bool IsUndoable() const;

	virtual void GetResults(ResultsArray& results) const;

	static I_Command*	Create(U32 type);
	static C_Syntax*	CreateSyntax();
	static const char*	GetName();

private:
	void ExpandSelection(C_Selection& selection);
	void AddChildren(
		C_Selection& selection, 
		I_DgNode* pGroupNode, 
		Xt::DomArray* pActorsArray, 
		Xt::DomArray* pSceneNodeArray);
};

}

#endif