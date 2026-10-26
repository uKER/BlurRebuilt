// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdClientServer/bdCommand/bdCommandFactory.h>

#define BD_LOG_LEVEL "bdClientServer/commandfactory"

BD_REGISTER_SINGLETON(bdCommandFactoryImpl)

bdUserCommand* bdCommandFactoryImpl::create() const
{
	BD_ASSERT(m_creator != BD_NULL, "bdCommandFactoryImpl::create, "
		"must set a command class with BD_REGISTER_COMMAND_CLASS before "
		"calling create.");

	return m_creator->create();
}


void bdCommandFactoryImpl::setClass(bdCreatorBase<bdUserCommand> *const creator)
{
	if (m_creator)
	{
		BD_WARN(BD_LOG_LEVEL,"command class already set (BD_REGISTER_COMMAND_CLASS should only be called once otherwise memory is leaked).");
	}
	else
	{
		m_creator = creator;
	}
}

bdCommandFactoryImpl::bdCommandFactoryImpl()
: m_creator(BD_NULL)
{
}

bdCommandFactoryImpl::~bdCommandFactoryImpl()
{
	if (m_creator)
	{
		delete m_creator;
	}
}

bdCommandFactory::bdCommandFactory()
{

}
