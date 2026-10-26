// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A singleton to create user command objects as required.

#ifndef BD_COMMAND_FACTORY_H
#define BD_COMMAND_FACTORY_H

#include <bdCore/bdCore.h>
#include <bdCore/bdUtilities/bdFactory.h>

#include <bdClientServer/bdCommand/bdUserCommand.h>

/// A singleton to create user command objects as required.
class bdCommandFactoryImpl
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Create a user command.
		/// \return	A pointer to the newly created user command.
		bdUserCommand* create() const;


		/// Set the creator object that this factory should use.
		/// This is the object that will be used to create an instance of
		/// bdUserCommand whenever one is required.
		/// \param creator[in]	The creator object to use.
		void setClass(bdCreatorBase<bdUserCommand> *const creator);

	protected:

		/// Grant friendship to bdSingleton<bdCommandFactoryImpl> so that it
		/// can construct the singleton instance of this class.
		friend class bdSingleton<bdCommandFactoryImpl>;

		/// Default constructor.
		/// This is protected so that this class cannot be instantiated, except
		/// as a singleton.
		bdCommandFactoryImpl();

		/// Destructor.
		~bdCommandFactoryImpl();

	protected:

		/// A pointer to the command creator object that this class uses.
		bdCreatorBase<bdUserCommand> *m_creator;
};

/// This class provides a point of access to the singleton instance.
/// Implementation details can be found in bdCommandFactoryImpl.
class bdCommandFactory : public bdSingleton<bdCommandFactoryImpl>
{
	protected:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Protected constructor.
		bdCommandFactory();
};

#define BD_REGISTER_COMMAND_CLASS(CLASS)\
bdCommandFactory::getInstance()->setClass(new bdCreator< CLASS, bdUserCommand >);

#endif // BD_COMMAND_FACTORY_H
