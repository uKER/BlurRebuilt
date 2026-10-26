#include "SystemPch.h"

#include <system/network/Address.h>
#include <system/network/Socket.h>
#include <system/network/SMTPClient.h>
#include <system/misc/Base64.h>

/// connect to an SMTP server and send e-mail
/// includes authentication
namespace Network
{

/// Connect to an SMTP server
bool SMTPClient::Connect(Util::C_String hostname)
{
	if (!Socket::Connect( Network::Address( hostname ), 25 ))
		return false;

	if (GetResponse() != 220)
		return false;

	Send("HELO");

	if (GetResponse() != 250)
		return false;

	// [Blur2PC] EHLO names this machine; retail named a Bizarre mail server.
	char ehloName[256] = "localhost";
	gethostname(ehloName, sizeof(ehloName));
	Send(Util::C_String("EHLO ") + ehloName);

	if (GetResponse() != 250)
		return false;

	return true;
}



//lint -restore

/// log into the SMTP server
bool SMTPClient::Login(Util::C_String username, Util::C_String password)
{
	Send("AUTH LOGIN");

	GetResponse();
//	printf("%s\n", Util::Base64::Decode(m_sResponse));

	Send(Util::Base64::Encode(username));

	GetResponse();
//	printf("%s\n", Util::Base64::Decode(m_sResponse));

	Send(Util::Base64::Encode(password));

	GetResponse();

	return true;
}

/// send a string to the server
void SMTPClient::Send(Util::C_String s)
{
	s = s + Util::C_String("\r\n");
//	printf("%s", s);

	Socket::Send(s.Chars(), s.Length());
}

/// get response number from the server
int SMTPClient::GetResponse()
{
	char buffer[1024];
	Util::C_String ret = "";

	int c = Socket::Receive(buffer, 1024);
	if (c>=0)
	{
		buffer[c] = 0;
//		printf("%s", buffer);
		m_sResponse = Util::C_String(buffer+4);
	}
	return atoi(buffer);
}

/// Send an e-mail
bool SMTPClient::SendEMailHeader(Util::C_String sender, Util::C_String recipient, Util::C_String subject, Util::C_String senderFriendlyName)
{
	Send(Util::C_String("MAIL FROM:") + sender);

	if (GetResponse() != 250)
		return false;

	Send(Util::C_String("RCPT TO:") + recipient);

	if (GetResponse() != 250)
		return false;

	Send("DATA");

	if (GetResponse() != 354)
		return false;

	if (senderFriendlyName == "")
		senderFriendlyName = sender;

	Send(  	Util::C_String("") +
			"From: " + sender + " (" + senderFriendlyName + ")\r\n" + 
			"To: " + recipient + "\r\n" +
			"Subject: " + subject + "\r\n"
			);

	return true;
}

/// Send an e-mail
bool SMTPClient::SendEMailHeaderHTML(Util::C_String sender, Util::C_String recipient, Util::C_String subject, Util::C_String senderFriendlyName)
{
	Send(Util::C_String("MAIL FROM:") + sender);

	if (GetResponse() != 250)
		return false;

	Send(Util::C_String("RCPT TO:") + recipient);

	if (GetResponse() != 250)
		return false;

	Send("DATA");

	if (GetResponse() != 354)
		return false;

	if (senderFriendlyName == "")
		senderFriendlyName = sender;

	Send(  	Util::C_String("") +
			"Mime-Version: 1.0\r\n" +
			"From: " + sender + " (" + senderFriendlyName + ")\r\n" + 
			"To: " + recipient + "\r\n" +
			"Subject: " + subject + "\r\n" +
			"Content-description: Mail message body\r\n" +
			"Content-Type: text/html; charset=US-ASCII\r\n" +
			"Content-Transfer-Encoding: 7bit\r\n"
			);

	return true;
}

/// send part of the message body
void SMTPClient::SendEMailBody(Util::C_String messageline)
{
	Socket::Send(messageline.Chars(), messageline.Length());
}

/// end the e-mail body
bool SMTPClient::FinishEMail()
{
	Send( Util::C_String("\r\n.\r\n") );

	if (GetResponse() != 250)
		return false;

	return true;
}

/// send quit message
bool SMTPClient::Quit()
{
	Send("QUIT");

	if (GetResponse() != 221)
		return false;

	return true;
}

}
