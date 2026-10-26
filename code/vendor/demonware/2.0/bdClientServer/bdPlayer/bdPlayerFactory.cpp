// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdClientServer/bdPlayer/bdPlayerFactory.h>

#define BD_LOG_LEVEL "bdClientServer/playerfactory"

BD_REGISTER_SINGLETON(bdPlayerFactoryImpl)

bdPlayer* bdPlayerFactoryImpl::create() const
{
	BD_ASSERT(m_creator != BD_NULL, "bdPlayerFactoryImpl::create, "
		"must set a command class with BD_REGISTER_PLAYER_CLASS before "
		"calling create.");

	return m_creator->create();
}

bdUInt32 bdPlayerFactoryImpl::getSizeOf() const
{
	BD_ASSERT(m_creator != BD_NULL, "bdPlayerFactoryImpl::getSizeOf, "
		"must set a command class with BD_REGISTER_PLAYER_CLASS before "
		"calling getSizeOf.");

	return m_creator->getSizeOf();
}

void bdPlayerFactoryImpl::setClass(bdCreatorBase<bdPlayer> *const creator)
{
	if (m_creator)
	{
		BD_WARN(BD_LOG_LEVEL,"player class already set (BD_REGISTER_PLAYER_CLASS should only be called once otherwise memory is leaked).");
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
