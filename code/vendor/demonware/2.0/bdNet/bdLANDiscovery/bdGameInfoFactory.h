// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURROSE: A singleton to create instances of a class derived from bdGameInfo.

#ifndef BD_GAME_INFO_FACTORY_H
#define BD_GAME_INFO_FACTORY_H

#include <bdCore/bdCore.h>
#include <bdCore/bdUtilities/bdFactory.h>

#include <bdNet/bdLANDiscovery/bdGameInfo.h>

/// A factory class to create instances of a class derived from bdGameInfo.
class bdGameInfoFactoryImpl
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Create a instance of a class derived from bdGameInfo.
		/// Uses the registered bdCreatorBase to create an instance of
		/// the registered derived class.
		/// \return A pointer to the newly created bdGameInfo derived object.
		bdGameInfo* create() const;

		/// Set the creator class that this factory uses.
		/// The bdCreatorBase<bdGameInfo> class is used to create instances of a
		/// class derived from bdGameInfo.
		/// \param creator[in] A pointer to the creator to use.
		///		bdGameInfoFactoryImpl will assume ownership of \a creator
		///		so it should be heap allocated and not deleted.
		void setClass(bdCreatorBase<bdGameInfo> *const creator);

	protected:

		/// Grant friendship to bdSingleton<bdGameInfoFactoryImpl> so that it
		/// can construct the singleton instance of this class.
		friend class bdSingleton<bdGameInfoFactoryImpl>;

		/// Default constructor.
		/// This is protected so that this class cannot be instantiated, except
		/// as a singleton.
		bdGameInfoFactoryImpl();

		/// Destructor.
		~bdGameInfoFactoryImpl();

	protected:

		/// A pointer to the creation class.
		bdCreatorBase<bdGameInfo> *m_creator;
};

/// This class provides a point of access to the singleton instance
/// of bdGameInfoFactoryImpl. Implementation details can be found in
/// bdGameInfoFactoryImpl.
class bdGameInfoFactory : public bdSingleton<bdGameInfoFactoryImpl>
{
	protected:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Protected constructor.
		bdGameInfoFactory();
};

/// Macro to register a class derived from bdGameInfo with the bdGameInfoFactory.
#define BD_REGISTER_GAME_INFO_CLASS(CLASS)\
bdGameInfoFactory::getInstance()->setClass(new bdCreator< CLASS, bdGameInfo >);

#endif // BD_GAME_INFO_FACTORY_H
