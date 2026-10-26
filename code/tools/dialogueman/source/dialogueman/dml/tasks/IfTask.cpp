#include "stdafx.h"

#include <DialogueMan/Dml/Tasks/IfTask.h>
#include <DialogueMan/Dml/Variable.h>

#include <wx/tokenzr.h>

namespace DMan
{
namespace Dml
{

const wxString C_IfTask::IN_COMP = "inComp";
const wxString C_IfTask::IN_ARG1 = "inArg1";
const wxString C_IfTask::IN_ARG2 = "inArg2";

C_IfTask::C_IfTask(const wxString& name) : 
	C_CompositeTask(name)
{}

C_IfTask::~C_IfTask()
{}

bool C_IfTask::Execute()
{
	if(!GetInput())
	{
		LOGERROR(wxGetApp().GetLogger(), "Task \"%s\": No input assigned", GetName());
		return false;
	}

	C_Variable& in = *GetInput();

	wxString	  inComp = GetInputVariable(IN_COMP); 
	C_MutantValue inArg1 = GetInputVariable(IN_ARG1);
	C_MutantValue inArg2 = GetInputVariable(IN_ARG2);
	
	for(int taskIdx = 0; taskIdx < GetChildCount(); taskIdx++)
	{
		C_Task& task = *m_tasks[taskIdx];

		//Copy Extra Input variables added to this task (those added by parent composite task)
		for(int inIdx = 0; inIdx < in.GetChildCount(); inIdx++)
		{
			C_Variable& inVar = in[inIdx];
			if(inVar.GetName() != IN_COMP &&
			   inVar.GetName() != IN_ARG1 && 
			   inVar.GetName() != IN_ARG2 )
			{
				C_CompositeVariable* pInVarCopy = new C_CompositeVariable(inVar.GetName());
				for(int inVarChildIdx = 0; inVarChildIdx < inVar.GetChildCount(); inVarChildIdx++)
				{
					pInVarCopy->Add(&inVar[inVarChildIdx], false);
				}

				C_Variable* pTaskInput = task.GetInput();
				pTaskInput->Add(pInVarCopy);
			}
		}

		if(inComp == "equal")
		{
			if(inArg1 == inArg2)
			{
				if(!task.Execute())
				{
					LOGERROR(wxGetApp().GetLogger(), 
						"Task \"%s\": Failed to execute subtask \"%s\"", 
						GetName(), task.GetName());
					return false;
				}	
			}
		}
		else if(inComp == "notEqual")
		{
			if(inArg1 != inArg2)
			{
				if(!task.Execute())
				{
					LOGERROR(wxGetApp().GetLogger(), 
						"Task \"%s\": Failed to execute subtask \"%s\"", 
						GetName(), task.GetName());
					return false;
				}	
			}
		}
		else if(inComp == "greaterThan")
		{
			if(inArg1 > inArg2)
			{
				if(!task.Execute())
				{
					LOGERROR(wxGetApp().GetLogger(), 
						"Task \"%s\": Failed to execute subtask \"%s\"", 
						GetName(), task.GetName());
					return false;
				}	
			}
		}
		else if(inComp == "greaterOrEqualThan")
		{
			if(inArg1 >= inArg2)
			{
				if(!task.Execute())
				{
					LOGERROR(wxGetApp().GetLogger(), 
						"Task \"%s\": Failed to execute subtask \"%s\"", 
						GetName(), task.GetName());
					return false;
				}	
			}
		}
		else if(inComp == "lessThan")
		{
			if(inArg1 < inArg2)
			{
				if(!task.Execute())
				{
					LOGERROR(wxGetApp().GetLogger(), 
						"Task \"%s\": Failed to execute subtask \"%s\"", 
						GetName(), task.GetName());
					return false;
				}	
			}
		}
		else if(inComp == "lessOrEqualThan")
		{
			if(inArg1 <= inArg2)
			{
				if(!task.Execute())
				{
					LOGERROR(wxGetApp().GetLogger(), 
						"Task \"%s\": Failed to execute subtask \"%s\"", 
						GetName(), task.GetName());
					return false;
				}	
			}
		}
		else
		{
			LOGERROR(wxGetApp().GetLogger(), "Task \"%s\": Invalid comparison operator", GetName());
			return false;
		}
	}

	return true;
}

} //namespace Dml
} //namespace DMan
