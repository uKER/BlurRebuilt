#include "stdafx.h"
#include <Python.h>

#include <EditorLib/Python/PythonHelpers.h>
#include <EditorLib/GlobalSignals.h>
#include <EditorLib/Core.h>

namespace EditorLib
{
namespace PythonHelpers
{

S32 ExecFile(const char* fileName)
{
	Btl::String tmpPath= Misc::FormatFileString(fileName);
	Btl::String tmpModule = Misc::RemoveFileExtension(Misc::ExtractFilename(tmpPath));

	if (tmpPath.Length() == 0)
		return 1;

	PyObject* pyFileObject = PyFile_FromString(&(tmpPath[0]), "r");
	if (pyFileObject == NULL) 
		return 1;

	bool exec = true;
	S32 result = 0;

	//add the script location to the python module search path
	Btl::String oldPath = Py_GetPath();
	Btl::String newPath = Misc::ExtractDirectory(tmpPath) + ";" + oldPath;

	PySys_SetPath(const_cast<char*>(newPath.Chars()));

	//Look for an "ActorStudioMain" function in the script
	PyObject *pyModule = PyImport_ImportModule(const_cast<char*>(tmpModule.Chars()));
	if (pyModule)
	{	
		PyObject *pyDict = PyModule_GetDict(pyModule);
		PyObject *pyFunc = PyDict_GetItemString(pyDict, "ActorStudioMain");
		if (pyFunc && PyCallable_Check(pyFunc))
		{
			exec = false;
			PyObject *pyResult = PyObject_CallObject(pyFunc, NULL);
			if (pyResult && PyInt_Check(pyResult))
				result = (S32)PyInt_AsLong(pyResult);
			else
				//Py_None or non-integer return value, assume everything went ok
				result = 0;

			if (pyResult)
				Py_DECREF(pyResult);
		}

		Py_DECREF(pyModule);
	}

	//clear any Python exceptions so we can carry on
	if (PyErr_Occurred())
	{
		PyErr_Print();
		PyErr_Clear();
	}

	if (exec)
	{
		//run the script - no exit codes from the script are possible
		result = PyRun_SimpleFile(PyFile_AsFile(pyFileObject), fileName);
	}

	//clear any further Python exceptions to prevent crashing later in Py_Finalize
	if (PyErr_Occurred())
	{
		PyErr_Print();
		PyErr_Clear();
	}

	//reset python module search path
	PySys_SetPath(const_cast<char*>(oldPath.Chars()));

 	Py_DECREF(pyFileObject);
	
	return result;
}

void TupleToString(Btl::String& outStr, PyObject* pArgs)
{
	if (PyTuple_Check(pArgs))
	{
		S32 cmdArgSize = PyTuple_Size(pArgs);
		for (S32 j = 0; j < cmdArgSize; ++j)
		{
			PyObject* pCmdArg = PyTuple_GetItem(pArgs, j);
			TupleToString(outStr, pCmdArg);
			if (j < cmdArgSize-1)
				outStr = outStr + " ";
		}
	}
	else if (PyString_Check(pArgs))
	{
		outStr += PyString_AsString(pArgs);
	}
	else if (PyInt_Check(pArgs))
	{
		S32 val = PyInt_AsLong(pArgs);

		Btl::String tmp;
		tmp.Printf("%i", val);
		outStr = outStr + tmp;
	}
	else if (PyFloat_Check(pArgs))
	{
		F32 val = (F32)PyFloat_AsDouble(pArgs);

		Btl::String tmp;
		tmp.Printf("%f", val);
		outStr = outStr + tmp;
	}
	else if (PyList_Check(pArgs))
	{
		S32 size = PyList_Size(pArgs);
		for (S32 j = 0; j < size; ++j)
		{
			PyObject* pItem = PyList_GetItem(pArgs, j);
			TupleToString(outStr, pItem);
			outStr = outStr + ",";
		}
	}
}

}
}
