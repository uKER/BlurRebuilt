#ifndef DMAN_DML_APPEND_TASK_H
#define DMAN_DML_APPEND_TASK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <DialogueMan/Dml/Task.h>

namespace DMan
{
namespace Dml
{
	class C_AppendTask : public C_Task
	{
	public:
		C_AppendTask(const wxString& name);
		virtual ~C_AppendTask();
	
	public:
		virtual bool Execute();

	public:
		static const wxString IN_ARRAY_PTR;
		static const wxString IN_ELEMENT_PTR;
	};

} //namespace Dml
} //namespace DMan

#endif //DMAN_DML_APPEND_TASK_H
