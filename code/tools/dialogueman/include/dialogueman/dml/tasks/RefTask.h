#ifndef DMAN_DML_REF_TASK_H
#define DMAN_DML_REF_TASK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <DialogueMan/Dml/Task.h>

namespace DMan
{
namespace Dml
{
	class C_RefTask : public C_Task
	{
	public:
		C_RefTask(const wxString& name);
		virtual ~C_RefTask();
	
	public:
		virtual bool Execute();

	public:
		static const wxString IN_VAR_NAME;
		static const wxString OUT_VAR;
	};

} //namespace Dml
} //namespace DMan

#endif //DMAN_DML_REF_TASK_H
