#include "stdafx.h"
#include <Python.h>

#include <EditorLib/Python/GlobalFuncs.h>
#include <EditorLib/Python/PythonHelpers.h>
#include <EditorLib/Command/CommandFactory.h>
#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/GlobalSignals.h>
#include <EditorLib/Core.h>

namespace EditorLib
{
namespace GlobalFuncs
{
	
PyObject* LogCaptureStdout(PyObject* pSelf, PyObject* pArgs)
{
	char* str = 0;
	if (PyArg_ParseTuple(pArgs, "s", &str))
	{
		static Btl::String buffer;
		size_t len = strlen(str);
		buffer = buffer + str;
		if (len > 0 && str[len-1] == '\n')
		{
			LOGINFO(C_Core::Instance().GetLogger(), buffer.Chars());
			buffer.Clear();
		}
	}

	Py_INCREF(Py_None);
	return Py_None;
}

PyObject* LogCaptureStderr(PyObject* pSelf, PyObject* pArgs)
{
	char* str = 0;
	if (PyArg_ParseTuple(pArgs, "s", &str))
	{
		static Btl::String buffer;
		size_t len = strlen(str);
		buffer = buffer + str;
		if (len > 0 && str[len-1] == '\n')
		{
			LOGERROR(C_Core::Instance().GetLogger(), buffer.Chars());
			buffer.Clear();
		}
	}

	Py_INCREF(Py_None);
	return Py_None;
}

static PyMethodDef g_logDefs[] = {
	{"CaptureStdout", LogCaptureStdout, METH_VARARGS, "Logs stdout"},
	{"CaptureStderr", LogCaptureStderr, METH_VARARGS, "Logs stderr"},
	{NULL, NULL, 0, NULL}
};

PyMethodDef* GetLogModuleDefs()
{
	return g_logDefs;
}

static PyObject* ExecuteProxy(PyObject *self, PyObject *pArgs)
{
	S32 cmdArgSize = PyTuple_Size(pArgs);
	if (cmdArgSize != 2)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject* pFuncName = PyTuple_GetItem(pArgs, 0);
	PyObject* pFuncArgs = PyTuple_GetItem(pArgs, 1);

	if (!PyString_Check(pFuncName) || !PyTuple_Check(pFuncArgs))
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
	
	Btl::String cmdName = PyString_AsString(pFuncName);
	cmdName = cmdName.Replace("ascmd_", "");
	
	Btl::String cmdArgString;
	PythonHelpers::TupleToString(cmdArgString, pFuncArgs);
	return Py_BuildValue("i", C_Core::Instance().GetCommandProcessor().Execute(cmdName.Chars(), cmdArgString.Chars()) ? 0 : 1);
}

static PyMethodDef g_asHelperDefs[] = {
	{"Execute", ExecuteProxy, METH_VARARGS, "For internal use only."},
    {NULL, NULL, 0, NULL}
};

PyMethodDef* GetAsHelperModuleDefs()
{
	return g_asHelperDefs;
}

}
}
