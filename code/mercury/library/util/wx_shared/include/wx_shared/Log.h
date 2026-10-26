#ifndef EDITOR_LOG_H
#define EDITOR_LOG_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Debug
{
	class TextSubscriber : public Debug::I_LogSubscriber, public wxLog
	{
	public:

		DECLARE_SUBSCRIBER(TextSubscriber);

		TextSubscriber();
		TextSubscriber(wxTextCtrl *pTextCtrl);

		~TextSubscriber();

		//I_LogSubscriber
		virtual void Log(Debug::LogEvent *pEvent);
		virtual void SetXMLParameter(const char *pName, const char *pValue);
		virtual bool Start();


		void Log(const wxColor& c, const wxChar* string);

	protected:

		//wxLog
		virtual void DoLog(wxLogLevel level, const wxChar *szString, time_t t);
		virtual void DoLogString(const wxChar *szString, time_t t);

	private:
		Debug::LogFormat	*m_pFormatter;
		Debug::LogFilter	*m_pFilter;
		MT::Mutex			*m_pMutex;

		wxTextCtrl			*m_pTextCtrl;

		DECLARE_NO_COPY_CLASS(TextSubscriber)
	};
}

#endif