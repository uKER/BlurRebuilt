#ifndef EDITOR_ASYNC_HELPERS_H
#define EDITOR_ASYNC_HELPERS_H

#ifdef _MSC_VER
#pragma once
#endif

#define ASYNC_SET_TITLE(op, text) op ? op->SetTitle(text) : LOGINFO(EditorLib::C_Core::Instance().GetLogger(), text)
#define ASYNC_NEW_MESSAGE(op, type, text) op ? op->NewMessage(type, text) : LOGINFO(EditorLib::C_Core::Instance().GetLogger(), text)
#define ASYNC_APPEND_MESSAGE(op, text) op ? op->AppendMessage(text) : LOGINFO(EditorLib::C_Core::Instance().GetLogger(), text)

#include "../../../../../../mercury/library/common/xtwin/include/XtWin/AsyncOpWin.h"

namespace EditorLib
{
	struct C_AsyncShellExecuteParams
	{
		Btl::String command;
		Btl::String params;
		Btl::String workingDir;
		//asyncOp status messages - remains open on warnings/errors
		bool categoriseOutput;
		S32 returnCode;
	};

	bool AsyncShellExecute(HWND parent, C_AsyncShellExecuteParams* pExecuteParams);

	template<typename PARAM_TYPE>
	bool AsyncTask(HWND parent, bool (*callback)(class AsyncOp*, PARAM_TYPE*), PARAM_TYPE* pParams);

	#include "AsyncHelpers.inl"
}


#endif
