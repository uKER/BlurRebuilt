#ifndef _NET_ENTITY_HANDLE
#define _NET_ENTITY_HANDLE

#include "netState/netHandle/NetHandle.h"
#include "netStream/netSerialise.h"
#include "netstate/StateEnums.h"

// C_GameNetEntity
class C_GameNetEntity : public netSerialisable 
{
public:
	C_GameNetEntity(const C_NetHandle& xNetHandle, int uType);

	C_NetHandle&	NetHandle()			{ return m_xNetHandle; }
	int				Type() const		{ return m_uType; }

	virtual bool	serialise(class netStream* stream);
	virtual bool	deserialise(class netStream* stream);

	int				CalculateSize();
	E_HandleType	HandleType() const;

private:
	//C_GameHandle	m_xGameHandle;
	C_NetHandle		m_xNetHandle;
	const int		m_uType;
};

#endif