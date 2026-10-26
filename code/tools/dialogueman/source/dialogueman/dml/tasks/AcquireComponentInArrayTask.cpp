#include "stdafx.h"

#include <DialogueMan/Dml/Tasks/AcquireComponentInArrayTask.h>
#include <DialogueMan/Dml/Variable.h>

#include <wx/tokenzr.h>

namespace DMan
{
namespace Dml
{

const wxString C_AcquireComponentInArrayTask::IN_ARRAY_PTR		= "inArrayPtr";
const wxString C_AcquireComponentInArrayTask::IN_INDEX			= "inIndex";
const wxString C_AcquireComponentInArrayTask::OUT_COMPONENT_PTR = "outComponentPtr";

C_AcquireComponentInArrayTask::C_AcquireComponentInArrayTask(const wxString& name) : 
	C_Task(name)
{}

C_AcquireComponentInArrayTask::~C_AcquireComponentInArrayTask()
{}

bool C_AcquireComponentInArrayTask::Execute()
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

	C_MutantValue inArrayPtr = GetInputVariable(IN_ARRAY_PTR);
	C_MutantValue inIndex	 = GetInputVariable(IN_INDEX);

	Xt::DomArray* pArray = (Xt::DomArray*)(void*)inArrayPtr;
	if(!pArray)
	{
		LOGERROR(wxGetApp().GetLogger(), "Task \"%s\": Invalid pArray pointer", GetName());
		return false;
	}

	int index = inIndex;
	if(index < 0)
	{
		LOGERROR(wxGetApp().GetLogger(), "Task \"%s\": Invalid index", GetName());
		return false;
	}

	Xt::DomComponent* pComponent = Xt::AcquireComponent(pArray, index);

	C_CompositeVariable* pOutVar = new C_CompositeVariable(GetName());
	pOutVar->Add(new C_SimpleVariable(OUT_COMPONENT_PTR, (void*)pComponent));

	GetOutput()->Add(pOutVar);

	return true;
}

} //namespace Dml
} //namespace DMan
