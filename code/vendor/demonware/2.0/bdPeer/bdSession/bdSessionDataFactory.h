// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURROSE: A singleton to create instances of a class derived from 
//          bdSessionData.

#ifndef BD_SESSION_DATA_FACTORY_H
#define BD_SESSION_DATA_FACTORY_H

#include <bdCore/bdCore.h>
#include <bdCore/bdUtilities/bdFactory.h>

#include <bdPeer/bdSession/bdSessionData.h>

/// A factory class to create instances of a class derived from bdPlayer.
class bdSessionDataFactoryImpl
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Create an instance of a class derived from bdPlayer.
		/// Uses the registered bdCreatorBase to create an instance of
		/// the registered derived class.
		/// \return A pointer to the newly created bdPlayer derived object.
		bdSessionData* create() const;

		/// Set the creator class that this factory uses.
		/// The bdCreatorBase<bdPlayer> class is used to create instances of a
		/// class derived from bdPlayer.
		/// \param creator[in] A pointer to the creator to use.
		///		bdCommandFactoryImpl will assume ownership of \a creator
		///		so it should be heap allocated and not deleted.
		void setClass(bdCreatorBase<bdSessionData> *const creator);

	protected:

		/// Grant friendship to bdSingleton<bdPlayerFactoryImpl> so that it can
		/// construct the singleton instance of this class.
		friend class bdSingleton<bdSessionDataFactoryImpl>;

		/// Default constructor.
		bdSessionDataFactoryImpl();

		/// Destructor.
		~bdSessionDataFactoryImpl();

	protected:

		/// A pointer to the creation class.
		bdCreatorBase<bdSessionData> *m_creator;
};

/// This class provides a point of access to the singleton instance
/// of bdPlayerFactoryImpl. Implementation details can be found in
/// bdPlayerFactoryImpl.
class bdSessionDataFactory : public bdSingleton<bdSessionDataFactoryImpl>
{
	protected:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor
		bdSessionDataFactory();
};

/// Macro to register a class derived from bdSessionData with the bdSessionDataFactory.
#define BD_REGISTER_SESSION_DATA_CLASS(CLASS)\
	bdSessionDataFactory::getInstance()->setClass(new bdCreator< CLASS, bdSessionData >);

#endif // BD_SESSION_DATA_FACTORY_H
