#ifndef DMAN_DML_ACQUIRE_FIELD_IN_ARRAY_TASK_H
#define DMAN_DML_ACQUIRE_FIELD_IN_ARRAY_TASK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <DialogueMan/Dml/Task.h>

namespace DMan
{
namespace Dml
{
	class C_AcquireFieldInArrayTask : public C_Task
	{
	public:
		C_AcquireFieldInArrayTask(const wxString& name);
		virtual ~C_AcquireFieldInArrayTask();
	
	public:
		virtual bool Execute();

	public:
		static const wxString IN_ARRAY_PTR;
		static const wxString IN_INDEX;
		static const wxString OUT_FIELD_PTR;
	};

} //namespace Dml
} //namespace DMan

#endif //DMAN_DML_ACQUIRE_FIELD_IN_ARRAY_TASK_H