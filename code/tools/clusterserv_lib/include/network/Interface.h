#ifndef __CLUSTER_INTERFACE_H__
#define __CLUSTER_INTERFACE_H__

// client thread
class ClientThread : public ost::Thread
{
public:
	int		m_iExitCode;
};

// start client in a separate thread
ClientThread *StartClient(char *pszIP = 0);

// create server. call JobServer->start() to start the server in a separate thread
JobServer *CreateServer(BaseObject *pInitialUpdate);


#endif
