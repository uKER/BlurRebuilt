#ifndef __NETWORK_CLIENTINTERFACE_H__
#define __NETWORK_CLIENTINTERFACE_H__

class ClientInterface : public BaseObject
{
public:
	DECLARE_PERSISTENCE(ClientInterface);

	virtual ~ClientInterface() {}
	virtual void	Initialize() { assert(0); }
	virtual BaseObject *Process(BaseObject *pJob) { assert(0); return NULL; }
	virtual void	CleanUp()	 { assert(0); }

};

#endif