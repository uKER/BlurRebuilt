#include "stdafx.h"

#include <DialogueMan/Dml/Tasks/AcquireFieldInComponentTask.h>
#include <DialogueMan/Dml/Variable.h>

#include <wx/tokenzr.h>

namespace DMan
{
namespace Dml
{

const wxString C_AcquireFieldInComponentTask::IN_COMPONENT_PTR = "inComponentPtr";
const wxString C_AcquireFieldInComponentTask::IN_FIELD_NAME = "inFieldName";
const wxString C_AcquireFieldInComponentTask::OUT_FIELD_PTR = "outFieldPtr";

C_AcquireFieldInComponentTask::C_AcquireFieldInComponentTask(const wxString& name) : 
	C_Task(name)
{}

C_AcquireFieldInComponentTask::~C_AcquireFieldInComponentTask()
{}

bool C_AcquireFieldInComponentTask::Execute()
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
	C_MutantValue inFieldName = GetInputVariable(IN_FIELD_NAME);

	Xt::DomComponent* pComponent = (Xt::DomComponent*)(void*)inComponentPtr;
	if(!pComponent)
	{
		LOGERROR(wxGetApp().GetLogger(), "Task \"%s\": Invalid component pointer", GetName());
		return false;
	}

	wxString fieldName = inFieldName;
	if(fieldName == "")
	{
		LOGERROR(wxGetApp().GetLogger(), "Task \"%s\": Invalid field name", GetName());
		return false;
	}

	Xt::DomField* pField = Xt::AcquireField(&pComponent->data, fieldName.c_str());

	C_CompositeVariable* pOutVar = new C_CompositeVariable(GetName());
	pOutVar->Add(new C_SimpleVariable(OUT_FIELD_PTR, (void*)pField));

	GetOutput()->Add(pOutVar);

	return true;
}

} //namespace Dml
} //namespace DMan
