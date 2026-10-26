#ifndef DMAN_DML_NEW_RECORD_TASK_H
#define DMAN_DML_NEW_RECORD_TASK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <DialogueMan/Dml/Task.h>

namespace DMan
{
namespace Dml
{
	class C_NewRecordTask : public C_Task
	{
	public:
		C_NewRecordTask(const wxString& name);
		virtual ~C_NewRecordTask();

	public:
		virtual bool Execute();

	public:
		static const wxString IN_PACK_NAME;
		static const wxString IN_RECORD_NAME;
		static const wxString IN_RECORD_TYPE;
		static const wxString OUT_RECORD_PTR;
		static const wxString OUT_RECORD_XTID;
	};

} //namespace Dml
} //namespace DMan

#endif //DMAN_DML_NEW_RECORD_TASK_H
