#include "stdafx.h"

#include <DialogueMan/Dml/Tasks/DivTask.h>
#include <DialogueMan/Dml/Variable.h>

#include <wx/tokenzr.h>

namespace DMan
{
namespace Dml
{

const wxString C_DivTask::IN_VAR_NAME = "inVarName";
const wxString C_DivTask::IN_VALUE = "inValue";

C_DivTask::C_DivTask(const wxString& name) : 
	C_Task(name)
{}

C_DivTask::~C_DivTask()
{}

bool C_DivTask::Execute()
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
		GetOutput()->Add(new C_SimpleVariable(nameOfVar, C_MutantValue(E_DataType_Double, 1.0))); 
	}
	else //add value
	{
		double prevVal = var.GetValue();
		double val     = value;

		if(val == 0.0)
		{
			LOGERROR(wxGetApp().GetLogger(), "Task \"%s\": Invalid denominator value of 0.0", GetName());
			return false;
		}

		double newVal  = prevVal / val;
		var.SetValue(newVal);
	}
	
	return true;
}

} //namespace Dml
} //namespace DMan
