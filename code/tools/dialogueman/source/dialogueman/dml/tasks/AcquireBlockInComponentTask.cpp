#include "stdafx.h"

#include <DialogueMan/Dml/Tasks/AcquireBlockInComponentTask.h>
#include <DialogueMan/Dml/Variable.h>

#include <wx/tokenzr.h>

namespace DMan
{
namespace Dml
{

const wxString C_AcquireBlockInComponentTask::IN_COMPONENT_PTR = "inComponentPtr";
const wxString C_AcquireBlockInComponentTask::IN_BLOCK_NAME = "inBlockName";
const wxString C_AcquireBlockInComponentTask::OUT_BLOCK_PTR = "outBlockPtr";

C_AcquireBlockInComponentTask::C_AcquireBlockInComponentTask(const wxString& name) : 
	C_Task(name)
{}

C_AcquireBlockInComponentTask::~C_AcquireBlockInComponentTask()
{}

bool C_AcquireBlockInComponentTask::Execute()
{
	if(!GetInput())
	{
		LOGERROR(wxGetApp().GetLogger(), "Task \"%s\": No input assigned", GetName());
		return false;
	}
	if(!GetOutput())
	{
		LOGERROR(wxGetApp().GetLogger(), "Task \"%s\": No output assigned", GetName());
		return false;
	}

	C_MutantValue inComponentPtr = GetInputVariable(IN_COMPONENT_PTR);
	C_MutantValue inBlockName = GetInputVariable(IN_BLOCK_NAME);

	Xt::DomComponent* pComponent = (Xt::DomComponent*)(void*)inComponentPtr;
	if(!pComponent)
	{
		LOGERROR(wxGetApp().GetLogger(), "Task \"%s\": Invalid component pointer", GetName());
		return false;
	}

	wxString blockName = inBlockName;
	if(blockName == "")
	{
		LOGERROR(wxGetApp().GetLogger(), "Task \"%s\": Invalid block name", GetName());
		return false;
	}

	Xt::DomBlock* pBlock = Xt::AcquireBlock(&pComponent->data, blockName.c_str());

	C_CompositeVariable* pOutVar = new C_CompositeVariable(GetName());
	pOutVar->Add(new C_SimpleVariable(OUT_BLOCK_PTR, (void*)pBlock));

	GetOutput()->Add(pOutVar);

	return true;
}

} //namespace Dml
} //namespace DMan
