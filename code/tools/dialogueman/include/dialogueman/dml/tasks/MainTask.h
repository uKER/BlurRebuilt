#ifndef DMAN_DML_MAIN_TASK_H
#define DMAN_DML_MAIN_TASK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <DialogueMan/Dml/Task.h>

namespace DMan
{
namespace Dml
{
	class C_MainTask : public C_CompositeTask
	{
	public:
		C_MainTask(const wxString& name);
		virtual ~C_MainTask();
		
	public:
		virtual bool Execute();
	};

} //namespace Dml
} //namespace DMan

#endif //DMAN_DML_MAIN_TASK_H
