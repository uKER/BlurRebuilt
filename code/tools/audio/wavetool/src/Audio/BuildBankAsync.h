#pragma once

#include "core/containers\string.h"

extern AsyncOp* gpAsyncOp;

void AsyncOpNewMessageFn(int msgType, Util::C_String& str, void* pContext);
void AsyncOpSetProgressFn(F32 fProgress, void* pContext);

