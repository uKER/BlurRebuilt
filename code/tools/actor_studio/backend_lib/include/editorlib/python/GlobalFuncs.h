#ifndef EDITOR_LIB_PYTHON_GLOBAL_FUNCS_H
#define EDITOR_LIB_PYTHON_GLOBAL_FUNCS_H

#pragma once

namespace EditorLib
{
namespace GlobalFuncs
{
	PyMethodDef* GetLogModuleDefs();

	PyMethodDef* GetAsHelperModuleDefs();
}
}

#endif
