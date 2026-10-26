#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#pragma once

#define _CONNECTION_H_

#define OP_READ_HEADER		0
#define OP_READ_BODY		1
#define OP_WRITE			2

#define LOCAL_BUFFER_SIZE 512

class Connection;

class WriteDataConnection
{
public:

	WriteDataConnection *StartSend();

	char		*m_pData;	// actual data
	int			m_iMessageSize;
	char		m_localBuffer[LOCAL_BUFFER_SIZE];
	bool		m_bUsingLocalBuffer;
	int			m_iWritePosition;
	bool		m_bDeleteData;
	bool		m_bInitialUpdate;
	int			m_nBytesSent;
	WriteDataConnection	*m_pNext, *m_pPrev;
	Connection	*m_pConnection;
};

class Connection : public virtual ost::SocketPort
{
public:
	friend class WriteDataConnection;

				Connection(ost::TCPSocket &socket, C_Network *pService, char *pszName);
				Connection(char *pszName);
	virtual		~Connection();

	int			m_iID;
	char		m_localBuffer[LOCAL_BUFFER_SIZE];
	bool		m_bUsingLocalBuffer;
	ost::Mutex	*m_QueueLock;
	bool		m_bConnected;
	int			m_nJobsCompleted;
	char		m_pszHostName[512];
	C_Network	*m_pNetwork;

	int			m_nTotalBytesReceived;

	// read 
	char		*m_pMsgBuffer;
	int			m_iReadPosition;
	int			m_iMessageSize;

	char		*m_pszName;

	enum ReadState
	{
		Read_Header,
		Read_Body,
	};

	ReadState	m_readState;

	// write 
	WriteDataConnection		*m_pSendQueue;
	WriteDataConnection		*m_pCurSendItem;

	// overloads
	void	expired();
	void	disconnect();
	void	pending();
	void	output();

	void AddToSendQueue(WriteDataConnection *p);
	void SendMessage(BaseObject *pMsg);
	void Send(const void* Data, int Length, SendType type = SendType_Copy);
	void WaitForMessageQueueToBeEmpty();
	void ProcessRawMessage(char *pData, int c);
	void Init(C_Network *pService);
	void Reset();
	void SendQueue();
	bool Connect(const ost::IPV4Host &ih, ost::tpport_t port, C_Network *pService);
};

#endif
