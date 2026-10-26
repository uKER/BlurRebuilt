// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Factory class for creating classes drived from bdReplica.

#ifndef BD_REPLICA_FACTORY_H
#define BD_REPLICA_FACTORY_H

#include <bdCore/bdCore.h>
#include <bdCore/bdUtilities/bdFactory.h>

#include <bdClientServer/bdReplica/bdReplica.h>

/// Factory class for creating classes drived from bdReplica.
class bdReplicaFactoryImpl : public bdFactory<bdReplica>
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		// Comment in base class.
		virtual bdBool registerClass(const bdUInt32 type,
								   bdCreatorBase<bdReplica> *creator);

		/// Get the number of creators that are registered with the factory.
		/// This is equivalent to the max type.
		bdUInt32 getMaxType() const;

	protected:

		/// Grant friendship to bdSingleton<bdReplicaFactoryImpl> so that it
		/// can construct the singleton instance of this class.
		friend class bdSingleton< bdReplicaFactoryImpl >;

		/// Default constructor.
		bdReplicaFactoryImpl();

		/// Virtual destructor.
		virtual ~bdReplicaFactoryImpl();

	protected:

		/// A counter for the number of creators for individual types that have
		/// been registered.
		bdUInt32 m_maxType;
};

/// This class provides a point of access to the singleton instance.
/// Implementation details can be found in bdReplicaFactoryImpl.
class bdReplicaFactory : public bdSingleton<bdReplicaFactoryImpl>
{
	protected:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdReplicaFactory();
};

/// This macro can be used to conveniently register new subclasses of bdReplica
/// with the factory.
/// \param TYPE_ID The type identifier for the class.
/// \param CLASS The actual class of the new type.
#define BD_REGISTER_CLASS(TYPE_ID, CLASS)\
if(!bdReplicaFactory::getInstance()->containsClass(TYPE_ID)) \
{ \
	bdCreator< CLASS, bdReplica > *const creator = new  bdCreator< CLASS, bdReplica >; \
	if(!bdReplicaFactory::getInstance()->registerClass(TYPE_ID, creator)) \
	{ \
		delete creator; \
	} \
}

#endif // BD_REPLICA_FACTORY_H
