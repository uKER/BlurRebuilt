// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURROSE: A singleton to create instances of a class derived from bdPlayer.

#ifndef BD_PLAYER_FACTORY_H
#define BD_PLAYER_FACTORY_H

#include <bdCore/bdCore.h>
#include <bdCore/bdUtilities/bdFactory.h>

#include <bdPeer/bdPlayer/bdPlayer.h>

/// A factory class to create instances of a class derived from bdPlayer.
class bdPlayerFactoryImpl
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Create an instance of a class derived from bdPlayer.
		/// Uses the registered bdCreatorBase to create an instance of
		/// the registered derived class.
		/// \return A pointer to the newly created bdPlayer derived object.
		bdPlayer* create() const;

		/// Set the creator class that this factory uses.
		/// The bdCreatorBase<bdPlayer> class is used to create instances of a
		/// class derived from bdPlayer.
		/// \param creator[in] A pointer to the creator to use.
		///		bdCommandFactoryImpl will assume ownership of \a creator
		///		so it should be heap allocated and not deleted.
		void setClass(bdCreatorBase<bdPlayer> *const creator);

	protected:

		/// Grant friendship to bdSingleton<bdPlayerFactoryImpl> so that it can
		/// construct the singleton instance of this class.
		friend class bdSingleton<bdPlayerFactoryImpl>;

		/// Default constructor.
		bdPlayerFactoryImpl();

		/// Destructor.
		~bdPlayerFactoryImpl();

	protected:

		/// A pointer to the creation class.
		bdCreatorBase<bdPlayer> *m_creator;
};

/// This class provides a point of access to the singleton instance
/// of bdPlayerFactoryImpl. Implementation details can be found in
/// bdPlayerFactoryImpl.
class bdPlayerFactory : public bdSingleton<bdPlayerFactoryImpl>
{
	protected:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

			/// Default constructor
			bdPlayerFactory();
};

/// Macro to register a class derived from bdPlayer with the bdPlayerFactory.
#define BD_REGISTER_PLAYER_CLASS(CLASS)\
	bdPlayerFactory::getInstance()->setClass(new bdCreator< CLASS, bdPlayer >);

#endif // BD_PLAYER_FACTORY_H
