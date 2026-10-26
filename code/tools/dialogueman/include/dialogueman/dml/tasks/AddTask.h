#ifndef DMAN_DML_ADD_TASK_H
#define DMAN_DML_ADD_TASK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <DialogueMan/Dml/Task.h>

namespace DMan
{
namespace Dml
{
	class C_AddTask : public C_Task
	{
	public:
		C_AddTask(const wxString& name);
		virtual ~C_AddTask();
	
	public:
		virtual bool Execute();

	public:
		static const wxString IN_VAR_NAME;
		static const wxString IN_VALUE;
	};

} //namespace Dml
} //namespace DMan

#endif //DMAN_DML_ADD_TASK_H
