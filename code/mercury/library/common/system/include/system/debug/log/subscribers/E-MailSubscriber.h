#ifndef __SYSTEM_EMAIL_SUBSCRIBER_H__
#define __SYSTEM_EMAIL_SUBSCRIBER_H__

#ifdef _MSC_VER
#pragma once
#endif

namespace Sys
{
	class I_File;
}
namespace MT
{
	class Mutex;
}

namespace Debug
{

class LogFormat;

/// implementation of a subscriber that save the log to a file
class EMailSubscriber : public I_LogSubscriber
{
public:
						DECLARE_SUBSCRIBER(EMailSubscriber);

						/// constructor
						EMailSubscriber();
						/// destructor
	virtual				~EMailSubscriber();

						/// receive a log event
	virtual void		Log(LogEvent *pEvent);

						/// parse a configuration parameter for this subscriber
	virtual void		SetXMLParameter(const char *pName, const char *pValue);

						/// startup
	virtual bool		Start();

						/// send an e-mail containing the last 'n' events received
	virtual void		SendEMail();

private:
	// parameters
	Util::C_String		m_sSMTPHost;
	Util::C_String		m_sUser;
	Util::C_String		m_sPassword;

	Util::C_String		m_sSender;
	Util::C_String		m_sRecipient;
	Util::C_String		m_sSubject;

	DebugLevel::Level	m_triggerLevel;
	int					m_iBufferSize;
	int					m_iMaxEMails;
	int					m_iMaxEMailsTimeout;
	int					m_iLastEmailSentTime;

	// circular event buffer
	Ut::Vector<LogEvent>	m_EventBuffer;
	int						m_iBufferPos;
	int						m_iBufferUsed;
	int						m_iNumEMailsSent;

	LogFormat			*m_pFormatter;
	MT::Mutex			*m_pMutex;
};

};


#endif
