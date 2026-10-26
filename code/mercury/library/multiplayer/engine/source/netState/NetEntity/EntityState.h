#ifndef _NET_STATE
#define _NET_STATE

#include "netState/netState/StateDataBuffer.h" 
#include "netState/netEntity/EntityState.h" 
#include "netState/netEntity/entityHeader.h"
#include "netState/compression/entityCompressor.h"

enum E_Error
{
	E_Error_Compression,
	E_Error_Deserialise
};

// net state interface
class C_EntityState
{
public:
						C_EntityState(const C_Header& xHeader, class netSerialisable* pxEntity, int uCreatedSQN, int uInitialSQN, int uSize, bool bBuffered);
						~C_EntityState();
	const C_Header&		Header() const	{ return m_xHeader; }
	C_Header&			Header()		{ return m_xHeader; }
	
	C_StateDataBuffer*	GetStateDataBuffer()
						{ 
							return m_pxStateData;
						}
	C_StateData*		GetStateData(bool bCreateIfNode = false)
						{ 
							return m_pxStateData ? m_pxStateData->GetStateData(0, bCreateIfNode) : NULL;
						}	
	int					CurrentState() const { return m_pxStateData ? m_pxStateData->CurrentState() : -2; }
	void				CreateInitialState();
	void				MigrateToNewServer(int uNewServerId, int uOldServerId, int uMyHostId);
	void				Delete(const int uDeletedStateId);
	bool				CreationRequired(int uClientsAck);
	bool				StateToEntity();
	bool				EntityToState(int uSqn);
	bool				EntityStateToPacket(C_Header& xHeader, int uAck, class netStream* stream);
	bool				StateToStream(class netStream* stream);
	bool				StreamToState(class netStream* stream, int uCurrentState);

	bool				PrepareClientsState(int uClientsState);
	bool				PrepareFullState();

	// debug
	bool					DebugState(char* str, int uBufferSize);
	bool					DebugMemory(char* str, int uBufferSize);
	bool					DebugEntity(char* str, int uBufferSize, int uGameStateId);
	const C_Compression&	Compression() { return m_xCompression; }

protected:
	C_Header				m_xHeader;
	class netSerialisable*	m_pxNetEntity;	// TODO: should be const!
	C_StateDataBuffer*		m_pxStateData;
	C_Compression			m_xCompression;
private:
	const int				m_uCreatedStateSQN;
};

#endif
