#ifndef __SERVER_H__
#define __SERVER_H__

class JobScheduler;
class JobServer;
class Connection;

typedef Ut::UtVector< Ut::DynamicVector< Connection* > > JobServerClientList;

#define MAX_SIMULTANEOUS_INITIAL_UPDATES 4
extern ost::Semaphore *g_InitialUpdateSemaphore;

class InitialUpdateObject
{
public:
	InitialUpdateObject(BaseObject *pInitialUpdate);
	~InitialUpdateObject() { delete [] m_pCompressedData; }

	char	*m_pCompressedData;
	int		m_iOriginalSize;
	int		m_iCompressedSize;
};

#define CLUSTERSERV_FLAG_WINDOWS	1
#define CLUSTERSERV_FLAG_LINUX		2
#define CLUSTERSERV_FLAG_PVS		4

struct FarmClientInfo
{
	int			iJobID;
	std::string sCommandline;
	int			iJobFlag;
};

class JobServer : protected C_Network, public IServer, public ost::Thread
{
public:
	// create a server.
	// pClient = object which will be deserialized on the client and process the jobs
	//			 (must be inherited from ClientInterface)
	JobServer(BaseObject *pClient);
	~JobServer();

	// set the commandline string to run on screensaver farm clients
	void			AddFarmClientCommandline(char *psz, int iJobFlag);
	// make the server thread exit itself
	void			Exit();

private:
	void run();
	void Process();
	void AddClient(ost::TCPSocket *socket);
	void AssociateClientWithCompletionPort(int iClient);
	virtual void ProcessMessage(Connection *, BaseObject *pMsg);

	bool IsRunning(int Sleep = 0) const;
	bool IsEnabled() const;

	void CheckFarmClientIsRunning();
	void CheckFarmClientIsRunning(FarmClientInfo &info);
	void DeleteFarmClientJob();

	JobServerClientList	m_ClientList;

	bool				m_QuitThread;
	int					m_iLastClientID;
	InitialUpdateObject	m_InitialUpdate;

	Ut::Vector<FarmClientInfo> m_FarmClientInfo;
	U32					m_iMsSinceLastCheckedClient;
};

#endif
