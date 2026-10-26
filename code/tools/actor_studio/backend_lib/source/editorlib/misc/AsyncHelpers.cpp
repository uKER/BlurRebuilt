#include "stdafx.h"

#include <EditorLib/CoreConfig.h>
#include <EditorLib/Core.h>
#include <EditorLib/Misc/AsyncHelpers.h>

#include <System/DebugLog.h>
#include <System/Debug/Debug.h>
#include <System/Debug/Log/DebugLog.h>
#include <System/debug/log/Format.h>
#include <System/debug/log/Filter.h>

#include <System/Shell/ShellExec.h>

#include "../../../../../../mercury/library/common/xtwin/include/XtWin/AsyncOpWin.h"

namespace Debug
{
class AsyncOpLogSubscriber : public Debug::I_LogSubscriber
{
public:
	DECLARE_SUBSCRIBER(AsyncOpLogSubscriber);

	AsyncOpLogSubscriber();
	AsyncOpLogSubscriber(AsyncOp* op, bool categoriseOutput);
	~AsyncOpLogSubscriber();

	virtual void Log(Debug::LogEvent *pEvent);
	virtual void SetXMLParameter(const char *pName, const char *pValue);
	virtual bool Start();

private:
	AsyncOp				*m_pAsyncOp;
	Debug::LogFormat	*m_pFormatter;
	Debug::LogFilter	*m_pFilter;
	bool				m_categorise;
};

IMPLEMENT_SUBSCRIBER(AsyncOpLogSubscriber, "asyncoplogwindow");

AsyncOpLogSubscriber::AsyncOpLogSubscriber()
: m_pAsyncOp(0)
, m_categorise(false)
{
	m_pFormatter = new Debug::LogFormat();
	m_pFilter = new Debug::LogFilter();
}

AsyncOpLogSubscriber::AsyncOpLogSubscriber(AsyncOp* op, bool categoriseOutput)
: m_pAsyncOp(op)
, m_categorise(categoriseOutput)
{
	m_pFormatter = new Debug::LogFormat();
	m_pFilter = new Debug::LogFilter();
}

AsyncOpLogSubscriber::~AsyncOpLogSubscriber()
{
	delete m_pFormatter;
	delete m_pFilter;
}

void AsyncOpLogSubscriber::Log(Debug::LogEvent *pEvent)
{
	if (m_pAsyncOp && m_pFilter->Filter(pEvent))
	{
		Btl::String message = m_pFormatter->Format(pEvent);

		AsyncOpIcon icon = kAoiNone;
		if (m_categorise)
		{
			switch(pEvent->level)
			{
			case DebugLevel::Fatal:
			case DebugLevel::Error:
				icon = kAoiError; break;
			case DebugLevel::Warn:
				icon = kAoiWarning; break;
			case DebugLevel::Info:
				icon = kAoiInfo; break;
			case DebugLevel::Debug:
			default:
				icon = kAoiNone; break;
			}
		}
		m_pAsyncOp->NewMessage(icon, message.Chars());
	}	
}

void AsyncOpLogSubscriber::SetXMLParameter(const char *pName, const char *pValue)
{
	if (!_stricmp(pName, "format"))
	{
		Util::C_String value(pValue);
		m_pFormatter->SetFormatString(value);
	}
	else if (!_stricmp(pName, "filter"))
	{
		Util::C_String value(pValue);
		m_pFilter->SetFilterString(value);
	}
}

bool AsyncOpLogSubscriber::Start()
{
	return true;
}

}

namespace EditorLib
{
bool ShellExecuteCheckForCancel(void* pContext)
{
	if (!pContext) return false;
	AsyncOp* op = (AsyncOp*)pContext;
	return op->IsCancelled();
}

bool ShellExecute(AsyncOp* op, C_AsyncShellExecuteParams* pExecuteParams)
{
	bool isInteractive = EditorLib::C_Core::Instance().GetConfig().appType==EditorLib::E_AppType_Interactive;

	Debug::Logger& shellLogger = Debug::GetLogger("tools.editor.shellexecute");
	shellLogger.SetPropagateMessages(false);

	Debug::AsyncOpLogSubscriber* pAsyncSubscriber = 0;
	if (op)
	{
		pAsyncSubscriber = new Debug::AsyncOpLogSubscriber(op, isInteractive && pExecuteParams->categoriseOutput);
		shellLogger.AddSubscriber(pAsyncSubscriber);
	}

	U32 kOneSecond = 1000;
	U32 kOneMinute = kOneSecond*60;
	U32 kOneHour = kOneMinute*60;

	pExecuteParams->returnCode = Sys::ShellExec::ExecuteCommandAndWait(
		pExecuteParams->command.Chars(),
		pExecuteParams->params.Chars(),
		pExecuteParams->workingDir.Chars(),
		Sys::ShellExec::E_LaunchState_Hide,
		op ? &shellLogger : &C_Core::Instance().GetLogger(),
		NULL,
		"",
		kOneHour,
		true,
		true,
		&ShellExecuteCheckForCancel,
		op);

	if (pAsyncSubscriber)
	{
		shellLogger.RemoveSubscriber(pAsyncSubscriber);
		SAFE_DELETE_PTR(pAsyncSubscriber)
	}

	Btl::String msg;
	msg.Printf("'%s' exited with code %d", pExecuteParams->command.Chars(), pExecuteParams->returnCode);

	if (op)
		op->NewMessage(isInteractive && pExecuteParams->returnCode != 0 ? kAoiWarning : kAoiInfo, msg.Chars());
	else if (pExecuteParams->returnCode != 0)
	{
		LOGDEBUG(C_Core::Instance().GetLogger(), msg.Chars());
	}

	return pExecuteParams->returnCode == 0;
}

bool AsyncShellExecute(HWND parent, C_AsyncShellExecuteParams* pExecuteParams)
{
	if (parent)
		return AsyncInvoke(parent, ShellExecute, pExecuteParams);
	else
		return ShellExecute(0, pExecuteParams);
}

}
