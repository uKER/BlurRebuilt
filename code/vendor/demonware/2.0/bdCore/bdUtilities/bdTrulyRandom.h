// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A class that uses operating system specific libraries to generate 
//			truly random numbers.

#ifndef BD_TRULYRANDOM_H
#define BD_TRULYRANDOM_H

#include <bdCore/bdMemory/bdMemory.h>
#include <bdCore/bdContainers/bdSingleton.h>

#define BD_MAX_TRULY_RANDOM (BD_UINT32_MAX)

/// A class that uses operating system specific libraries to generate truly 
/// random numbers.
/// \note	The guarantee of true randomness depends on the operating system's
///			implementation. See bdPlatformTrulyRandom
class bdTrulyRandomImpl
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS
		
		/// Get a random unsigned integer.
		/// \return	The next random unsigned integer.
		bdUInt getRandomUInt() const;

		/// Fill an array of bytes of a specified size with random bytes.
		/// \param	in[in,out]	The byte array to fill in.
		/// \param	length[in]	The size of the array.
		void getRandomUByte8(bdUByte8 in[], const bdUInt length) const;
				
	protected:

		/// Grant friendship to bdSingleton<bdTrulyRandomImpl> so that it can
		/// construct the singleton instance of this class.
		friend class bdSingleton<bdTrulyRandomImpl>;

		/// Default constructor. 
		/// This is protected so that this class cannot be instantiated, except
		/// as a singleton.
		bdTrulyRandomImpl();
		
};

/// This class provides a point of access to the singleton instance.
/// Implementation details can be found in bdTrulyRandomImpl.
class bdTrulyRandom : public bdSingleton<bdTrulyRandomImpl>
{
	BD_DECLARE_NEW_AND_DELETE_OPERATORS

	protected:
		bdTrulyRandom();
};

#endif // BD_TRULYRANDOM_H
