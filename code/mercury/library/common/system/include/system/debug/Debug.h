#ifndef SYSTEM_DEBUG_DEBUG_H
#define SYSTEM_DEBUG_DEBUG_H

#ifdef _MSC_VER
#pragma once
#endif

//
// DebugLn
//
// Display a line of debug in the output window in the IDE.  Please remove all occurances of this function in your code
// before you commit, if you need a more permanent debug display use the logging functions instead.
//

void DebugLn(const char* text, ...);

//
// Includes
//

#include <System/Debug/Variables.h>
#include "TestDebugVariables.h"

#endif // SYSTEM_DEBUG_DEBUG_H
