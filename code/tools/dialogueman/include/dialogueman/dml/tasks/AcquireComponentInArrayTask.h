#ifndef DMAN_DML_ACQUIRE_COMPONENT_IN_ARRAY_TASK_H
#define DMAN_DML_ACQUIRE_COMPONENT_IN_ARRAY_TASK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <DialogueMan/Dml/Task.h>

namespace DMan
{
namespace Dml
{
	class C_AcquireComponentInArrayTask : public C_Task
	{
	public:
		C_AcquireComponentInArrayTask(const wxString& name);
		virtual ~C_AcquireComponentInArrayTask();
	
	public:
		virtual bool Execute();

	public:
		static const wxString IN_ARRAY_PTR;
		static const wxString IN_INDEX;
		static const wxString OUT_COMPONENT_PTR;
	};

} //namespace Dml
} //namespace DMan

#endif //DMAN_DML_ACQUIRE_COMPONENT_IN_ARRAY_TASK_H