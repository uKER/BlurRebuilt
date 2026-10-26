#ifndef _BUFFERED_ENTITY
#define _BUFFERED_ENTITY

#include "netState/compression/entityCompressor.h"
#include "netState/netEntity/EntityState.h"

class C_BufferedEntity
{
public:
					C_BufferedEntity(int uSize);
	void			SetState(const C_StateData& xNewState, const C_StateData& xPrevState);
	
	C_StateData&	State();
	C_Compression&  Compression();

private:
	C_Compression	m_xCompressData;	// compression flags between this state and the previous one
	C_StateData		m_xState;			// full state for this entity
};

#endif