#ifndef DMAN_DML_SET_COMPONENT_TYPE_H
#define DMAN_DML_SET_COMPONENT_TYPE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <DialogueMan/Dml/Task.h>

namespace DMan
{
namespace Dml
{
	class C_SetComponentTypeTask : public C_Task
	{
	public:
		C_SetComponentTypeTask(const wxString& name);
		virtual ~C_SetComponentTypeTask();
	
	public:
		virtual bool Execute();

	public:
		static const wxString IN_COMPONENT_PTR;
		static const wxString IN_COMPONENT_TYPE;
	};

} //namespace Dml
} //namespace DMan

#endif //DMAN_DML_SET_COMPONENT_TYPE_H
