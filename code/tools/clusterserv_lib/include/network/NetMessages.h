#pragma once

class JobServer;
class JobClient;
class Connection;

// parent message class
class NETMSG_GENERIC : public BaseObject
{
	DECLARE_PERSISTENCE( NETMSG_GENERIC );

	virtual void Process(Connection *pConnection, JobServer *p) { assert(0); }
	virtual void Process(Connection *pConnection, JobClient *p) { assert(0); }
};

// client requesting to be connected to the server
class NETMSG_JOB_CLIENTINFO : public NETMSG_GENERIC
{
public:
	DECLARE_PERSISTENCE( NETMSG_JOB_CLIENTINFO );

	std::string m_csName;

	virtual void Process(Connection *pConnection, JobServer *p);

	// example of how to add extra code to serialize the contents of a message
	void write( ost::Engine& ar )
	{
		ar << m_csName;
	}
	void read( ost::Engine& ar )
	{
		ar >> m_csName;
	}
};

// this message is sent from the server to the client. it contains job information
class NETMSG_JOB : public NETMSG_GENERIC
{
public:
	DECLARE_PERSISTENCE(NETMSG_JOB);

	NETMSG_JOB() {};
	NETMSG_JOB(int i, BaseObject *p) : m_iJob(i), m_pJob(p) {};

	virtual void Process(Connection *pConnection, JobClient *p);

	int		 m_iJob;
	BaseObject *m_pJob;

	void write( ost::Engine& ar )
	{
		ar << m_iJob << m_pJob;
	}
	void read( ost::Engine& ar )
	{
		ar >> m_iJob >> m_pJob;
	}
};

// this message is sent from the client back to the server. it contains the job results
class NETMSG_JOB_RESULT : public NETMSG_GENERIC
{
public:
	DECLARE_PERSISTENCE(NETMSG_JOB_RESULT);

	int		 m_iJob;
	BaseObject *m_pResult;

	void Process(Connection *pConnection, JobServer *p);

	void write( ost::Engine& ar )
	{
		ar << m_iJob << m_pResult;
	}
	void read( ost::Engine& ar )
	{
		ar >> m_iJob >> m_pResult;
	}
};

// this message is sent from the client to request a job
class NETMSG_REQUEST_JOB : public NETMSG_GENERIC
{
public:
	DECLARE_PERSISTENCE(NETMSG_REQUEST_JOB);

	void Process(Connection *pConnection, JobServer *p);

};

// client receiving the initial data (the integrator + spatial hierarchy)
class NETMSG_INITIALUPDATE: public NETMSG_GENERIC
{
public:
	DECLARE_PERSISTENCE(NETMSG_INITIALUPDATE);

	BaseObject *pData;

	void Process(Connection *pConnection, JobClient *p);
	void write( ost::Engine& ar )
	{
		ar << pData;
	}
	void read( ost::Engine& ar )
	{
		ar >> pData;
	}
};

// this message is sent from the client to request a job
class NETMSG_QUIT : public NETMSG_GENERIC
{
public:
	DECLARE_PERSISTENCE(NETMSG_QUIT);

	void Process(Connection *pConnection, JobClient *p);
};
