#include "stdafx.h"

#include <System/DebugLog.h>
#include <System/debug/log/Format.h>
#include <System/debug/log/Filter.h>

#include <EditorApp/Components/Log.h>

namespace Debug
{
	IMPLEMENT_SUBSCRIBER(TextSubscriber, "logwindow");
}


using namespace Debug;

TextSubscriber::TextSubscriber()
: m_pTextCtrl(0)
{
	delete m_pMutex;
	delete m_pFormatter;
	delete m_pFilter;
}


TextSubscriber::TextSubscriber(wxTextCtrl *pTextCtrl)
: wxLog()
, Debug::I_LogSubscriber()
, m_pTextCtrl(pTextCtrl)
{
	m_pFormatter = new Debug::LogFormat();
	m_pFilter = new Debug::LogFilter();
	m_pMutex = new MT::Mutex;

	m_pTextCtrl->SetEditable(false);
}


TextSubscriber::~TextSubscriber()
{
	delete m_pMutex;
	delete m_pFormatter;
	delete m_pFilter;
}


void TextSubscriber::Log(Debug::LogEvent *pEvent)
{
	if (m_pFilter->Filter(pEvent))
	{
		switch(pEvent->level)
		{
		case DebugLevel::Fatal:
		case DebugLevel::Error:
		case DebugLevel::Warn:
			Log(*wxRED, m_pFormatter->Format(pEvent));
			break;
		case DebugLevel::Debug:
			Log(*wxGREEN, m_pFormatter->Format(pEvent));
			break;
		case DebugLevel::Info:
		default:
			Log(*wxBLACK, m_pFormatter->Format(pEvent));
			break;
		}
	}
}

void TextSubscriber::SetXMLParameter(const char *pName, const char *pValue)
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

bool TextSubscriber::Start()
{
	return true;
}


void TextSubscriber::Log(const wxColor& c, const wxChar* szString)
{
	if (m_pTextCtrl)
	{
		m_pMutex->Wait();
		// Count number of newlines
		int lines = 1;
		const wxChar* cstr = szString;
		for( ; *cstr ; ++cstr )
			if( *cstr == '\n' )
				++lines;

		m_pTextCtrl->Freeze();
		m_pTextCtrl->ShowPosition(m_pTextCtrl->GetLastPosition());
		m_pTextCtrl->SetDefaultStyle(wxTextAttr(c));

		wxString message;
		TimeStamp(&message);
		message += szString;
		m_pTextCtrl->AppendText(message);
		m_pTextCtrl->ScrollLines(lines);
		m_pTextCtrl->Thaw();

		m_pMutex->Release();
	}
}

void TextSubscriber::DoLog(wxLogLevel level, const wxChar *szString, time_t t)
{
	wxString msg;
	msg << szString << '\n';
	switch(level)
	{
	case wxLOG_FatalError:
	case wxLOG_Error:
	case wxLOG_Warning:
		Log(*wxRED, msg);
		break;
	case wxLOG_Debug:
	case wxLOG_Trace:
		Log(*wxBLUE, msg);
		break;
	case wxLOG_Status:
		Log(*wxGREEN, msg);
		break;
	case wxLOG_Message:
	case wxLOG_Info:
	default:
		Log(*wxBLACK, msg);
		break;
	}
}

void TextSubscriber::DoLogString(const wxChar *szString, time_t t)
{
	Log(*wxBLACK, szString);
}

