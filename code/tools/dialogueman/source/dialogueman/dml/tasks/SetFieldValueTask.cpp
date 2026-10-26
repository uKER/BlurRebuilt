#include "stdafx.h"

#include <DialogueMan/Dml/Tasks/SetFieldValueTask.h>
#include <DialogueMan/Dml/Variable.h>

#include <wx/tokenzr.h>

namespace DMan
{
namespace Dml
{

const wxString C_SetFieldValueTask::IN_FIELD_PTR   = "inFieldPtr";
const wxString C_SetFieldValueTask::IN_FIELD_VALUE = "inFieldValue";

C_SetFieldValueTask::C_SetFieldValueTask(const wxString& name) : 
	C_Task(name)
{}

C_SetFieldValueTask::~C_SetFieldValueTask()
{}

bool C_SetFieldValueTask::Execute()
{
	if(!GetInput())
	{
		LOGERROR(wxGetApp().GetLogger(), "Task \"%s\": No input assigned", GetName());
		return false;
	}
	
	C_MutantValue inFieldPtr   = GetInputVariable(IN_FIELD_PTR);
	C_MutantValue inFieldValue = GetInputVariable(IN_FIELD_VALUE);

	Xt::DomField* pField = (Xt::DomField*)(void*)inFieldPtr;
	if(!pField)
	{
		LOGERROR(wxGetApp().GetLogger(), "Task \"%s\": Invalid field pointer", GetName());
		return false;
	}

	wxString fieldValue = inFieldValue;

	return Xt::Set(pField, fieldValue.c_str());
}

} //namespace Dml
} //namespace DMan
