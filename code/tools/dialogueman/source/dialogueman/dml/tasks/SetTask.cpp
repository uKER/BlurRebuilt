#include "stdafx.h"

#include <DialogueMan/Dml/Tasks/SetTask.h>
#include <DialogueMan/Dml/Variable.h>

#include <wx/tokenzr.h>

namespace DMan
{
namespace Dml
{

const wxString C_SetTask::IN_VAR_NAME = "inVarName";
const wxString C_SetTask::IN_VALUE = "inValue";

C_SetTask::C_SetTask(const wxString& name) : 
	C_Task(name)
{}

C_SetTask::~C_SetTask()
{}

bool C_SetTask::Execute()
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

	wxString      nameOfVarToSet = GetInputVariable(IN_VAR_NAME);
	C_MutantValue valueToSet	 = GetInputVariable(IN_VALUE);

	C_Variable& varToSet = C_Variable::SolveReference(nameOfVarToSet, GetOutput(), false);
	
	if(varToSet.GetName() == C_Variable::END_VAR.GetName()) //variable not found, add it
	{
		GetOutput()->Add(new C_SimpleVariable(nameOfVarToSet, valueToSet)); 
	}
	else //reset value
	{
		varToSet.SetValue(valueToSet);
	}
	
	return true;
}

} //namespace Dml
} //namespace DMan
