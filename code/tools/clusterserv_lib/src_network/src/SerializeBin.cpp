// this code uses serializes an object and stores the resulting binary data + vice versa
#include "Network/StdAfx.h"
#include "Network/Network.h"
#include "Network/SerializeBin.h"

#include <sstream>
#include <vector>
#include <list>
#include <assert.h>

#include "tools_common/memory_stream.hpp"

#include "System/Memory.h"

// 
void CBinarySerialize::Purge()
{
	if (!m_pData) return;

	delete [] m_pData; // CMemFile uses malloc to allocate it's data
	m_pData = NULL;
}

// serialize an object and store the resulting data in m_pData
void CBinarySerialize::SerializeObject(BaseObject *pObject, int iGrowSize)
{
	MEMORY_TRACKER_PUSH("tools.clusterserv.serializeobject");

	// create memory stream
	std::memorystream ms;
	ms.getbuf()->setgrowsize( iGrowSize );
	ost::Engine oa(ms, ost::Engine::modeWrite);

	// serialize the object into the stream
	oa << pObject;

	// this transfers ownership of the buffer from the memstream to us
	m_iFileLength = (int)ms.getbuf()->getlength();
	m_pData = ms.getbuf()->detach();

	MEMORY_TRACKER_POP("tools.clusterserv.serializeobject");
}

// de-serialize an object, the serialized data is in m_pData
BaseObject *CBinarySerialize::ReadObject()
{
	MEMORY_TRACKER_PUSH("tools.clusterserv.readobject");

	// create memory stream containing our serialized data
	std::memorystream ms;
	ost::Engine ia(ms, ost::Engine::modeRead);

	// pass ownership of our memory buffer to the memory stream
	ms.getbuf()->attach(m_pData, m_iFileLength);
	m_pData = NULL;

	// de-serialize the object
	BaseObject *pObj = NULL;
	ia >> pObj;

	// detach the buffer or it will be freed when the memorystream goes out of scope
	ms.getbuf()->detach();

	MEMORY_TRACKER_POP("tools.clusterserv.readobject");

	return pObj;
}

