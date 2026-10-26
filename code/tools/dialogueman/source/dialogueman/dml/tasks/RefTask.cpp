#include "stdafx.h"

#include <DialogueMan/Dml/Tasks/RefTask.h>
#include <DialogueMan/Dml/Variable.h>

#include <wx/tokenzr.h>

namespace DMan
{
namespace Dml
{

const wxString C_RefTask::IN_VAR_NAME   = "inVarName";
const wxString C_RefTask::OUT_VAR		= "outVar";

C_RefTask::C_RefTask(const wxString& name) : 
	C_Task(name)
{}

C_RefTask::~C_RefTask()
{}

bool C_RefTask::Execute()
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

	wxString inVarName = GetInputVariable(IN_VAR_NAME);
	C_Variable& refVar = C_Variable::SolveReference(inVarName, GetOutput(), false);

	//Copy refVar
	C_Variable* pRefVarCopy = NULL;
	if(refVar.GetChildCount() == 0)
	{
		pRefVarCopy = new C_SimpleVariable(OUT_VAR, refVar.GetValue());
	}
	else
	{
		pRefVarCopy = new C_CompositeVariable(OUT_VAR);
		for(int i = 0; i < refVar.GetChildCount(); i++)
		{
			pRefVarCopy->Add(&refVar[i], false);
		}
	}
	
	C_CompositeVariable* pOutVar = new C_CompositeVariable(GetName());
	pOutVar->Add(pRefVarCopy);

	GetOutput()->Add(pOutVar);
	
	return true;
}

} //namespace Dml
} //namespace DMan
