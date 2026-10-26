#ifndef DMAN_DML_ACQUIRE_BLOCK_IN_COMPONENT_TASK_H
#define DMAN_DML_ACQUIRE_BLOCK_IN_COMPONENT_TASK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <DialogueMan/Dml/Task.h>

namespace DMan
{
namespace Dml
{
	class C_AcquireBlockInComponentTask : public C_Task
	{
	public:
		C_AcquireBlockInComponentTask(const wxString& name);
		virtual ~C_AcquireBlockInComponentTask();
	
	public:
		virtual bool Execute();

	public:
		static const wxString IN_COMPONENT_PTR;
		static const wxString IN_BLOCK_NAME;
		static const wxString OUT_BLOCK_PTR;
	};

} //namespace Dml
} //namespace DMan

#endif //DMAN_DML_ACQUIRE_BLOCK_IN_COMPONENT_TASK_H
