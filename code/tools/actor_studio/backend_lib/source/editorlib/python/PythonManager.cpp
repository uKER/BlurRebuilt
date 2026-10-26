#include "stdafx.h"
#include <Python.h>

#include <EditorLib/Command/CommandFactory.h>
#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Command/Argument/ArgumentDatabase.h>
#include <EditorLib/Command/Argument/Argument.h>
#include <EditorLib/Command/Argument/Syntax.h>
#include <EditorLib/Command/Commands.h>

#include <EditorLib/Python/PythonHelpers.h>
#include <EditorLib/Python/GlobalFuncs.h>
#include <EditorLib/Python/PythonManager.h>
#include <EditorLib/Python/XtFuncs.h>

#include <EditorLib/GlobalSignals.h>
#include <EditorLib/Core.h>

extern "C" {
	void init_collision();
	void init_helpers();
}

namespace EditorLib
{

C_PythonManager::C_PythonManager()
{
	//do not initialise the 'site' module
	//this is not embedded in the python binary, & we've no need to distribute these additional python modules
	Py_NoSiteFlag = 1;

	Py_Initialize();

	// TODO: Fix SWIG bindings
	/*init_helpers();
	init_collision();
	PyRun_SimpleString("import helpers \n");
	PyRun_SimpleString("import collision \n");*/
}

C_PythonManager::~C_PythonManager()
{
	// memory leakage ...
	Py_Finalize();
}

C_Status C_PythonManager::Initialise()
{
	if (!Py_InitModule("log", GlobalFuncs::GetLogModuleDefs()))
		return E_Status_Failure;

	int result = PythonHelpers::ExecFile("./scripts/editorlib/startup/boot.py");
	if (result != 0) return E_Status_Failure;

	if (!Py_InitModule("asHelpers", GlobalFuncs::GetAsHelperModuleDefs()))
		return E_Status_Failure;

	if (!Py_InitModule("xtHelpers", XtFuncs::GetXtHelpersDef()))
		return E_Status_Failure;

	result = PyRun_SimpleString("import asHelpers\n");
	if (result != 0) return E_Status_Failure;

	result = PyRun_SimpleString("import xtHelpers\n");
	if (result != 0) return E_Status_Failure;

	return E_Status_Success;
}

C_Status C_PythonManager::RegisterMethods(Btl::String string, PyMethodDef* pyMethodDef)
{
	if (!Py_InitModule(string.Chars(), pyMethodDef))
		return E_Status_Failure;

	Btl::String importCommand = Btl::String("import ") + string + "\n";
	if (PyRun_SimpleString(importCommand.Chars()) != 0)
		return E_Status_Failure;

	return E_Status_Success;
}

C_Status C_PythonManager::RegisterCommand(const char* pName)
{
	//C_Syntax* pSyntax = it->GetValue().createSyntax();
	//TODO: Add command level help into syntax object and acquire it here. Also have command processor
	//automagically handle -h and --help requests

	char buf[2048];
	sprintf(
		buf, 
		"@FuncTrace\n" \
		"def ascmd_%s(funcName, *__args):\n" \
		"	return asHelpers.Execute(funcName, __args);\n" \
		"AStudioCore.Commands.%s = ascmd_%s;\n" \
		"del ascmd_%s\n",
		pName, pName, pName, pName);

	int result = PyRun_SimpleString(buf);
	if (result != 0) return E_Status_Failure;

	//delete pSyntax;
	return E_Status_Success;
}

}
