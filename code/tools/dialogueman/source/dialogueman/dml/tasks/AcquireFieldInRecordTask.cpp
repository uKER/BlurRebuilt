#include "stdafx.h"

#include <DialogueMan/Dml/Tasks/AcquireFieldInRecordTask.h>
#include <DialogueMan/Dml/Variable.h>

namespace DMan
{
namespace Dml
{

const wxString C_AcquireFieldInRecordTask::IN_RECORD_PTR = "inRecordPtr";
const wxString C_AcquireFieldInRecordTask::IN_FIELD_NAME = "inFieldName";
const wxString C_AcquireFieldInRecordTask::OUT_FIELD_PTR = "outFieldPtr";

C_AcquireFieldInRecordTask::C_AcquireFieldInRecordTask(const wxString& name) : 
	C_Task(name)
{}

C_AcquireFieldInRecordTask::~C_AcquireFieldInRecordTask()
{}

bool C_AcquireFieldInRecordTask::Execute()
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

	C_MutantValue inRecordPtr = GetInputVariable(IN_RECORD_PTR);
	C_MutantValue inFieldName = GetInputVariable(IN_FIELD_NAME);

	Xt::DomRecord* pRecord = (Xt::DomRecord*)(void*)inRecordPtr;
	if(!pRecord)
	{
		LOGERROR(wxGetApp().GetLogger(), "Task \"%s\": Invalid record pointer", GetName());
		return false;
	}

	wxString fieldName = inFieldName;
	if(fieldName == "")
	{
		LOGERROR(wxGetApp().GetLogger(), "Task \"%s\": Invalid field name", GetName());
		return false;
	}

	Xt::DomField* pField = Xt::AcquireField(&pRecord->root.data, fieldName.c_str());

	C_CompositeVariable* pOutVar = new C_CompositeVariable(GetName());
	pOutVar->Add(new C_SimpleVariable(OUT_FIELD_PTR, (void*)pField));

	GetOutput()->Add(pOutVar);

	return true;
}

} //namespace Dml
} //namespace DMan
