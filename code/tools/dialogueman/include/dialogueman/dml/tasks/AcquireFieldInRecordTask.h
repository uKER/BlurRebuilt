#ifndef DMAN_DML_ACQUIRE_FIELD_IN_RECORD_TASK_H
#define DMAN_DML_ACQUIRE_FIELD_IN_RECORD_TASK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <DialogueMan/Dml/Task.h>

namespace DMan
{
namespace Dml
{
	class C_AcquireFieldInRecordTask : public C_Task
	{
	public:
		C_AcquireFieldInRecordTask(const wxString& name);
		virtual ~C_AcquireFieldInRecordTask();
	
	public:
		virtual bool Execute();

	public:
		static const wxString IN_RECORD_PTR;
		static const wxString IN_FIELD_NAME;
		static const wxString OUT_FIELD_PTR;
	};

} //namespace Dml
} //namespace DMan

#endif //DMAN_DML_ACQUIRE_FIELD_IN_RECORD_TASK_H
