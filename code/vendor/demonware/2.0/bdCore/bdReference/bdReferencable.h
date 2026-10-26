// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Defines a common base class that all reference counted classes must
//			inherit from.

#ifndef BD_REFERENCABLE_H
#define BD_REFERENCABLE_H

#include <bdCore/bdMemory/bdMemory.h>
#ifdef BD_PLATFORM_PS3
#include <cell/atomic.h>
#endif

/// This class provides functions and data members that objects must inherit
/// in order to be reference counted.
/// It is only used by bdReference, and by reference counted classes.
class bdReferencable
{
    public:

        BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor.
		inline bdReferencable();

		/// Virtual destructor.
		virtual ~bdReferencable();

		/// Increment the reference count for the object.
		/// \return	The new reference count.
        inline bdInt addRef();

		/// Decrement the reference count for the object.
		/// \return	The new reference count.
        inline bdInt releaseRef();

		/// Get the reference count of the object.
		/// \return	The current reference count.
		inline bdInt getRefCount() const;

	protected:

		/// The current reference count of the object.
		bdInt m_refCount;

};

#include <bdCore/bdReference/bdReferencable.inl>

#endif // BD_REFERENCABLE_H


