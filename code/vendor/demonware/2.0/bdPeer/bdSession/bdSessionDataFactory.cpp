// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPeer/bdSession/bdSessionDataFactory.h>

BD_REGISTER_SINGLETON(bdSessionDataFactoryImpl)

#define BD_LOG_LEVEL "sessionDataFactory"

bdSessionData* bdSessionDataFactoryImpl::create() const
{
	BD_ASSERT(m_creator != BD_NULL, "Must set a session data class with "
		"BD_REGISTER_SESSION_DATA_CLASS before calling create.");

	return m_creator->create();
}

void bdSessionDataFactoryImpl::setClass(bdCreatorBase<bdSessionData> *const creator)
{
	if (m_creator)
	{
		BD_WARN(BD_LOG_LEVEL,"Player class already set ,BD_REGISTER_SESSION_DATA_CLASS  should only be called once otherwise memory is leaked.");
	}
	else
	{
		m_creator = creator;
	}
}

bdSessionDataFactoryImpl::bdSessionDataFactoryImpl()
: m_creator(BD_NULL)
{
}

bdSessionDataFactoryImpl::~bdSessionDataFactoryImpl()
{
	if (m_creator)
	{
		delete m_creator;
	}
}

bdSessionDataFactory::bdSessionDataFactory()
{

}
