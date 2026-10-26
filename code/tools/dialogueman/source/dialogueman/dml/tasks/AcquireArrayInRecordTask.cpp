#include "stdafx.h"

#include <DialogueMan/Dml/Tasks/AcquireArrayInRecordTask.h>
#include <DialogueMan/Dml/Variable.h>

#include <wx/tokenzr.h>

namespace DMan
{
namespace Dml
{

const wxString C_AcquireArrayInRecordTask::IN_RECORD_PTR = "inRecordPtr";
const wxString C_AcquireArrayInRecordTask::IN_ARRAY_NAME = "inArrayName";
const wxString C_AcquireArrayInRecordTask::OUT_ARRAY_PTR = "outArrayPtr";

C_AcquireArrayInRecordTask::C_AcquireArrayInRecordTask(const wxString& name) : 
	C_Task(name)
{}

C_AcquireArrayInRecordTask::~C_AcquireArrayInRecordTask()
{}

bool C_AcquireArrayInRecordTask::Execute()
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
	C_MutantValue inArrayName = GetInputVariable(IN_ARRAY_NAME);
	
	Xt::DomRecord* pRecord = (Xt::DomRecord*)(void*)inRecordPtr;
	if(!pRecord)
	{
		LOGERROR(wxGetApp().GetLogger(), "Task \"%s\": Invalid record pointer", GetName());
		return false;
	}

	wxString arrayName = inArrayName;
	if(arrayName == "")
	{
		LOGERROR(wxGetApp().GetLogger(), "Task \"%s\": Invalid array name", GetName());
		return false;
	}

	Xt::DomArray* pArray = Xt::AcquireArray(&pRecord->root.data, arrayName.c_str());

	C_CompositeVariable* pOutVar = new C_CompositeVariable(GetName());
	pOutVar->Add(new C_SimpleVariable(OUT_ARRAY_PTR, (void*)pArray));

	GetOutput()->Add(pOutVar);
	
	return true;
}

} //namespace Dml
} //namespace DMan
