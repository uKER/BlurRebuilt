#ifndef DMAN_DML_NEW_FIELD_TASK_H
#define DMAN_DML_NEW_FIELD_TASK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <DialogueMan/Dml/Task.h>

namespace DMan
{
namespace Dml
{
	class C_NewFieldTask : public C_Task
	{
	public:
		C_NewFieldTask(const wxString& name);
		virtual ~C_NewFieldTask();
	
	public:
		virtual bool Execute();

	public:
		static const wxString OUT_FIELD_PTR;
	};

} //namespace Dml
} //namespace DMan

#endif //DMAN_DML_NEW_FIELD_TASK_H
