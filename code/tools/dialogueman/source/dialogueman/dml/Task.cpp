#include "stdafx.h"

#include <DialogueMan/Dml/Task.h>
#include <DialogueMan/Dml/Variable.h>

#include <wx/tokenzr.h>

namespace DMan
{
namespace Dml
{

/////////////////////////// C_Task \\\\\\\\\\\\\\\\\\\\\\\\\\\

C_Task::C_Task(const wxString& name) :
	m_name(name),
	m_pInput(NULL),
	m_pOutput(NULL),
	m_pGlobals(NULL),
	m_takeInputOwnership(false),
	m_takeOutputOwnership(false)
{}

C_Task::~C_Task()
{
	if(m_takeInputOwnership)
		delete m_pInput;

	if(m_takeOutputOwnership)
		delete m_pOutput;
}

wxString C_Task::GetName() const
{
	return m_name;
}

void C_Task::SetInput(C_Variable* pInput, bool takeOwnership)
{
	m_pInput = pInput;
	m_takeInputOwnership = takeOwnership;
}

C_Variable* C_Task::GetInput()
{
	return m_pInput;
}

void C_Task::SetOutput(C_Variable* pOutput, bool takeOwnership)
{
	m_pOutput = pOutput;
	m_takeOutputOwnership = takeOwnership;
}

C_Variable* C_Task::GetOutput()
{
	return m_pOutput;
}

void C_Task::SetGlobals(C_Variable* pGlobals)
{
	m_pGlobals = pGlobals;
}

C_Variable* C_Task::GetGlobals()
{
	return m_pGlobals;
}

void C_Task::Add(C_Task* pTask)
{}

void C_Task::Delete(const wxString& name)
{}

int C_Task::GetChildCount()
{
	return 0;
}

C_Task& C_Task::FindChildTask(const wxString& name)
{
	return *this;
}

int C_Task::FindChildTaskIndex(const wxString& name)
{
	return 0;
}

C_Task& C_Task::operator[](int index)
{
	return *this;
}

C_Task& C_Task::operator[](const wxString& name)
{
	return *this;
}

C_Variable& C_Task::GetInputVariable(const wxString& inputName)
{
	C_Variable& in = *GetInput();
	C_Variable& retVal = in[inputName];

	if(retVal.GetValue() == C_Variable::END_VAR.GetValue() && GetGlobals())
	{
		C_Variable* pPrevContext = in.GetContext();
		in.SetContext(GetGlobals());
		C_Variable& retValGlobal = in[inputName];
		in.SetContext(pPrevContext);
		return retValGlobal;
	}

	return retVal;
}


/////////////////////////// C_NullTask \\\\\\\\\\\\\\\\\\\\\\\\\\\

C_NullTask::C_NullTask(const wxString& name) : 
	C_Task(name)
{}

C_NullTask::~C_NullTask()
{}

bool C_NullTask::Execute()
{
	return true;
}

/////////////////////////// C_CompositeTask \\\\\\\\\\\\\\\\\\\\\\\\\\\

C_CompositeTask::C_CompositeTask(const wxString& name) :
	C_Task(name),
	m_nullTask("__NULLTASK"),
	m_endTask("__ENDTASK")
{}

C_CompositeTask::~C_CompositeTask()
{
	for(int i = 0; i < m_tasks.Size(); i++)
	{
		delete m_tasks[i];
	}
}

void C_CompositeTask::Add(C_Task* pTask)
{
	Delete(pTask->GetName()); //delete if previously added
	m_tasks.PushBack(pTask);
}

void C_CompositeTask::Delete(const wxString& name)
{
	int varPos = FindChildTaskIndex(name);
	if(varPos >= 0 && varPos < m_tasks.Size())
	{
		delete m_tasks[varPos]; 
		m_tasks.EraseByIndex(varPos);
	}
}

int C_CompositeTask::GetChildCount()
{
	return m_tasks.Size();
}

C_Task& C_CompositeTask::FindChildTask(const wxString& name)
{
	for(int i = 0; i < m_tasks.Size(); i++)
	{
		if(m_tasks[i]->GetName() == name)
		{
			return *m_tasks[i];
		}
	}
	return m_endTask;
}

int C_CompositeTask::FindChildTaskIndex(const wxString& name)
{
	for(int i = 0; i < m_tasks.Size(); i++)
	{
		if(m_tasks[i]->GetName() == name)
		{
			return i;
		}
	}
	return -1;
}


C_Task& C_CompositeTask::operator[](int index)
{
	if(index >= 0 && index < m_tasks.Size())
	{
		C_Task* pTaskAtIndex = m_tasks[index];
		return *pTaskAtIndex;
	}
	return m_endTask;
}

C_Task& C_CompositeTask::operator[](const wxString& name)
{
	C_Task& found = FindChildTask(name);
	return found;
}

} //namespace Dml
} //namespace DMan
