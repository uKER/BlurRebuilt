#ifndef STDAFX_H
#define STDAFX_H

#ifdef _MSC_VER
#pragma once
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <xtl.h>
#include <xgraphics.h>

#include <Core/Core.h>
#include <Core/Maths.h>
#include <Core/Utility/Utility.h>
#include <System/DebugLog.h>
#include <System/Debug/Debug.h>
#include <Display.h>

#include "Samples/System/Debug.h"

#ifdef  __cplusplus
extern "C" {
#endif

_CRTIMP void __cdecl _wassert(__in_z const wchar_t * _Message, __in_z const wchar_t *_File, __in unsigned _Line);

#ifdef  __cplusplus
}
#endif

#define assert(_Expression) (void)( (!!(_Expression)) || (_wassert(_CRT_WIDE(#_Expression), _CRT_WIDE(__FILE__), __LINE__), 0) )



#endif // STDAFX_H
