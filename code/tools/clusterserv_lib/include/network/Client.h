#pragma once

#ifdef _CONNECTION_H_

class InitialClientUpdate;
class ClientInterface;

class JobClient : public C_Network, public ClientThread
{
public:
	JobClient(char *pszIP);
	~JobClient();

	void run();
	bool Connect();
	void Exit();
	void SendMessage(BaseObject *pMsg);

	bool IsRunning(int Sleep = 0) const;
	bool IsEnabled() const;

	virtual void ProcessMessage(Connection *, BaseObject *pMsg);

	ost::Event			*m_ReceivedInitialUpdateEvent;
	ost::Semaphore		*m_JobCountSemaphore;
	JobQueue			m_JobQueue;
	ost::IPV4Host		m_ServerAddr;
	bool				m_QuitThread;
	Connection			*m_pConnection;
	ClientInterface		*m_pInterface;
	bool				m_bDone;
	int					m_nClientStall;
};

#endif

ClientThread *StartClient(char *pszIP);
void StartClientProcess(char *psz);
void StartFarmClient(int n);
void CommandLine(char *psz);
