#ifndef DMAN_DML_ACQUIRE_ARRAY_IN_RECORD_TASK_H
#define DMAN_DML_ACQUIRE_ARRAY_IN_RECORD_TASK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <DialogueMan/Dml/Task.h>

namespace DMan
{
namespace Dml
{
	class C_AcquireArrayInRecordTask : public C_Task
	{
	public:
		C_AcquireArrayInRecordTask(const wxString& name);
		virtual ~C_AcquireArrayInRecordTask();
	
	public:
		virtual bool Execute();

	public:
		static const wxString IN_RECORD_PTR;
		static const wxString IN_ARRAY_NAME;
		static const wxString OUT_ARRAY_PTR;
	};
	
} //namespace Dml
} //namespace DMan

#endif //DMAN_DML_ACQUIRE_ARRAY_IN_RECORD_TASK_H
