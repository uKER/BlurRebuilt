// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdClientServer/bdReplica/bdReplicaFactory.h>

BD_REGISTER_SINGLETON(bdReplicaFactoryImpl)

bdBool bdReplicaFactoryImpl::registerClass(const bdUInt32 type, 
										 bdCreatorBase<bdReplica> *creator)
{
	m_maxType = BD_MAX(type, m_maxType);
	const bdBool registered = bdFactory<bdReplica>::registerClass(type, creator);
	return registered;
}

bdUInt32 bdReplicaFactoryImpl::getMaxType() const
{
	return m_maxType;
}

bdReplicaFactoryImpl::bdReplicaFactoryImpl()
: m_maxType(0)
{
}

bdReplicaFactoryImpl::~bdReplicaFactoryImpl()
{
}

bdReplicaFactory::bdReplicaFactory()
{
}
