#include "stdafx.h"

#include <DialogueMan/Dml/Tasks/ForTask.h>
#include <DialogueMan/Dml/Variable.h>

#include <wx/tokenzr.h>

namespace DMan
{
namespace Dml
{

const wxString C_ForTask::IN_ITEMS			= "inItems";
const wxString C_ForTask::IN_ITEM_VAR_NAME  = "inItemVarName";

C_ForTask::C_ForTask(const wxString& name) : 
	C_CompositeTask(name)
{}

C_ForTask::~C_ForTask()
{}

bool C_ForTask::Execute()
{
	if(!GetInput())
	{
		LOGERROR(wxGetApp().GetLogger(), "Task \"%s\": No input assigned", GetName());
		return false;
	}

	C_Variable& in = *GetInput();

	wxString inItems = in[IN_ITEMS].GetValue(false);
	C_Variable& itemsVar = C_Variable::SolveReference(inItems, GetGlobals(), true);

	wxString itemVarName = GetInputVariable(IN_ITEM_VAR_NAME);

	for(int itemIdx = 0; itemIdx < itemsVar.GetChildCount(); itemIdx++)
	{
		C_Variable& itemVar = itemsVar[itemIdx];

		for(int taskIdx = 0; taskIdx < GetChildCount(); taskIdx++)
		{
			C_Task& task = *m_tasks[taskIdx];
			C_Variable* pTaskInput = task.GetInput();

			//Copy Extra Input variables added to this task (those added by parent composite task)
			for(int inIdx = 0; inIdx < in.GetChildCount(); inIdx++)
			{
				C_Variable& inVar = in[inIdx];
				if(inVar.GetName() != IN_ITEMS &&
				   inVar.GetName() != IN_ITEM_VAR_NAME)
				{
					C_CompositeVariable* pInVarCopy = new C_CompositeVariable(inVar.GetName());
					for(int inVarChildIdx = 0; inVarChildIdx < inVar.GetChildCount(); inVarChildIdx++)
					{
						pInVarCopy->Add(&inVar[inVarChildIdx], false);
					}

					pTaskInput->Add(pInVarCopy);
				}
			}
			
			//Copy ItemVar variables
			C_Variable* pItemVarCopy = NULL;
			if(itemVar.GetChildCount() == 0)
			{
				pItemVarCopy = new C_SimpleVariable(itemVarName, itemVar.GetValue());
			}
			else
			{
				pItemVarCopy = new C_CompositeVariable(itemVarName);
				for(int i = 0; i < itemVar.GetChildCount(); i++)
				{
					pItemVarCopy->Add(&itemVar[i], false);
				}
			}
						
			pTaskInput->Add(pItemVarCopy);

			if(!task.Execute())
			{
				LOGERROR(wxGetApp().GetLogger(), 
					"Task \"%s\": Failed to execute subtask \"%s\" on item \"%s\"", 
					GetName(), task.GetName(), itemVar.GetName());
				return false;
			}
		}
	}

	return true;
}

} //namespace Dml
} //namespace DMan
