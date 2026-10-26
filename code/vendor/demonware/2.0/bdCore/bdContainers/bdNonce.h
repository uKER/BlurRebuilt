// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A template nonce object.

#ifndef BD_NONCE_H
#define BD_NONCE_H

/// A nonce of \c size bytes.
/// This class provides nonce values and is suitable to be used as static variables.
/// Upon initial creation, it is set to a default value of zero. To ensure the nonce
/// is properly intitialized with a random value, \c ensureCreated() should be called.
template<bdUInt size>
class bdNonce
{
	public:

			BD_DECLARE_NEW_AND_DELETE_OPERATORS

			/// Default constructor. The nonce is initialized to zero.
			inline bdNonce();

			/// Default destructor.
			inline ~bdNonce();

			/// Make sure the nonce is initialized to a random value. It is safe to call
			/// this method multiple times: only the first call has effect.
			inline void ensureCreated();

			/// Get random data contained in this nonce. \c ensureCreated() must be called
			/// at least once before calling this method.
			/// \return A pointer to an array of random bytes of size \c size.
			inline const bdUByte8* getData();

	protected:
		/// The nonce itself.
		bdUByte8 m_nonce[size];
		/// True, if the nonce has been filled with random data.
		bdBool m_initialised;
};

#include <bdCore/bdContainers/bdNonce.inl>

#endif // BD_NONCE_H
