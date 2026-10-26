#include "stdafx.h"

#include <DialogueMan/Dml/Tasks/RenameRecordTask.h>
#include <DialogueMan/Dml/Variable.h>

#include <Dialogueman/Misc/Utils.h>

#include <wx/tokenzr.h>

namespace DMan
{
namespace Dml
{

const wxString C_RenameRecordTask::IN_RECORD_PTR	  = "inRecordPtr";
const wxString C_RenameRecordTask::IN_NEW_RECORD_NAME = "inNewRecordName";

C_RenameRecordTask::C_RenameRecordTask(const wxString& name) : 
	C_Task(name)
{}

C_RenameRecordTask::~C_RenameRecordTask()
{}

bool C_RenameRecordTask::Execute()
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

	C_MutantValue inRecordPtr	  = GetInputVariable(IN_RECORD_PTR);
	C_MutantValue inNewRecordName = GetInputVariable(IN_NEW_RECORD_NAME);

	Xt::DomRecord* pRecord = (Xt::DomRecord*)(void*)inRecordPtr;
	if(!pRecord)
	{
		LOGERROR(wxGetApp().GetLogger(), "Task \"%s\": Invalid record pointer", GetName());
		return false;
	}
	
	wxString newRecordName = inNewRecordName;

	if(newRecordName == "")
	{
		LOGERROR(wxGetApp().GetLogger(), "Task \"%s\": Invalid new record name", GetName());
		return false;	
	}

	pRecord->name = newRecordName.c_str();
	Xt::SetDirty(pRecord, true);

	return true;
}

} //namespace Dml
} //namespace DMan
