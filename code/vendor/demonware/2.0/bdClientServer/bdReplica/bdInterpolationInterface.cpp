// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdClientServer/bdReplica/bdInterpolationInterface.h>

bdInterpolationInterface::bdInterpolationInterface()
: m_dirtyFlags(0)
{
}

bdInterpolationInterface::~bdInterpolationInterface()
{
}

void bdInterpolationInterface::applyCommand(bdUserCommand *const,
											const bdBool)
{
}

void bdInterpolationInterface::extrapolate(const bdFloat32, 
										   const bdBool)
{
}

void bdInterpolationInterface::serialize(bdBitBufferRef)
{
}

bdBool bdInterpolationInterface::deserialize(bdBitBufferRef)
{
	return true;
}

void bdInterpolationInterface::setDirtyFlags(const bdUInt32 flags)
{
	m_dirtyFlags |= flags;
}

bdUInt32 bdInterpolationInterface::getDirtyFlags() const
{
	return m_dirtyFlags;
}

bdBool bdInterpolationInterface::testDirtyFlags(const bdUInt32 flags) const
{
	return (m_dirtyFlags & flags)?true:false;
}

void bdInterpolationInterface::clearDirtyFlags(const bdUInt32 flags)
{
	m_dirtyFlags &= ~flags;
}

void bdInterpolationInterface::writeFullState(bdBitBufferRef buffer)
{	
	const bdUInt32 backupFlags = m_dirtyFlags;
	m_dirtyFlags = BD_UINT32_MAX;
	serialize(buffer);
	m_dirtyFlags = backupFlags;
}
