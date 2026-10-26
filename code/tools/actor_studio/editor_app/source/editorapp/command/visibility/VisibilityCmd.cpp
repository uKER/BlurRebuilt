#include "stdafx.h"
#include <Misc_Shared/Variant/KeyValue.h>

#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/XtChange.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Xt/XtSemantic.h>
#include <EditorLib/Command/Argument/ArgumentDatabase.h>
#include <EditorLib/Command/Argument/Argument.h>
#include <EditorLib/Command/Argument/Syntax.h>
#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Command/ICommand.h>
#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/DgNodeHelpers.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/Attr/IPlug.h>
#include <EditorLib/Dg/Attr/NullPlug.h>
#include <EditorLib/Dg/Attr/StaticPlug.h>
#include <EditorLib/Dg/XtNodeHelpers.h>
#include <EditorLib/Misc/UndoManager.h>
#include <EditorLib/Core.h>

#include <Aftk/AftkTypedefs.h>
#include <Aftk/AftkEnum.h>
#include <Aftk/AftkMaths.h>
#include <Aftk/AftkEuler.h>

#include <EditorApp/Command/Visibility/VisibilityCmd.h>


namespace Editor
{

C_VisibilityCmd::C_VisibilityCmd(U32 type) : EditorLib::I_Command(type)
{
}

C_VisibilityCmd::~C_VisibilityCmd()
{
}

EditorLib::C_Status C_VisibilityCmd::Do()
{
	EditorLib::C_Status status;
	
	EditorLib::NodeIdArray ids;
	if (m_args.IsSet('#'))	//Get the passed node IDs
	{
		const EditorLib::ArgumentDataArray& data = *m_args.GetArgData('#');
		Btl::String nodes;
		data[0].GetValue(nodes);
		EditorLib::DgNodeHelpers::BuildNodeList(ids, nodes, false, false);
		if (nodes.Length() > 0 && ids.Size() == 0)
		{
			LOGERROR(EditorLib::C_Core::Instance().GetLogger(), "Nothing found to select!");
			return EditorLib::E_Status_Failure;
		}
	} 
	else //Get all the node IDs
	{
		EditorLib::C_DgNodeIterator it = EditorLib::C_Core::Instance().GetDgManager().GetDgIterator();
		while ((EditorLib::I_DgNode*)it)
		{
			if (!IsInteresting(it)) 
			{
				it++;
				continue;
			}
			
			ids.PushBack(it->GetId());
			it++;
		}
	}

	bool visible = true;
	
	if (m_args.IsSet('s')) //Show mode
	{
		visible = true;	
	}
	else if (m_args.IsSet('h')) //Hide mode
	{
		visible = false;
	}
			
	if (ids.Size() == 0)
	{
		LOGERROR(EditorLib::C_Core::Instance().GetLogger(), "No object selected.");
		return EditorLib::E_Status_Failure;
	}

	EditorLib::C_CheckpointHelper c(&m_pChange, GetName());
	for (S32 i = 0; i < ids.Size(); i++)
	{
		EditorLib::C_NodeId id = ids[i];
		EditorLib::I_DgNode* pNode = EditorLib::C_Core::Instance().GetDgManager().FindNodeById(id);
	
		EditorLib::C_SceneXtNodeBase* pXtSceneNode = pNode->IsA(EditorLib::E_ObjType_SceneXtNodeBase) ? 
			(EditorLib::C_SceneXtNodeBase*)pNode : 0;
		if(pXtSceneNode)
		{
			Xt::DomComponent* pComponent = Xt::AsComponent(pXtSceneNode->GetXtData());
			assert(pComponent);

			if(visible)
				Xt::Set(&pComponent->data, "__visible", "true");
			else
				Xt::Set(&pComponent->data, "__visible", "false");

			Util::C_KeyValue attr;
			attr.SetValue(id.AsString());
			m_results.PushBack(attr);
		}
	}

	return EditorLib::E_Status_Success;
}

bool C_VisibilityCmd::IsUndoable() const
{
	return true;
}

void C_VisibilityCmd::GetResults(EditorLib::ResultsArray& results) const
{
	results = m_results;
}

EditorLib::I_Command* C_VisibilityCmd::Create(U32 type)
{
	return new C_VisibilityCmd(type);
}

EditorLib::C_Syntax* C_VisibilityCmd::CreateSyntax()
{
	EditorLib::C_Syntax* pSyntax = new EditorLib::C_Syntax();
	{
		{
			EditorLib::C_Argument arg(
				'#', 
				"",
				"Nodes to alter visibility",
				EditorLib::C_Argument::E_Required_False);
			EditorLib::VarTypeArray& dataTypes = arg.GetDataTypes();
			dataTypes.PushBack(Util::E_VarType_String);
			pSyntax->AddArg(arg);
		}
		{
			EditorLib::C_Argument arg(
				's', 
				"show",
				"Show actors",
				EditorLib::C_Argument::E_Required_False);
			pSyntax->AddArg(arg);
		}
		{
			EditorLib::C_Argument arg(
				'h', 
				"hide",
				"Hide actors",
				EditorLib::C_Argument::E_Required_False);
			pSyntax->AddArg(arg);
		}
	}

	return pSyntax;
}

const char* C_VisibilityCmd::GetName()
{
	static char* p = "visibility";
	return p;
}

bool C_VisibilityCmd::IsInteresting(EditorLib::I_DgNode* pNode)
{
	if (pNode && !pNode->IsReadOnly() && pNode->IsA(EditorLib::E_ObjType_SceneNode))
		return true;

	return false;
}

}
