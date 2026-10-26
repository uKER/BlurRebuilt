#ifndef DMAN_DML_FOR_TASK_H
#define DMAN_DML_FOR_TASK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <DialogueMan/Dml/Task.h>

namespace DMan
{
namespace Dml
{
	class C_ForTask : public C_CompositeTask
	{
	public:
		C_ForTask(const wxString& name);
		virtual ~C_ForTask();
		
	public:
		virtual bool Execute();
		
	public:
		static const wxString IN_ITEMS;
		static const wxString IN_ITEM_VAR_NAME;
	};
	
} //namespace Dml
} //namespace DMan

#endif //DMAN_DML_FOR_TASK_H
