#ifndef _ENTITY_STATE_BUFFER
#define _ENTITY_STATE_BUFFER

#include "StateData.h"

// entity state buffer
class C_StateDataBuffer
{
public:
							C_StateDataBuffer(const int uMaxStates, const int uEntityDataSize);
							~C_StateDataBuffer();
	void					NewBuffer(int uMaxStates, int uEntityDataSize);
	void					DestroyBuffer();
	void					ResizeBuffer(int uMaxStates, int uEntityDataSize);
	int						CurrentState() const;
	int						NumStates()				{ return m_uNumStates; }	
	int						MaxStates()				{ return m_uMaxStates; }
	C_StateData*			Latest(bool bCreateIfNone = false);
	C_StateData*			FindState(const int uStateNum) const;
	void					Acknowledge(int uSqn);
	class C_Compression&	GetCompression(int i);
	bool					NewState(const C_StateData& xState);
	C_StateData*			GetStateData(int i, const bool bCreateIfNone = false);
	bool					PrepareClientsState(class C_Compression& xCompress, int uClientState);
private:
	void					IncrementBuffer();
private:
	class C_BufferedEntity**	m_pxBuffer;
	int							m_uIndex;
	int							m_uNumStates;
	int							m_uMaxStates;
};

#endif