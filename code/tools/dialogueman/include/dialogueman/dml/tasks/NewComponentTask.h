#ifndef DMAN_DML_NEW_COMPONENT_TASK_H
#define DMAN_DML_NEW_COMPONENT_TASK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <DialogueMan/Dml/Task.h>

namespace DMan
{
namespace Dml
{
	class C_NewComponentTask : public C_Task
	{
	public:
		C_NewComponentTask(const wxString& name);
		virtual ~C_NewComponentTask();
	
	public:
		virtual bool Execute();

	public:
		static const wxString OUT_COMPONENT_PTR;
	};

} //namespace Dml
} //namespace DMan

#endif //DMAN_DML_NEW_COMPONENT_TASK_H
