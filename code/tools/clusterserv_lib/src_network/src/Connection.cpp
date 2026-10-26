#include "Network/StdAfx.h"
#include "Network/Network.h"
#include "Network/Connection.h"
#include "Network/NetMessages.h"
#include "Network/SerializeBin.h"
#include "Network/Debug.h"
#include "zlib/compression.h"

#define MAX_NETWORK_BUFFER_SIZE (1024*1024)

#pragma optimize( "", off ) // turn off all optimizations

ost::Mutex connectionMutex; // this mutex is meant for printing stuff

// a new client has connected - add it to our list
Connection::Connection(ost::TCPSocket &socket, C_Network *pNetwork, char *pszName)
	:	m_pszName( pszName ),
		SocketPort( NULL, socket ) // wait for incoming connection on this socket
{
	Reset();
	Init(pNetwork);
}

// 
Connection::Connection(char *pszName)
	:	m_pszName( pszName ),
		SocketPort() // attempt to connect to a given address + port
{
	Reset();
}

// connect manually
bool Connection::Connect(const ost::IPV4Host &ih, ost::tpport_t port, C_Network *pService)
{
	// try to connect
	int r = 0;
	try
	{
		r = connect(ih,port);
	}
	catch (...)
	{
		return false;
	}
	if (r != 0)
		return false;

	// attach to service
	Init(pService);
	return true;
}

void Connection::Reset()
{
	m_pMsgBuffer = NULL;
	m_pSendQueue = NULL;
	m_bConnected = true;
	m_readState = Read_Header;
	m_nJobsCompleted = 0;
	m_iReadPosition = 0;
	m_nTotalBytesReceived = 0;
	m_pCurSendItem = NULL;
	m_pNetwork = NULL;

	m_QueueLock = new ost::Mutex( "send queue" );
}

// common initialization code
void Connection::Init(C_Network *pNetwork)
{
	m_pNetwork = pNetwork;
	strcpy(m_pszHostName, "?unknown?");

	// Set up non-blocking reads
	setCompletion( false );
	setLinger( false );

	// Set to time out in 10 seconds
	setTimer( 100000 );

	attach( pNetwork->GetService() );
}

void Connection::WaitForMessageQueueToBeEmpty()
{
	// wait for send queue to become empty
	while (1)
	{
		m_QueueLock->enter();

		if (!m_pSendQueue)
		{
			m_QueueLock->leave();
			break;
		}

		m_QueueLock->leave();

		ost::Thread::sleep(100);
	}
}

// clean up connection
Connection::~Connection()
{
	// delete remaining message receive buffer contents
	if (m_pMsgBuffer && m_pMsgBuffer != m_localBuffer)
		delete [] m_pMsgBuffer;

	// delete remaining message send queue contents
	WriteDataConnection *pWD2, *pWD = m_pSendQueue;
	while (pWD)
	{
		if (pWD->m_bDeleteData)
		{
			delete pWD->m_pData;
		}
		pWD2 = pWD;
		pWD = pWD->m_pPrev;
		delete pWD2;
	}

	delete m_QueueLock;
}

// send the contents of the send queue 
void Connection::SendQueue()
{
	WriteDataConnection *pItem = m_pCurSendItem;

	while (pItem)
	{
		int iSendSize = pItem->m_iMessageSize - pItem->m_iWritePosition;
		if (iSendSize > MAX_NETWORK_BUFFER_SIZE)
			iSendSize = MAX_NETWORK_BUFFER_SIZE;
		// has the item finished?
		if (iSendSize<=0)
		{
			if (pItem->m_bInitialUpdate)
			{
				g_InitialUpdateSemaphore->post();
				S_DEBUG( g_NetLog, m_pszName << ": finished sending initial update to " << m_pszHostName << "\n");
			}

			// finished - do cleanup
			if (pItem->m_bDeleteData)
				delete pItem->m_pData;

			// nothing to send here, so get pointer to next send in the queue
			WriteDataConnection *pNext = pItem->m_pNext;
			if (pNext) pNext->m_pPrev = NULL;
			delete pItem;
			pItem = pNext; // send next in the queue
			continue;
		}

		// send this item
		void *buf = pItem->m_pData + pItem->m_iWritePosition;
		int i = send(buf, iSendSize);
		if (i>0)
		{
			pItem->m_iWritePosition += i;
//			g_netdebugManager << m_pszName << ": SENT " << i << " / " << iSendSize << "\n";
		}
		else
		{
/*
			g_netdebugMgrMutex.enter();
#ifdef WIN32
			g_netdebugManager << m_pszName << ": send error " << WSAGetLastError() << "\n";
#else
			g_netdebugManager << m_pszName << ": send error (" << iSendSize << ")\n";
#endif
			g_netdebugMgrMutex.leave();
*/
		}

		// if the call to 'send' didn't manage to send all the data, break and wait until we can send more
		if (i<iSendSize)
			break;
	}

	// update current item + queue head
	m_pCurSendItem = pItem;
	if (!pItem)
	{
		m_pSendQueue = NULL;
		setDetectOutput( false );
	}
	else
		setDetectOutput( true );	// detect when we can send the rest of the data
}

// add to send queue, or if there is no queue, just send this
void Connection::AddToSendQueue(WriteDataConnection *p)
{
	m_QueueLock->enter();

//	g_netdebugManager << m_pszName << ": AddToSendQueue\n";

	if (m_pSendQueue)
	{
		// add this send to the start of the queue
		m_pSendQueue->m_pNext = p;
		p->m_pPrev = m_pSendQueue;
		m_pSendQueue = p;
		setDetectOutput( true );
	}
	else
	{
		// start this send right now
		m_pSendQueue = p;
		m_pCurSendItem = p;
		SendQueue();
	}

	m_QueueLock->leave();
}

// The send function sends data on a connected socket.
// bcopy = true -> make a copy of the data
// bcopy = false -> send this data - data will be deleted when it's finished sending
void Connection::Send(const void* Data, int Length, SendType type)
{
	// create a new buffer to send
	WriteDataConnection *p = new WriteDataConnection;
	p->m_iWritePosition = 0;
	p->m_iMessageSize = Length;
	p->m_pConnection = this;
	p->m_pNext = NULL;
	p->m_pPrev = NULL;
	p->m_bInitialUpdate = false;
	if (Length < LOCAL_BUFFER_SIZE && type == SendType_Copy)
	{
		p->m_pData = p->m_localBuffer;
		memcpy(p->m_localBuffer, Data, Length);
		p->m_bUsingLocalBuffer = true;
		p->m_bDeleteData = false;
	}
	else
	{
		p->m_bUsingLocalBuffer = false;
		switch (type)
		{
		case SendType_InitialUpdate:
			p->m_bInitialUpdate = true;
		case SendType_AsIs:
			p->m_pData = (char*)Data;
			p->m_bDeleteData = false;
			break;
		case SendType_Delete:
			p->m_pData = (char*)Data;
			p->m_bDeleteData = true;
			break;
		case SendType_Copy:
			assert(0); // this hasn't been implemented yet
			break;
		}
	}

	AddToSendQueue(p);
}

void Connection::disconnect()
{
	m_QueueLock->enter();
	if (m_pCurSendItem && m_pCurSendItem->m_bInitialUpdate)
	{
		// cancel the sending of the initial update
		g_InitialUpdateSemaphore->post();
		S_DEBUG( g_NetLog, m_pszName << ": cancelled sending initial update to " << m_pszHostName << "\n");
	}
	m_QueueLock->leave();

	S_DEBUG( g_NetLog, m_pszName << ": disconnected '" << m_pszHostName << "'\n");

	service->detach(this);

	m_pNetwork->m_bCompletionError = true;

	ost::Thread::sleep(1000);
}

void Connection::expired()
{
}

// called when input data is pending for this connection
void Connection::pending()
{
	// peek to see if there really is data to be read
	int dummy;
	int iData = peek(&dummy, 4);
//	g_netdebugManager << m_pszName << ":peek = " << iData << "\n";
	if (iData <= 0)
	{
		disconnect(); // received 'pending', but no data to receive = closed connection
		return; // no data - quit out without doing a 'receive'		
	}

	int iBytesLeft;
	int iBytesRead;
	char *pBuf;
	bool bNoMoreInput = false;
	do
	{
		switch (m_readState)
		{
		case Read_Header:
			{
				// transfer the header
				iBytesLeft = 4 - m_iReadPosition;
				pBuf = ((char*)&m_iMessageSize) + m_iReadPosition;
				iBytesRead = receive(pBuf, iBytesLeft);
//				g_netdebugManager << m_pszName << ":headerread = " << iBytesRead << " \n";
				if (iBytesRead >=0)
					m_iReadPosition += iBytesRead;

				// did we transfer the whole 4 bytes of the header?
				if (m_iReadPosition == 4)
				{
					m_iMessageSize = ntohl(m_iMessageSize);

					// yes, set up the read for the body
					if (m_iMessageSize < LOCAL_BUFFER_SIZE)
						m_pMsgBuffer = m_localBuffer;
					else
					{
						if (m_pMsgBuffer && m_pMsgBuffer != m_localBuffer)
							delete [] m_pMsgBuffer;
						m_pMsgBuffer = new char[m_iMessageSize];
					}
					m_iReadPosition = 0;
					m_readState = Read_Body;

//					g_netdebugManager << m_pszName << ":read header: " << m_iMessageSize << " bytes\n";
				}
			}
			break;

		case Read_Body:
			{
				// transfer the body
				iBytesLeft = m_iMessageSize - m_iReadPosition;
				pBuf = m_pMsgBuffer + m_iReadPosition;
				iBytesRead = receive(pBuf, iBytesLeft);
//				g_netdebugManager << m_pszName << ":bytesread = " << (m_iReadPosition + iBytesRead) << " / " << m_iMessageSize << "\n";
				if (iBytesRead >=0)
					m_iReadPosition += iBytesRead;

				assert(m_iReadPosition <= m_iMessageSize);
				if (m_iReadPosition == m_iMessageSize)
				{
					// the whole message has been read, so we can process it
					ProcessRawMessage(m_pMsgBuffer, m_iMessageSize);
					m_nTotalBytesReceived += m_iMessageSize + 4;
					// delete the message buffer we used
					if (m_pMsgBuffer != m_localBuffer &&
						m_pMsgBuffer != NULL)
					{
						delete [] m_pMsgBuffer;
						m_pMsgBuffer = NULL;
					}
					m_iReadPosition = 0;
					m_readState = Read_Header;
				}
			}
			break;
		}
	}
	while (iBytesRead>0);

//	g_netdebugManager << "pend\n";
}

// called when output data is pending for this connection
void Connection::output()
{
	m_QueueLock->enter();

//	g_netdebugManager << m_pszName << ": output\n";
	SendQueue();

	m_QueueLock->leave();
}

// the whole message has been read, so we can process it
void Connection::ProcessRawMessage(char *pData, int c)
{
	CBinarySerialize bs;
	int iDecompressedSize = ntohl( *((int*)pData) );
	char *pDecompressedData = NULL;

//	g_netdebugManager << "RECV: " << c << " - " << iDecompressedSize << "\n";

	// check whether the message was actually compressed
	if (iDecompressedSize > c-4)
	{
		// decompress the message using zlib
		CCompression cmp;
		pDecompressedData = new char [iDecompressedSize];
		cmp.Decompress((char*)pData+4, c-4, pDecompressedData, iDecompressedSize);
		bs.m_iFileLength = iDecompressedSize;
		bs.m_pData		 = pDecompressedData;
	}
	else
	{
		// this message was not compressed
		bs.m_iFileLength = c-4;
		bs.m_pData		 = (char*)pData+4;
	}

	// deserialize the object
	BaseObject *pObj = bs.ReadObject();
	if (pDecompressedData)
		delete [] pDecompressedData;

	m_pNetwork->ProcessMessage(this, pObj);
}

// serialize a message and send it over the network
void Connection::SendMessage(BaseObject *pMsg) 
{
	CBinarySerialize data;
	data.SerializeObject(pMsg);

	char 	*pCompressedData, *pCompressedDataPlus8;
	int		iCompressedSize;
	bool	bCompressed = true;

	// don't bother try to compress stuff if it's very small anyway
	pCompressedData = new char [data.m_iFileLength+8];
	if (data.m_iFileLength>32)
	{
		// compress the message using zlib
		CCompression c;
		// reserve 8 bytes for the header
		pCompressedDataPlus8 = pCompressedData+8;
		c.Compress(data.m_pData, data.m_iFileLength, &pCompressedDataPlus8, &iCompressedSize);
	}
	else
		iCompressedSize=data.m_iFileLength;

	// if the compression didn't do anything, send the message uncompressed
	if (iCompressedSize>=(int)data.m_iFileLength)
	{
		iCompressedSize = data.m_iFileLength;
		memcpy(pCompressedData+8, data.m_pData, data.m_iFileLength);
		bCompressed = false;
	}

	// set the length of this message first
	((int*)pCompressedData)[0] = htonl(iCompressedSize + 4);
	// set the original data length
	((int*)pCompressedData)[1] = htonl(data.m_iFileLength);
	// send the message
	Send(pCompressedData, iCompressedSize + 8, SendType_Delete);

	delete data.m_pData;
	// (pCompressedData will be destroyed by the send operation once it is completed)
}

// NETWORK

C_Network::C_Network()
{
}

void C_Network::Send(Connection *pConnection, const void* Data, int Length, SendType type)
{
	pConnection->Send(Data, Length, type);
}

void C_Network::SendMessage(Connection *pConnection, BaseObject *pMsg)
{
	pConnection->SendMessage(pMsg);
}

