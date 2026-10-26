#include "stdafx.h"
#include <Python.h>

#include <EditorLib/Misc/Verification.h>
#include <EditorLib/Python/GlobalFuncs.h>
#include <EditorLib/Python/PythonHelpers.h>
#include <EditorLib/GlobalSignals.h>
#include <EditorLib/Core.h>
#include <EditorLib/CoreConfig.h>

#include <EditorApp/Frame/Frame.h>
#include <EditorApp/Frame/Menus/FrameMenuBar.h>
#include <EditorApp/Misc/MenuBuilder.h>
#include <EditorApp/Tasks/LevelTasks.h>
#include <EditorApp/App.h>

#include <EditorApp/Editor/Manipulators/SelectManipulator.h>

namespace Editor
{
namespace EditorFuncs
{

static Btl::Map<S32, Btl::Pair<Btl::String, Btl::String> >* g_commandMenuEntries = new Btl::Map<S32, Btl::Pair<Btl::String, Btl::String> >();
static Btl::Map<S32, PyObject*>* g_pythonMenuEntries = new Btl::Map<S32, PyObject*>();


static void MenuUpdate(wxUpdateUIEvent& e)
{
	e.Enable(Tasks::IsLevelLoaded());
}

static void MenuCommand(wxCommandEvent& e)
{
	Btl::Map<S32, Btl::Pair<Btl::String, Btl::String> >::Iterator it = g_commandMenuEntries->Find(e.GetId());

	if (it == g_commandMenuEntries->End())
		return;

	Btl::String cmd = it->GetValue().first;
	Btl::String cmdArgs = it->GetValue().second;
	EditorLib::C_Core::Instance().GetCommandProcessor().Execute(cmd, cmdArgs);
}

static void MenuPython(wxCommandEvent& e)
{
	Btl::Map<S32, PyObject*>::Iterator it = g_pythonMenuEntries->Find(e.GetId());

	if (it == g_pythonMenuEntries->End())
		return;

	PyObject *pyResult = PyObject_CallObject(it->GetValue(), NULL);

	if (pyResult)
		Py_DECREF(pyResult);

	if (PyErr_Occurred())
	{
		PyErr_Print();
		PyErr_Clear();
	}

	return;
}

static PyObject* IsSelectModeObject(PyObject* self, PyObject* pArgs)
{
	return Py_BuildValue("b", C_SelectManipulator::GetSelectionMode() == E_SelectionMode_Object);
}

static PyObject* RegisterMenu(PyObject* self, PyObject* pArgs)
{
	if (EditorLib::C_Core::Instance().GetConfig().appType != EditorLib::E_AppType_Interactive) 
	{
		Py_INCREF(Py_None);
		return Py_None;
	}

	S32 cmdArgSize = PyTuple_Size(pArgs);
	if (cmdArgSize == 3)
	{
		PyObject* pMenuPath = PyTuple_GetItem(pArgs, 0);
		PyObject* pMenuCmd = PyTuple_GetItem(pArgs, 1);
		PyObject* pMenuCmdArgs = PyTuple_GetItem(pArgs, 2);

		if (!PyString_Check(pMenuPath) || !PyString_Check(pMenuCmd) || !PyString_Check(pMenuCmdArgs))
		{
			Py_INCREF(Py_None);
			return Py_None;
		}

		Btl::String menuPath = PyString_AsString(pMenuPath);
		Btl::String menuCmd = PyString_AsString(pMenuCmd);
		Btl::String menuCmdArgs = PyString_AsString(pMenuCmdArgs);

		Editor::C_Frame *pFrame = wxGetApp().GetFrame();
		wxMenuBar* pWxMenuBar = pFrame->GetMenuBar();

		S32 menuId = pFrame->GetFrameMenuBar()->GetMenuBuilder()->AddMenuEntry(menuPath.Chars(), pFrame, &MenuCommand, &MenuUpdate);

		g_commandMenuEntries->Insert(menuId, Btl::Pair<Btl::String, Btl::String>(menuCmd, menuCmdArgs));
	}

	if (cmdArgSize == 2)
	{
		PyObject* pMenuPath = PyTuple_GetItem(pArgs, 0);
		PyObject* pMenuFunction = PyTuple_GetItem(pArgs, 1);

		if (!PyString_Check(pMenuPath) || !PyCallable_Check(pMenuFunction))
		{
			Py_INCREF(Py_None);
			return Py_None;
		}

		Py_INCREF(pMenuFunction);

		Btl::String menuPath = PyString_AsString(pMenuPath);

		Editor::C_Frame *pFrame = wxGetApp().GetFrame();
		wxMenuBar* pWxMenuBar = pFrame->GetMenuBar();

		S32 menuId = pFrame->GetFrameMenuBar()->GetMenuBuilder()->AddMenuEntry(menuPath.Chars(), pFrame, &MenuPython, &MenuUpdate);

		g_pythonMenuEntries->Insert(menuId, pMenuFunction);
	}

	Py_INCREF(Py_None);
	return Py_None;
}

bool RunVerifyExport(void* function, XtId id)
{
	PyObject* pVerifyFunction = (PyObject*)function;

	// Add args

	PyObject* args = PyTuple_New(1);
	PyTuple_SetItem(args, 0, PyLong_FromUnsignedLong(id));
	PyObject* pyResult = PyObject_CallObject(pVerifyFunction, args);

	if (pyResult)
	{
		Py_DECREF(pyResult);

		if (PyBool_Check(pyResult))
			return pyResult == Py_True;
	}

	if (PyErr_Occurred())
	{
		PyErr_Print();
		PyErr_Clear();

		return false;
	}

	return true;
}

static PyObject* RegisterVerifyExport(PyObject* self, PyObject* pArgs)
{
	S32 cmdArgSize = PyTuple_Size(pArgs);

	if (cmdArgSize == 1)
	{
		PyObject* pVerifyFunction = PyTuple_GetItem(pArgs, 0);

		if (PyCallable_Check(pVerifyFunction))
		{
			Py_INCREF(pVerifyFunction);

			EditorLib::C_Core::Instance().GetVerificationSystem().RegisterCallback(Ut::Delegate<bool(void*, XtId)>(&RunVerifyExport), pVerifyFunction);
		}
	}

	Py_INCREF(Py_None);
	return Py_None;
}

static PyMethodDef g_edHelperDefs[] = {
	{"RegisterMenu", RegisterMenu, METH_VARARGS, "Register a new menu. Args (menu, command, arguments). Example (\"File|Save\", \"level\", \"-s\"."},
	{"RegisterVerifyExport", RegisterVerifyExport, METH_VARARGS, "Register a function to be called before an export occurs. Args (menu, command, arguments). Example (\"File|Save\", \"level\", \"-s\"."},
	{"IsSelectModeObject", IsSelectModeObject, METH_NOARGS, "Returns true if the select manipulator is currently in object mode"},
    {NULL, NULL, 0, NULL}
};

PyMethodDef* GetEdHelperModuleDefs()
{
	return g_edHelperDefs;
}

}
}