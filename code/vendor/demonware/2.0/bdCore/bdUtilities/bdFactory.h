// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A factory class used to create an object derived from a base class.

#ifndef BD_FACTORY_H
#define BD_FACTORY_H

#include <bdCore/bdMemory/bdMemory.h>
#include <bdCore/bdContainers/bdHashMap.h>

/// Abstract base class for object creators.
template<typename BASE>
class bdCreatorBase
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Virtual destructor.
		virtual ~bdCreatorBase();

		/// Create an object of type \a BASE.
		virtual BASE* create() const;

		/// Get the size of the object that this creator creates.
		virtual bdUInt getSizeOf() const = 0;
};

/// Template class for creator objects.
/// \param DERIVED Specifies the actual class this creator creates
/// \param BASE Specifies the base class of the derived class this creator creates
template<typename DERIVED, typename BASE>
class bdCreator : public bdCreatorBase<BASE>
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Virtual destructor.
		virtual ~bdCreator();

		/// Creates an object of the DERIVED class.
		/// \return A pointer to a newly created instance of DERIVED.
		virtual BASE* create() const;	

		/// Get the size of DERIVED, created by the create function.
		/// \return sizeof(DERIVED)
		virtual bdUInt getSizeOf() const;
};

/// A factory class that creates objects with a common base class.
/// An application must call registerClass() for each object type
/// that needs to be created through the factory.
template<typename BASE>
class bdFactory
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Virtual destructor.
		/// Deletes all registered creators.
		virtual ~bdFactory();

		/// Creates an object of a particular type.
		/// \param type[in]	The type of the object to be created
		/// \return A pointer to the object created.
		BASE* create(const bdUInt type) const;

		/// Registers a creator object with the factory class.
		/// From this point on, the bdFactory assumes ownership of the
		/// \a creator, and will delete it when the factory is destroyed.
		/// \param type[in]		An integer corresponding to a BASE derived class.
		/// \param creator[in]	A pointer to the creator object to be
		///						registered.
		/// \return
		///	- true, if the type creator pair is successfully registered.
		/// - false otherwise.
		bdBool registerClass(const bdUInt type,
							 bdCreatorBase<BASE> *creator);

		/// Checks to see if a particular type is already registered with the
		/// factory
		/// \param type[in]		An integer corresponding to a BASE derived class.
		/// \return
		///	- true, if the type is already registered.
		/// - false otherwise.
		bdBool containsClass(const bdUInt type);

	protected:

		/// Map of creator object to object types.
		bdHashMap<bdUInt, bdCreatorBase<BASE> *> m_creators;
};


/// Abstract base class for typed object creators.
template<typename BASE>
class bdTypedCreatorBase
{
public:

	BD_DECLARE_NEW_AND_DELETE_OPERATORS

	/// Virtual destructor.
	virtual ~bdTypedCreatorBase();

	/// Creates an object of the DERIVED class.
	/// \return A pointer to a newly created instance of DERIVED.
	virtual BASE* create(const bdUInt type) const;	

	/// Get the size of the object that this creator creates.
	virtual bdUInt getSizeOf() const = 0;
};

/// Template class for typed creator objects.
/// \param DERIVED Specifies the actual class this creator creates
/// \param BASE Specifies the base class of the derived class this creator creates
template<typename DERIVED, typename BASE>
class bdTypedCreator : public bdTypedCreatorBase<BASE>
{
public:

	BD_DECLARE_NEW_AND_DELETE_OPERATORS

	/// Virtual destructor.
	virtual ~bdTypedCreator();

	/// Creates an object of the DERIVED class.
	/// \return A pointer to a newly created instance of DERIVED.
	virtual BASE* create(const bdUInt type) const;	

	/// Get the size of DERIVED, created by the create function.
	/// \return sizeof(DERIVED)
	virtual bdUInt getSizeOf() const;
};

/// A factory class that creates objects based on a type.
/// An application must call registerClass() for each object type
/// that needs to be created through the factory.
template<typename BASE>
class bdTypedFactory
{
public:

	BD_DECLARE_NEW_AND_DELETE_OPERATORS

	/// Virtual destructor.
	/// Deletes all registered creators.
	virtual ~bdTypedFactory();

	/// Creates an object of a particular type.
	/// \param type[in]	The type of the object to be created
	/// \return A pointer to the object created.
	BASE* create(const bdUInt type) const;

	/// Registers a creator object with the factory class.
	/// From this point on, the bdFactory assumes ownership of the
	/// \a creator, and will delete it when the factory is destroyed.
	/// \param type[in]		An integer corresponding to a BASE derived class.
	/// \param creator[in]	A pointer to the creator object to be
	///						registered.
	/// \return
	///	- true, if the type creator pair is successfully registered.
	/// - false otherwise.
	bdBool registerClass(const bdUInt type,
						 bdTypedCreatorBase<BASE> *creator);

	/// Checks to see if a particular type is already registered with the
	/// factory
	/// \param type[in]		An integer corresponding to a BASE derived class.
	/// \return
	///	- true, if the type is already registered.
	/// - false otherwise.
	bdBool containsClass(const bdUInt type);

protected:

	/// Map of creator object to object types.
	bdHashMap<bdUInt, bdTypedCreatorBase<BASE> *> m_creators;
};

#include <bdCore/bdUtilities/bdFactory.inl>

#endif // BD_FACTORY_H



