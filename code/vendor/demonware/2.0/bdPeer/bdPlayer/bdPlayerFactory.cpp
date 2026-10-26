// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPeer/bdPlayer/bdPlayerFactory.h>

BD_REGISTER_SINGLETON(bdPlayerFactoryImpl)

#define BD_LOG_LEVEL "playerFactory"

bdPlayer* bdPlayerFactoryImpl::create() const
{
	BD_ASSERT(m_creator != BD_NULL, "bdPlayerFactoryImpl::create, "
		"must set a command class with BD_REGISTER_PLAYER_CLASS before "
		"calling create.");

	return m_creator->create();
}

void bdPlayerFactoryImpl::setClass(bdCreatorBase<bdPlayer> *const creator)
{
	if (m_creator)
	{
		BD_WARN(BD_LOG_LEVEL,"Player class already set ,BD_REGISTER_PLAYER_CLASS should only be called once otherwise memory is leaked.");
	}
	else
	{
		m_creator = creator;
	}
}

bdPlayerFactoryImpl::bdPlayerFactoryImpl()
: m_creator(BD_NULL)
{
}

bdPlayerFactoryImpl::~bdPlayerFactoryImpl()
{
	if (m_creator)
	{
		delete m_creator;
	}
}

bdPlayerFactory::bdPlayerFactory()
{

}
