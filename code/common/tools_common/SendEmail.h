#pragma once

#include <windows.h>
#include <mapi.h>
#include <cassert>
#include <string>
#include <sstream>
#include <Tools_Common/status.h>
#include <core/containers/vector.h>

#define MAPIInit()		m_hMAPI = ::LoadLibrary("MAPI32.DLL")
#define MAPIDeinit()	::FreeLibrary(m_hMAPI)

#define MAPILogon \
	(!m_hMAPI) ? MAPI_E_FAILURE : ((LPMAPILOGON) GetProcAddress(m_hMAPI, "MAPILogon"))
#define MAPILogoff \
	(!m_hMAPI) ? MAPI_E_FAILURE : ((LPMAPILOGOFF) GetProcAddress(m_hMAPI, "MAPILogoff"))
#define MAPISendMail \
	(!m_hMAPI) ? MAPI_E_FAILURE : ((LPMAPISENDMAIL) GetProcAddress(m_hMAPI, "MAPISendMail"))
#define MAPISendDocuments \
	(!m_hMAPI) ? MAPI_E_FAILURE : ((LPMAPISENDDOCUMENTS) GetProcAddress(m_hMAPI, "MAPISendDocuments"))
#define MAPIFindNext \
	(!m_hMAPI) ? MAPI_E_FAILURE : ((LPMAPIFINDNEXT) GetProcAddress(m_hMAPI, "MAPIFindNext"))
#define MAPIReadMail \
	(!m_hMAPI) ? MAPI_E_FAILURE : ((LPMAPIREADMAIL) GetProcAddress(m_hMAPI, "MAPIReadMail"))
#define MAPISaveMail \
	(!m_hMAPI) ? MAPI_E_FAILURE : ((LPMAPISAVEMAIL) GetProcAddress(m_hMAPI, "MAPISaveMail"))
#define MAPIDeleteMail \
	(!m_hMAPI) ? MAPI_E_FAILURE : ((LPMAPIDELETEMAIL) GetProcAddress(m_hMAPI, "MAPIDeleteMail"))
#define MAPIFreeBuffer \
	(!m_hMAPI) ? MAPI_E_FAILURE : ((LPMAPIFREEBUFFER) GetProcAddress(m_hMAPI, "MAPIFreeBuffer"))
#define MAPIAddress \
	(!m_hMAPI) ? MAPI_E_FAILURE : ((LPMAPIADDRESS) GetProcAddress(m_hMAPI, "MAPIAddress"))
#define MAPIDetails \
	(!m_hMAPI) ? MAPI_E_FAILURE : ((LPMAPIDETAILS) GetProcAddress(m_hMAPI, "MAPIDetails"))
#define MAPIResolveName \
	(!m_hMAPI) ? MAPI_E_FAILURE : ((LPMAPIRESOLVENAME) GetProcAddress(m_hMAPI, "MAPIResolveName"))

namespace Tu
{
	class SendEmail
	{
	public:
		SendEmail(Status& status);

		~SendEmail();

		void AddRecipient(const std::string& recipient);

		void SetSubject(const std::string& subject);

		void SetBody(const std::string& body);

		void AttachFile(const std::string& file);

		Status Send(bool bDialog = false);
	
	private:
		SendEmail();

		HINSTANCE		m_hMAPI;
		
		std::string		m_subject;
		std::string		m_body;

		Ut::UtVector< Ut::DynamicVector< std::string> > m_recipients;
		Ut::UtVector< Ut::DynamicVector< std::string> > m_files;
	};

	inline SendEmail::SendEmail(Status& status)
	{
		MAPIInit();

		if (!m_hMAPI)
		{
			status = Status(Status::FAILURE, "SendEmail: Could not initialise MAPI.");
		}
	}

	inline SendEmail::~SendEmail()
	{
		MAPIDeinit();
	}

	inline void SendEmail::AddRecipient(const std::string& recipient)
	{
		m_recipients.PushBack(recipient);
	}

	inline void SendEmail::SetSubject(const std::string& subject)
	{
		m_subject = subject;
	}

	inline void SendEmail::SetBody(const std::string& body)
	{
		m_body = body;
	}

	inline void SendEmail::AttachFile(const std::string& file)
	{
		m_files.PushBack(file);
	}

	inline Status SendEmail::Send(bool bDialog)
	{
		if (m_recipients.Size() == 0)
		{
			return Status(Status::FAILURE, "SendEmail: No recipients.");
		}

		MapiRecipDesc*	pRecipientDescriptionList;
		MapiFileDesc*	pFileDescriptionList;
		pRecipientDescriptionList = new MapiRecipDesc[m_recipients.Size()];
		if (m_files.Size() > 0)
		{
			pFileDescriptionList = new MapiFileDesc[m_files.Size()];
		}
		else
		{
			pFileDescriptionList = 0;
		}

		MapiRecipDesc sender;
		memset(&sender, 0, sizeof(sender));

		//Fill sender struct
		sender.ulReserved = 0;
		sender.ulRecipClass = MAPI_ORIG;
		sender.lpszName = 0;
		sender.lpszAddress = "SMTP:Sender";
		sender.lpEntryID = 0;
		sender.ulEIDSize = 0;

		//Initialise recipients
		for (U32 iRecipient = 0; iRecipient < (U32)m_recipients.Size(); iRecipient++)
		{
			pRecipientDescriptionList[iRecipient].lpEntryID = 0;
			pRecipientDescriptionList[iRecipient].ulRecipClass = MAPI_TO;
			pRecipientDescriptionList[iRecipient].lpszAddress = 0;
			pRecipientDescriptionList[iRecipient].lpszName = (LPTSTR)malloc(m_recipients[iRecipient].length()+1);
			pRecipientDescriptionList[iRecipient].ulEIDSize = 0;
			pRecipientDescriptionList[iRecipient].ulReserved = 0;

			strcpy(pRecipientDescriptionList[iRecipient].lpszName, m_recipients[iRecipient].c_str());
		}

		//Initialise files

		for (U32 iFile = 0; iFile < (U32)m_files.Size(); iFile++)
		{
			pFileDescriptionList[iFile].flFlags = 0;
			pFileDescriptionList[iFile].lpFileType = 0;
			pFileDescriptionList[iFile].lpszFileName = 0;
			pFileDescriptionList[iFile].lpszPathName = (LPTSTR)malloc(m_files[iFile].length()+1);
			pFileDescriptionList[iFile].nPosition = 0xFFFFFFFF;
			pFileDescriptionList[iFile].ulReserved = 0;

			strcpy(pFileDescriptionList[iFile].lpszPathName, m_files[iFile].c_str());
		}

		MapiMessage msg;
		memset(&msg, 0, sizeof(msg));

		msg.flFlags = 0;
		msg.lpFiles = pFileDescriptionList;
		msg.lpOriginator = 0;
		msg.lpRecips = pRecipientDescriptionList;
		msg.lpszConversationID = 0;
		msg.lpszDateReceived = 0;
		msg.lpszMessageType = 0;

		char* pBodyText = new char[m_body.length()];
		strcpy(pBodyText, m_body.c_str());
		msg.lpszNoteText = pBodyText;

		char* pSubjectText = new char[m_subject.length()];
		strcpy(pSubjectText, m_subject.c_str());
		msg.lpszSubject = pSubjectText;
		msg.nFileCount = m_files.Size();
		msg.nRecipCount = m_recipients.Size();
		msg.ulReserved = 0;

		Status returnStatus = Status::SUCCESS;
		int iFlags = MAPI_LOGON_UI;
		if (bDialog) iFlags |= MAPI_DIALOG;
		ULONG lRet = MAPISendMail(0, 0, &msg, iFlags, 0);
		if (lRet != 0)
		{
			switch(lRet)
			{
			case MAPI_E_USER_ABORT:
				returnStatus = Status(Status::FAILURE, "SendEmail: MAPI: Sending aborted by user.");
				break;
			case MAPI_E_FAILURE:
				returnStatus = Status(Status::FAILURE, "SendEmail: MAPI: Failure.");
				break;
			case MAPI_E_LOGON_FAILURE:
				returnStatus = Status(Status::FAILURE, "SendEmail: MAPI: Logon failure.");
				break;
			case MAPI_E_DISK_FULL:
				returnStatus = Status(Status::FAILURE, "SendEmail: MAPI: Disk full.");
				break;
			case MAPI_E_INSUFFICIENT_MEMORY:
				returnStatus = Status(Status::FAILURE, "SendEmail: MAPI: Insufficient memory.");
				break;
			case MAPI_E_ACCESS_DENIED:
				returnStatus = Status(Status::FAILURE, "SendEmail: MAPI: Access denied.");
				break;
			case MAPI_E_TOO_MANY_SESSIONS:
				returnStatus = Status(Status::FAILURE, "SendEmail: MAPI: Too many sessions.");
				break;
			case MAPI_E_TOO_MANY_FILES:
				returnStatus = Status(Status::FAILURE, "SendEmail: MAPI: Too many files.");
				break;
			case MAPI_E_TOO_MANY_RECIPIENTS:
				returnStatus = Status(Status::FAILURE, "SendEmail: MAPI: Too many recipients.");
				break;
			case MAPI_E_ATTACHMENT_NOT_FOUND:
				returnStatus = Status(Status::FAILURE, "SendEmail: MAPI: Attachment not found.");
				break;
			case MAPI_E_ATTACHMENT_OPEN_FAILURE:
				returnStatus = Status(Status::FAILURE, "SendEmail: MAPI: Attachment open failure.");
				break;
			case MAPI_E_ATTACHMENT_WRITE_FAILURE:
				returnStatus = Status(Status::FAILURE, "SendEmail: MAPI: Attachment write failure.");
				break;
			case MAPI_E_UNKNOWN_RECIPIENT:
				returnStatus = Status(Status::FAILURE, "SendEmail: MAPI: Unknown recipient.");
				break;
			case MAPI_E_BAD_RECIPTYPE:
				returnStatus = Status(Status::FAILURE, "SendEmail: MAPI: Bad recipient type.");
				break;
			case MAPI_E_NO_MESSAGES:
				returnStatus = Status(Status::FAILURE, "SendEmail: MAPI: No messages.");
				break;
			case MAPI_E_INVALID_MESSAGE:
				returnStatus = Status(Status::FAILURE, "SendEmail: MAPI: Invalid message.");
				break;
			case MAPI_E_TEXT_TOO_LARGE:
				returnStatus = Status(Status::FAILURE, "SendEmail: MAPI: Text too large.");
				break;
			case MAPI_E_INVALID_SESSION:
				returnStatus = Status(Status::FAILURE, "SendEmail: MAPI: Invalid session.");
				break;
			case MAPI_E_TYPE_NOT_SUPPORTED:
				returnStatus = Status(Status::FAILURE, "SendEmail: MAPI: Type not supported.");
				break;
			case MAPI_E_AMBIGUOUS_RECIPIENT:
				returnStatus = Status(Status::FAILURE, "SendEmail: MAPI: Ambiguous recipient.");
				break;
			case MAPI_E_MESSAGE_IN_USE:
				returnStatus = Status(Status::FAILURE, "SendEmail: MAPI: Message in use.");
				break;
			case MAPI_E_NETWORK_FAILURE:
				returnStatus = Status(Status::FAILURE, "SendEmail: MAPI: Network failure.");
				break;
			case MAPI_E_INVALID_EDITFIELDS:
				returnStatus = Status(Status::FAILURE, "SendEmail: MAPI: Invalid edit fields.");
				break;
			case MAPI_E_INVALID_RECIPS:
				returnStatus = Status(Status::FAILURE, "SendEmail: MAPI: Invalid recipients.");
				break;
			case MAPI_E_NOT_SUPPORTED:
				returnStatus = Status(Status::FAILURE, "SendEmail: MAPI: Not supported.");
				break;
			default:
				returnStatus = Status(Status::FAILURE, "SendEmail: MAPI: Unknown error.");
				break;
			}
		}

		for (U32 iRecipient = 0; iRecipient < (U32)m_recipients.Size(); iRecipient++)
		{
			free(pRecipientDescriptionList[iRecipient].lpszAddress);
		}
		delete[] pRecipientDescriptionList;

		for (U32 iFile = 0; iFile < (U32)m_files.Size(); iFile++)
		{
			free(pFileDescriptionList[iFile].lpszFileName);
		}
		delete[] pFileDescriptionList;

		return returnStatus;
	}
}