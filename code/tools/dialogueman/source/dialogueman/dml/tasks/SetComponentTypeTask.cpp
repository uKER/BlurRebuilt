#include "stdafx.h"

#include <DialogueMan/Dml/Tasks/SetComponentTypeTask.h>
#include <DialogueMan/Dml/Variable.h>

#include <wx/tokenzr.h>

namespace DMan
{
namespace Dml
{

const wxString C_SetComponentTypeTask::IN_COMPONENT_PTR  = "inComponentPtr";
const wxString C_SetComponentTypeTask::IN_COMPONENT_TYPE = "inComponentType";

C_SetComponentTypeTask::C_SetComponentTypeTask(const wxString& name) : 
	C_Task(name)
{}

C_SetComponentTypeTask::~C_SetComponentTypeTask()
{}

bool C_SetComponentTypeTask::Execute()
{
	if(!GetInput())
	{
		LOGERROR(wxGetApp().GetLogger(), "Task \"%s\": No input assigned", GetName());
		return false;
	}

	C_MutantValue inComponentPtr  = GetInputVariable(IN_COMPONENT_PTR);
	C_MutantValue inComponentType = GetInputVariable(IN_COMPONENT_TYPE);
	
	Xt::DomComponent* pComponent = (Xt::DomComponent*)(void*)inComponentPtr;
	if(!pComponent)
	{
		LOGERROR(wxGetApp().GetLogger(), "Task \"%s\": Invalid component pointer", GetName());
		return false;
	}

	wxString componentType = inComponentType;
	if(componentType == "")
	{
		LOGERROR(wxGetApp().GetLogger(), "Task \"%s\": Invalid component type name", GetName());
		return false;
	}

	return Xt::SetType(pComponent, componentType.c_str() );
}

} //namespace Dml
} //namespace DMan
