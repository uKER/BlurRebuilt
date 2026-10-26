#ifndef DEBUG_H
#define DEBUG_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Audio
{

typedef void (*FuncPrintString)(U32 uAttribs, const char* pchText, void* pContext);

int Print(U32 uAttribs, const char *format, ...);
void SetPrintStringCallback(FuncPrintString funcPtr, void* pContext);

}
#endif
