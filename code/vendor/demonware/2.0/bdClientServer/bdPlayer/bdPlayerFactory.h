// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURROSE: A singleton to create instances of the bdPlayer class.

#ifndef BD_PLAYER_FACTORY_H
#define BD_PLAYER_FACTORY_H

#include <bdCore/bdCore.h>
#include <bdCore/bdUtilities/bdFactory.h>

#include <bdClientServer/bdPlayer/bdPlayer.h>

/// A singleton to create instances of the player class.
class bdPlayerFactoryImpl
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Create a bdPlayer object.
		/// \return A pointer to the newly created bdPlayer object.
		bdPlayer* create() const;

		/// Find out how big the objects from this factory are.
		/// \return The size of the objects that this factory creates.
		bdUInt32 getSizeOf() const;
		
		/// Set the creator class that this factory uses.
		/// \param creator[in] The creator to use.
		void setClass(bdCreatorBase<bdPlayer> *const creator);

	protected:

		/// Default constructor.
		bdPlayerFactoryImpl();

		/// Destructor.
		~bdPlayerFactoryImpl();

		/// Grant friendship to bdSingleton<bdPlayerFactoryImpl> so that it can
		/// construct the singleton instance of this class.
		friend class bdSingleton<bdPlayerFactoryImpl>;

	protected:
	
		/// A pointer to the creator that this class uses.
		bdCreatorBase<bdPlayer> *m_creator;
};

/// This class provides a point of access to the singleton instance.
/// Implementation details can be found in bdPlayerFactoryImpl.
class bdPlayerFactory : public bdSingleton<bdPlayerFactoryImpl>
{
	protected:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor
		bdPlayerFactory();
};

#define BD_REGISTER_PLAYER_CLASS(CLASS)\
bdPlayerFactory::getInstance()->setClass(new bdCreator< CLASS, bdPlayer >);

#endif // BD_PLAYER_FACTORY_H
