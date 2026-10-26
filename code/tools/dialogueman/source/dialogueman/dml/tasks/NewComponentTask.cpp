#include "stdafx.h"

#include <DialogueMan/Dml/Tasks/NewComponentTask.h>
#include <DialogueMan/Dml/Variable.h>

#include <wx/tokenzr.h>

namespace DMan
{
namespace Dml
{

const wxString C_NewComponentTask::OUT_COMPONENT_PTR = "outComponentPtr";

C_NewComponentTask::C_NewComponentTask(const wxString& name) : 
	C_Task(name)
{}

C_NewComponentTask::~C_NewComponentTask()
{}

bool C_NewComponentTask::Execute()
{
	if(!GetOutput())
	{
		LOGERROR(wxGetApp().GetLogger(), "Task \"%s\": No output assigned", GetName());
		return false;
	}
	
	Xt::DomComponent* pComponent = new Xt::DomComponent;
	GenerateIdForComponent(pComponent, false);

	C_CompositeVariable* pOutVar = new C_CompositeVariable(GetName());
	pOutVar->Add(new C_SimpleVariable(OUT_COMPONENT_PTR, (void*)pComponent));

	GetOutput()->Add(pOutVar);
	return true;
}

} //namespace Dml
} //namespace DMan
