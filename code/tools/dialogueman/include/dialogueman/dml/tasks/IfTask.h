#ifndef DMAN_DML_IF_TASK_H
#define DMAN_DML_IF_TASK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <DialogueMan/Dml/Task.h>

namespace DMan
{
namespace Dml
{
	class C_IfTask : public C_CompositeTask
	{
	public:
		C_IfTask(const wxString& name);
		virtual ~C_IfTask();
		
	public:
		virtual bool Execute();
		
	public:
		static const wxString IN_COMP;
		static const wxString IN_ARG1;
		static const wxString IN_ARG2;
	};

} //namespace Dml
} //namespace DMan

#endif //DMAN_DML_IF_TASK_H
