#ifndef __NETWORK_H__
#define __NETWORK_H__

class Connection;

enum SendType
{
	SendType_Copy,
	SendType_AsIs,
	SendType_Delete,
	SendType_InitialUpdate,
};

class C_Network
{
public:
			C_Network();
	virtual ~C_Network();
	void	Process();
	virtual void ProcessMessage(Connection *pConnection, BaseObject *pMsg) = 0;
	void	Send(Connection *p, const void* Data, int Length, SendType type = SendType_Copy);
	void	SendMessage(Connection *pConnection, BaseObject *pMsg);
			// get a socket service with an empty slow
	ost::SocketService *GetService();

	Ut::Vector<ost::SocketService*>	m_Services;

	bool				m_bCompletionError;

	int					m_nBytesSent;
	int					m_nBytesReceived;
};

#endif
