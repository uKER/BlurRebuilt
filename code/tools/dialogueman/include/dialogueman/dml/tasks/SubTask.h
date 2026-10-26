#ifndef DMAN_DML_SUB_TASK_H
#define DMAN_DML_SUB_TASK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <DialogueMan/Dml/Task.h>

namespace DMan
{
namespace Dml
{
	class C_SubTask : public C_Task
	{
	public:
		C_SubTask(const wxString& name);
		virtual ~C_SubTask();
	
	public:
		virtual bool Execute();

	public:
		static const wxString IN_VAR_NAME;
		static const wxString IN_VALUE;
	};

} //namespace Dml
} //namespace DMan

#endif //DMAN_DML_SUB_TASK_H
