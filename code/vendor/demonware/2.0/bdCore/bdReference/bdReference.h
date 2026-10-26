// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A reference-counted smart pointer class.

#ifndef BD_REFERENCE_H
#define BD_REFERENCE_H

#include <bdCore/bdMemory/bdMemory.h>

/// A reference-counted smart pointer class.
/// This class can be used in conjunction with any class that inherits from
/// bdReferencable. The overloaded operators in this class maintain a reference
/// count for the class that is being referenced. Each time an instance of
/// \c bdReference is copied, the count is incremented, and each time one goes
/// out of scope, the reference count is decremented. When the count reaches
/// zero the instance is safely and automatically deleted.
template <typename T>
class bdReference
{
    public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor.
		inline bdReference();

		/// Constructor that takes a pointer to an object.
		/// \param	p[in]	A pointer to the object to reference.
		/// \note	This constructor is not intended to be called explicitly.
		inline bdReference(T* p);

		/// Copy constructor.
		/// \param	other[in]	The reference to copy.
		inline bdReference(const bdReference<T>& other);

		/// A constructor that \c static_casts another reference to type \c T.
		/// \param	other[in]	The reference to copy.
		// Sorry about the mess here, but MSVC6 requires that member templates
		// be defined in the class declaration.
		// <http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclang/html/_core_members_of_class_templates.asp>
		// gcc and MSVC7 have no problems if the definition is outside the
		// class declaration.
		template <class U>
		inline bdReference(bdReference<U>& other)
		{
			m_ptr = static_cast<T*>(other.operator->());
			if (m_ptr != BD_NULL)
			{
				m_ptr->addRef();
			}
		}

		/// Destructor.
		inline ~bdReference();

		/// Assignment operator.
		/// \param	p[in]	A pointer to the object to assign.
		inline void operator =(T* p);

		/// Assignment operator.
		/// \param	other[in]	The reference to the object to assign.
#ifdef BD_COMPILER_MSVC
		inline const bdReference<T>& operator =(const bdReference<T>& other);
#else
		inline const bdReference<T>& operator =(const bdReference<T>& other)
		{
			if (&other != this)
			{
				if (m_ptr != BD_NULL)
				{
					if (m_ptr->releaseRef() == 0)
					{
						delete m_ptr;
					}
				}
				m_ptr = other.m_ptr;
				if (m_ptr != BD_NULL)
				{
					m_ptr->addRef();
				}
			}
			return *this;
		}
#endif


		/// An assignment operator that \c reinterpret_casts another reference
		/// to type T.
		/// \param	other[in]	The reference to the object to assign.
		/// \return				The result of the assignment, i.e., this
		///						reference.
		// Sorry about the mess here, but MSVC6 requires that member templates
		// be defined in the class declaration.
		// <http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclang/html/_core_members_of_class_templates.asp>
		// gcc and MSVC7 have no problems if the definition is outside the
		// class declaration.
		template <class U>
#ifdef BD_COMPILER_MSVC6
		inline const bdReference<T>& operator =(bdReference<U>& other)
#else
		inline const bdReference<T>& operator =(const bdReference<U>& other)
#endif
		{
			//if (&other != this)
			if (reinterpret_cast<void*>(this) != reinterpret_cast<void*>(const_cast<bdReference<U>*>(&other)))
			{
				if (m_ptr != BD_NULL)
				{
					if (m_ptr->releaseRef() == 0)
					{
						delete m_ptr;
					}
				}
				// assign the embedded m_ptr of the other reference to this.
				// If you are looking at this it is most likely that you tried
				// to assign two unrelated reference classes.
				const T* temp = other.operator->();
				m_ptr = const_cast<T*>(temp);
				if (m_ptr != BD_NULL)
				{
					m_ptr->addRef();
				}
			}

			return *this;
		}

		/// Equality operator to compare this reference to \a other.
		/// \param	other[in]	The reference to compare \a this to.
		/// \return
		/// -	True, if both references point to the same object.
		/// -	False, if the references point to different objects.
		// Sorry about the mess here, but MSVC6 requires that member templates
		// be defined in the class declaration.
		// <http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclang/html/_core_members_of_class_templates.asp>
		// gcc and MSVC7 have no problems if the definition is outside the
		// class declaration.
		template <class U>
		inline bdBool operator ==(const bdReference<U>& other) const
		{
			return (m_ptr == static_cast<const T*>(other.operator->()));
		}

		/// Inequality operator to compare this  reference to \a other.
		/// \param	other[in]	The reference to compare \a this to.
		/// \return
		/// -	True, if the references point to different objects.
		/// -	False, if both references point to the same object.
		// Sorry about the mess here, but MSVC6 requires that member templates
		// be defined in the class declaration.
		// <http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclang/html/_core_members_of_class_templates.asp>
		// gcc and MSVC7 have no problems if the definition is outside the
		// class declaration.
		template <class U>
		inline bdBool operator !=(const bdReference<U>& other) const
		{
			return (m_ptr != static_cast<const T*>(other.operator->()));
		}

		/// Cast operator to convert to a boolean.
		/// This is convenient when checking if the pointer has been
		/// initialized.
        /// \return
		/// -	True, if the reference is valid.
		/// -	False, if the reference is not valid and the pointer is
		///		\c BD_NULL.
		//inline operator bdBool() const;

		/// Not operator to determine if the object pointer is \c BD_NULL.
		/// This is convenient when checking if the pointer has been
		/// initialized.
		/// \return
		/// -	True, if the reference is not valid and the pointer is
		///		\c BD_NULL.
		/// -	False, if the reference is valid.
		inline bdBool operator !() const;

		/// Determine if the object pointer is \c BD_NULL.
		/// This is convenient when checking if the pointer has been
		/// initialized.
		/// \return
		/// -	True, if the reference is not valid and the pointer is
		///		\c BD_NULL.
		/// -	False, if the reference is valid.
		inline bdBool isNull() const;

		/// Determine if the object pointer is not \c BD_NULL.
		/// This is convenient when checking if the pointer has been
		/// initialized.
		/// \return
		/// -	True,  if the reference is valid.
		/// -	False, if the reference is not valid and the pointer is
		///		\c BD_NULL.
		inline bdBool notNull() const;

		/// Cast operator to convert to a pointer to a \c T.
		/// \return		A pointer to the object referenced.
		inline operator T*();

		/// Arrow operator.
		/// \return	A pointer to the object referenced by \a this.
		inline T* operator ->();

		/// Arrow operator.
		/// \return	A const pointer to the object referenced by \a this.
		inline T* operator ->() const;

		/// De-reference operator.
		/// \return	The object referenced by \a this.
		inline T& operator *() ;

		/// De-reference operator.
		/// \return	The const object referenced by \a const this.
		inline const T& operator *() const;

		/// Less-than operator.
		/// Compares the object referenced by \a this to \a other, based on
		/// their locations in memory.
		/// \param	other[in]	The other object to compare.
		/// \return
		/// -	True, if the object referenced by \a this is at a lower memory
		///		location than \a other.
		/// -	False, if the object referenced by \a this is at a higher or
		///		equal location in memory.
		inline bdBool operator<(const bdReference<T>& other) const;

		/// Greater-than operator.
		/// Compares the object referenced by \a this to \a other, based on
		/// their locations in memory.
		/// \param	other[in]	The other object to compare.
		/// \return
		/// -	True, if the object referenced by \a this is at a higher memory
		///		location than \a other.
		/// -	False, if the object referenced by \a this is at a lower or
		///		equal location in memory.
		inline bdBool operator>(const bdReference<T>& other) const;

	protected:

		/// The pointer to the object referenced by \a this.
		T* m_ptr;
};

/// Cast a bdReference to base class to a derived class.
/// This templated function is for use  where \c static_cast would be used with
/// normal pointers.
/// \note	Usage: <code>DerivedClassRef ref1 = reference_cast<BaseClass>(ref2);
///			</code>
/// \note	The template parameter used is the name of the base class, not a
///			pointer to the base class.
/// \note	This function uses \c static_cast. The compiler will catch type
///			mismatches that can be statically determined, but will miss others.
///			Always be sure that the object to be cast matches the type it will
///			be cast to.
/// \param	referenceClass[in]	A bdReference to cast into a reference pointing
///								to a derived type.
///	\return						A pointer to the referenced object as a derived
///								type.
template<typename T, typename U>
T *reference_cast(bdReference<U> referenceClass)
{
	return (static_cast<T*>(referenceClass.operator ->()));

}

#define BD_REFERENCE(CLASS)	class CLASS; typedef bdReference<CLASS> CLASS##Ref

#include <bdCore/bdReference/bdReference.inl>

#endif // BD_REFERENCE_H


