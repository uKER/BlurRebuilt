#include "stdafx.h"

#include <DialogueMan/Dml/Tasks/AppendTask.h>
#include <DialogueMan/Dml/Variable.h>

#include <wx/tokenzr.h>

namespace DMan
{
namespace Dml
{

const wxString C_AppendTask::IN_ARRAY_PTR = "inArrayPtr";
const wxString C_AppendTask::IN_ELEMENT_PTR = "inElementPtr";

C_AppendTask::C_AppendTask(const wxString& name) : 
	C_Task(name)
{}

C_AppendTask::~C_AppendTask()
{}

bool C_AppendTask::Execute()
{
	if(!GetInput())
	{
		LOGERROR(wxGetApp().GetLogger(), "Task \"%s\": No input assigned", GetName());
		return false;
	}
	
	C_MutantValue inArrayPtr = GetInputVariable(IN_ARRAY_PTR);
	C_MutantValue inElementPtr = GetInputVariable(IN_ELEMENT_PTR);
	
	Xt::DomArray* pArray = (Xt::DomArray*)(void*)inArrayPtr;
	if(!pArray)
	{
		LOGERROR(wxGetApp().GetLogger(), "Task \"%s\": Invalid array pointer", GetName());
		return false;
	}

	Xt::DomElement* pElement = (Xt::DomElement*)(void*)inElementPtr;
	if(!pElement)
	{
		LOGERROR(wxGetApp().GetLogger(), "Task \"%s\": Invalid element pointer", GetName());
		return false;
	}

	Xt::Append(pArray, pElement);
	
	return true;
}

} //namespace Dml
} //namespace DMan
