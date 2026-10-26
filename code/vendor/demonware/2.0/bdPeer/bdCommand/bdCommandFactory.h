// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A singleton to create instances of a class derived from bdCommand.

#ifndef BD_COMMAND_FACTORY_H
#define BD_COMMAND_FACTORY_H

#include <bdCore/bdCore.h>
#include <bdCore/bdUtilities/bdFactory.h>

#include <bdPeer/bdCommand/bdCommand.h>

/// A factory class to create instances of a class derived from bdCommand.
class bdCommandFactoryImpl
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Create a instance of a class derived from bdCommand.
		/// Uses the registered bdCreatorBase to create an instance of
		/// the registered derived class.
		/// \return A pointer to the newly created bdCommand derived object.
		bdCommand* create() const;

		/// Set the creator class that this factory uses.
		/// The bdCreatorBase<bdCommand> class is used to create instances of a 
		/// class derived from bdCommand.
		/// \param creator[in] A pointer to the creator to use. 
		///		bdCommandFactoryImpl will assume ownership of \a creator
		///		so it should be heap allocated and not deleted.
		void setClass(bdCreatorBase<bdCommand> *const creator);

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
	
		/// A pointer to the creation class.
		bdCreatorBase<bdCommand> *m_creator;
};

/// This class provides a point of access to the singleton instance
/// of bdCommandFactoryImpl. Implementation details can be found in 
/// bdCommandFactoryImpl.
class bdCommandFactory : public bdSingleton<bdCommandFactoryImpl>
{
	protected:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Protected constructor.
		bdCommandFactory();
};

/// Macro to register a class derived from bdCommand with the bdCommandFactory.
#define BD_REGISTER_COMMAND_CLASS(CLASS)\
bdCommandFactory::getInstance()->setClass(new bdCreator< CLASS, bdCommand >);

#endif // BD_COMMAND_FACTORY_H
