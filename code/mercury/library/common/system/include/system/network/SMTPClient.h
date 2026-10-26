#ifndef __SYSTEM_NETWORK_SMTP_H__
#define __SYSTEM_NETWORK_SMTP_H__

namespace Network
{

/// connect to an SMTP server and send e-mail. Includes authentication
class SMTPClient : TCPSocket
{
public:
					/// Connect to an SMTP server
	bool			Connect(Util::C_String hostname);

					/// log into the SMTP server
	bool			Login(	Util::C_String username,
							Util::C_String password);

					/// Send an e-mail header (this must be followed by e-mail body)
	bool			SendEMailHeader(	Util::C_String sender, 
										Util::C_String recipient, 
										Util::C_String subject,
										Util::C_String senderFriendlyName = "");

					/// Send an e-mail header (this must be followed by e-mail body)
	bool			SendEMailHeaderHTML(	Util::C_String sender, 
											Util::C_String recipient, 
											Util::C_String subject,
											Util::C_String senderFriendlyName = "");

					/// send the e-mail body (or a part thereof)
	void			SendEMailBody( Util::C_String message);

					/// mark the end of the message body and send the actual e-mail
	bool			FinishEMail();

					/// send quit message
	bool			Quit();

protected:
					/// send a string to the server
	void			Send(Util::C_String s);
					/// get response number from the server
	int				GetResponse();

	Util::C_String	m_sResponse;
};

}

#endif
