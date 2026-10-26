#ifndef EDITOR_LIB_PYTHON_HELPERS_H
#define EDITOR_LIB_PYTHON_HELPERS_H

#pragma once

namespace EditorLib
{
namespace PythonHelpers
{
	S32 ExecFile(const char* fileName);

	void TupleToString(Btl::String& outStr, PyObject* pArgs);
}
}

#endif
