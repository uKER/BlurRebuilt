// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdNet/bdLANDiscovery/bdGameInfoFactory.h>

BD_REGISTER_SINGLETON(bdGameInfoFactoryImpl)

#define BD_LOG_LEVEL "gameInfoFactory"

bdGameInfo* bdGameInfoFactoryImpl::create() const
{
	BD_ASSERT(m_creator != BD_NULL, "bdGameInfoFactoryImpl::create, "
		"must set a game info class with BD_REGISTER_GAME_INFO_CLASS before "
		"calling create.");

	return m_creator->create();
}


void bdGameInfoFactoryImpl::setClass(bdCreatorBase<bdGameInfo> *const creator)
{
	if (m_creator)
	{
		BD_WARN(BD_LOG_LEVEL,"Game info class already set ,BD_REGISTER_GAME_INFO_CLASS should only be called once otherwise memory is leaked.");
	}
	else
	{
		m_creator = creator;
	}
}

bdGameInfoFactoryImpl::bdGameInfoFactoryImpl()
: m_creator(BD_NULL)
{
}

bdGameInfoFactoryImpl::~bdGameInfoFactoryImpl()
{
	if (m_creator)
	{
		delete m_creator;
	}
}

bdGameInfoFactory::bdGameInfoFactory()
{

}
