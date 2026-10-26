#include "stdafx.h"

#include <DialogueMan/Dml/Tasks/AcquireArrayInComponentTask.h>
#include <DialogueMan/Dml/Variable.h>

#include <wx/tokenzr.h>

namespace DMan
{
namespace Dml
{

const wxString C_AcquireArrayInComponentTask::IN_COMPONENT_PTR = "inComponentPtr";
const wxString C_AcquireArrayInComponentTask::IN_ARRAY_NAME = "inArrayName";
const wxString C_AcquireArrayInComponentTask::OUT_ARRAY_PTR = "outArrayPtr";

C_AcquireArrayInComponentTask::C_AcquireArrayInComponentTask(const wxString& name) : 
	C_Task(name)
{}

C_AcquireArrayInComponentTask::~C_AcquireArrayInComponentTask()
{}

bool C_AcquireArrayInComponentTask::Execute()
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
	C_MutantValue inArrayName = GetInputVariable(IN_ARRAY_NAME);
	
	Xt::DomComponent* pComponent = (Xt::DomComponent*)(void*)inComponentPtr;
	if(!pComponent)
	{
		LOGERROR(wxGetApp().GetLogger(), "Task \"%s\": Invalid component pointer", GetName());
		return false;
	}

	wxString arrayName = inArrayName;
	if(arrayName == "")
	{
		LOGERROR(wxGetApp().GetLogger(), "Task \"%s\": Invalid array name", GetName());
		return false;
	}

	Xt::DomArray* pArray = Xt::AcquireArray(&pComponent->data, arrayName.c_str());

	C_CompositeVariable* pOutVar = new C_CompositeVariable(GetName());
	pOutVar->Add(new C_SimpleVariable(OUT_ARRAY_PTR, (void*)pArray));

	GetOutput()->Add(pOutVar);

	return true;
}

} //namespace Dml
} //namespace DMan
