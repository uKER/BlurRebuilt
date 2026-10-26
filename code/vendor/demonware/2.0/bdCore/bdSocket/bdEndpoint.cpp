// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdSocket/bdEndpoint.h>
#include <bdCore/bdSocket/bdCommonAddr.h>
#include <bdCore/bdContainers/bdHashMap.h>


bdEndpoint::bdEndpoint()
{
}

bdEndpoint::bdEndpoint(const bdCommonAddrRef addr,
					   const bdSecurityID& secID)
: m_ca(addr),
  m_secID(secID)
{
}

bdBool bdEndpoint::operator == (const bdEndpoint &other) const
{
	// Only safe to get the hashes if the 2 bdCommonAddrRefs are valid
	bdBool result = false;
	if( m_ca.notNull() && other.m_ca.notNull())
	{
		result = m_ca->getHash() == other.m_ca->getHash();
		for( bdUInt i=0; result && i<8; i++ )
		{
			result = result && ( m_secID.ab[i] == other.m_secID.ab[i] );
		}
	}
	else if(m_ca.isNull() && other.m_ca.isNull())
	{
		// Todo  - think about this..
		// This is an interesting thing...
		// is Null == Null
		result = true;
	}
	return result;
}

bdCommonAddrRef bdEndpoint::getCommonAddr() const
{
	return m_ca;
}

const bdSecurityID& bdEndpoint::getSecID() const
{
	return m_secID;
}

bdUInt bdEndpointHashingClass::getHash(const bdEndpoint &other) const
{
	bdUInt hash = 0;
	if(other.getCommonAddr().notNull())
	{
		hash = other.getCommonAddr()->getHash();	
		bdHashingClass hasher;
		hash += hasher.getHash(other.getSecID());

	}
	return hash;
}

bdUInt bdEndpoint::getSerializedLength() const
{
	// TODO: write this in terms of commonAddr.getSerializedLength() + securityId.getSerializedLength() 
	bdUInt length = BD_COMMON_ADDR_SERIALIZED_SIZE + BD_SECURITY_ID_LENGTH;
	return length;
}

bdBool bdEndpoint::serialize(void *data,
						 const bdUInt size,
						 const bdUInt offset,
						 bdUInt &newOffset) const
{
	bdBool status = true;
	newOffset = offset;

	const bdBool validStart = offset <= size;
	BD_ASSERT(validStart, "Offset is past the end of the destination buffer.");

	if( getSerializedLength() <= size-offset )
	{
		// Serialize the common Addr 
		m_ca->serialize( reinterpret_cast<bdUByte8 *>( data ) + newOffset );
		newOffset = newOffset + BD_COMMON_ADDR_SERIALIZED_SIZE;

		// Serialize the security ID
		bdMemcpy( reinterpret_cast<bdUByte8 *>( data )+ newOffset, m_secID.ab, BD_SECURITY_ID_LENGTH );
		newOffset = newOffset + BD_SECURITY_ID_LENGTH;
	}
	else
	{
		status = false;
	}

	if (!status)
	{
		newOffset = offset;
	}
	return status;
}

bdBool bdEndpoint::deserialize(	bdCommonAddrRef me,	
								const void* data,
								const bdUInt size,
								const bdUInt offset,
								bdUInt &newOffset)
{
	bdBool status = true;
	newOffset = offset;

	// Deserialize the commonAddr, if there's enough room
	if( newOffset + BD_COMMON_ADDR_SERIALIZED_SIZE <= size )
	{
		if( m_ca.isNull() )
		{
			m_ca = new bdCommonAddr();
		}
		m_ca->deserialize( me, reinterpret_cast<const bdUByte8 *>( data ) + newOffset );
		newOffset = newOffset + BD_COMMON_ADDR_SERIALIZED_SIZE;
	}
	else
	{
		status = false;
	}

	// Deserialize the security ID, if there's enough room
	if( status && newOffset + BD_SECURITY_ID_LENGTH <= size )
	{
		bdMemcpy( m_secID.ab, reinterpret_cast<const bdUByte8 *>( data ) + newOffset, BD_SECURITY_ID_LENGTH );
		newOffset = newOffset + BD_SECURITY_ID_LENGTH;
	}
	else
	{
		status = false;
	}

	if (!status)
	{
		newOffset = offset;
	}
	return status;
}

