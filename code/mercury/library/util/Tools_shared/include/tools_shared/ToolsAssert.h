#ifndef MERCURY_TOOLS_ASSERT_H
#define MERCURY_TOOLS_ASSERT_H

#ifdef _MSCVER
#pragma once
#endif

#include "core/BizAssert.h"

namespace Tools
{

#define TOOLS_ASSERT_EXCEPTION_CODE 0xc0dedead

/// override for core assert
bool ToolsAssert(const char *pszFunction, const char *pszExpression, const char *pszFilename, int iLineNumber);
bool BizToolsAssert(BizAssertKind kind, const char* expr, const char* msg, const char* file, int line);

};

#endif
