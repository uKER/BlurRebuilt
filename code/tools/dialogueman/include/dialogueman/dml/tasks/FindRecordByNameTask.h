#ifndef DMAN_DML_FIND_RECORD_BY_NAME_TASK_H
#define DMAN_DML_FIND_RECORD_BY_NAME_TASK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <DialogueMan/Dml/Task.h>

namespace DMan
{
namespace Dml
{
	class C_FindRecordByNameTask : public C_Task
	{
	public:
		C_FindRecordByNameTask(const wxString& name);
		virtual ~C_FindRecordByNameTask();

	public:
		virtual bool Execute();

	public:
		static const wxString IN_PACK_NAME;
		static const wxString IN_RECORD_NAME;		
		static const wxString OUT_RECORD_PTR;
		static const wxString OUT_RECORD_XTID;
	};

} //namespace Dml
} //namespace DMan

#endif //DMAN_DML_FIND_RECORD_BY_NAME_TASK_H
