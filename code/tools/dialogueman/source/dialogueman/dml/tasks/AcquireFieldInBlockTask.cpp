#include "stdafx.h"

#include <DialogueMan/Dml/Tasks/AcquireFieldInBlockTask.h>
#include <DialogueMan/Dml/Variable.h>

#include <wx/tokenzr.h>

namespace DMan
{
namespace Dml
{

const wxString C_AcquireFieldInBlockTask::IN_BLOCK_PTR  = "inBlockPtr";
const wxString C_AcquireFieldInBlockTask::IN_FIELD_NAME = "inFieldName";
const wxString C_AcquireFieldInBlockTask::OUT_FIELD_PTR = "outFieldPtr";

C_AcquireFieldInBlockTask::C_AcquireFieldInBlockTask(const wxString& name) : 
	C_Task(name)
{}

C_AcquireFieldInBlockTask::~C_AcquireFieldInBlockTask()
{}

bool C_AcquireFieldInBlockTask::Execute()
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

	C_MutantValue inBlockPtr = GetInputVariable(IN_BLOCK_PTR);
	C_MutantValue inFieldName = GetInputVariable(IN_FIELD_NAME);

	Xt::DomBlock* pBlock = (Xt::DomBlock*)(void*)inBlockPtr;
	if(!pBlock)
	{
		LOGERROR(wxGetApp().GetLogger(), "Task \"%s\": Invalid block pointer", GetName());
		return false;
	}

	wxString fieldName = inFieldName;
	if(fieldName == "")
	{
		LOGERROR(wxGetApp().GetLogger(), "Task \"%s\": Invalid field name", GetName());
		return false;
	}

	Xt::DomField* pField = Xt::AcquireField(pBlock, fieldName.c_str());

	C_CompositeVariable* pOutVar = new C_CompositeVariable(GetName());
	pOutVar->Add(new C_SimpleVariable(OUT_FIELD_PTR, (void*)pField));

	GetOutput()->Add(pOutVar);

	return true;
}

} //namespace Dml
} //namespace DMan
