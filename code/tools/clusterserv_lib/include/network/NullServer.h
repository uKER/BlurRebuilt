#ifndef __NULL_SERVER_H__
#define __NULL_SERVER_H__

class ClientInterface;
class JobScheduler;

class NullServer : public IServer, public ost::Thread
{
public:
	NullServer(BaseObject *pInitialUpdate);
	~NullServer();

	void		Exit();

	bool		m_bExitThread;

	void		run();
	void		ProcessJobs();

private:
	ClientInterface	*m_pInitialUpdate;
};

#endif
