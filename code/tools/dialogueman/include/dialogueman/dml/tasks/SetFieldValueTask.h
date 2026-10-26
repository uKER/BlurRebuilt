#ifndef DMAN_DML_SET_FIELD_VALUE_TASK_H
#define DMAN_DML_SET_FIELD_VALUE_TASK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <DialogueMan/Dml/Task.h>

namespace DMan
{
namespace Dml
{
	class C_SetFieldValueTask : public C_Task
	{
	public:
		C_SetFieldValueTask(const wxString& name);
		virtual ~C_SetFieldValueTask();
	
	public:
		virtual bool Execute();

	public:
		static const wxString IN_FIELD_PTR;
		static const wxString IN_FIELD_VALUE;
	};

} //namespace Dml
} //namespace DMan

#endif //DMAN_DML_SET_FIELD_VALUE_TASK_H
