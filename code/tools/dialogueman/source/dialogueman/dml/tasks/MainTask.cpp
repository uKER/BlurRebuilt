#include "stdafx.h"

#include <DialogueMan/Dml/Tasks/MainTask.h>
#include <DialogueMan/Dml/Variable.h>

#include <wx/tokenzr.h>

namespace DMan
{
namespace Dml
{

C_MainTask::C_MainTask(const wxString& name) : 
	C_CompositeTask(name)
{}

C_MainTask::~C_MainTask()
{}

bool C_MainTask::Execute()
{
	for(int taskIdx = 0; taskIdx < GetChildCount(); taskIdx++)
	{
		C_Task& task = *m_tasks[taskIdx];
		if(!task.Execute())
		{
			LOGERROR(wxGetApp().GetLogger(), 
				"Task \"%s\": Failed to execute subtask \"%s\"", 
				GetName(), task.GetName());
			return false;
		}
	}
	return true;
}

} //namespace Dml
} //namespace DMan
