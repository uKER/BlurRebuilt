#ifndef EDITOR_LIB_PYTHON_MANAGER_H
#define EDITOR_LIB_PYTHON_MANAGER_H

#pragma once

struct PyMethodDef;

namespace EditorLib
{
	class C_PythonManager
	{
	public:
		C_PythonManager();
		~C_PythonManager();

		C_Status Initialise();

		C_Status RegisterMethods(Btl::String moduleName, PyMethodDef* pyMethodDef);

		C_Status RegisterCommand(const char* pName);
	};
}

#endif
