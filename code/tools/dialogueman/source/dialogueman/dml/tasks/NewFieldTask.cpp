#include "stdafx.h"

#include <DialogueMan/Dml/Tasks/NewFieldTask.h>
#include <DialogueMan/Dml/Variable.h>

#include <wx/tokenzr.h>

namespace DMan
{
namespace Dml
{

const wxString C_NewFieldTask::OUT_FIELD_PTR = "outFieldPtr";

C_NewFieldTask::C_NewFieldTask(const wxString& name) : 
	C_Task(name)
{}

C_NewFieldTask::~C_NewFieldTask()
{}

bool C_NewFieldTask::Execute()
{
	if(!GetOutput())
	{
		LOGERROR(wxGetApp().GetLogger(), "Task \"%s\": No output assigned", GetName());
		return false;
	}
	
	Xt::DomField* pField = new Xt::DomField();
	
	C_CompositeVariable* pOutVar = new C_CompositeVariable(GetName());
	pOutVar->Add(new C_SimpleVariable(OUT_FIELD_PTR, (void*)pField));

	GetOutput()->Add(pOutVar);
	return true;
}

} //namespace Dml
} //namespace DMan
