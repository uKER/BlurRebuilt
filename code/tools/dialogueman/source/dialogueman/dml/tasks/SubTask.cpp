#include "stdafx.h"

#include <DialogueMan/Dml/Tasks/SubTask.h>
#include <DialogueMan/Dml/Variable.h>

#include <wx/tokenzr.h>

namespace DMan
{
namespace Dml
{

const wxString C_SubTask::IN_VAR_NAME = "inVarName";
const wxString C_SubTask::IN_VALUE = "inValue";

C_SubTask::C_SubTask(const wxString& name) : 
	C_Task(name)
{}

C_SubTask::~C_SubTask()
{}

bool C_SubTask::Execute()
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

	wxString      nameOfVar  = GetInputVariable(IN_VAR_NAME);
	C_MutantValue value		 = GetInputVariable(IN_VALUE);

	C_Variable& var = C_Variable::SolveReference(nameOfVar, GetOutput(), false);
	
	if(var.GetName() == C_Variable::END_VAR.GetName()) //variable not found, add it
	{
		GetOutput()->Add(new C_SimpleVariable(nameOfVar, C_MutantValue(E_DataType_Double, 0.0))); 
	}
	else //sub value
	{
		double prevVal = var.GetValue();
		double val	   = value;
		double newVal  = prevVal - val;
		var.SetValue(newVal);
	}
	
	return true;
}

} //namespace Dml
} //namespace DMan
