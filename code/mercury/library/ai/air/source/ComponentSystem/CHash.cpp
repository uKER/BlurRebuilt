#include "stdafx.h"
#include <ComponentSystem/CHash.h>

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", off)
#endif
#endif

namespace airacingtech
{
	#define INVALID_HASH 0xffffffff
	#define HASH_INIT	0x811c9dc5
	#define HASH_PRIME	0x01000193
	#define CONVERT_BACKSLASH
	#define CASE_INSENSITIVE

	CHash::CHash() : mHashValue(INVALID_HASH)
	{
	}

	CHash::CHash(unsigned int hashValue) : mHashValue(hashValue)
	{
		// warning, this will not set the string value 
	}

	CHash::CHash(const CHash &rhs) : mHashValue(rhs.mHashValue) 
#ifndef DONT_RECORD_STRING
		, m_HashName(rhs.m_HashName)
#endif
	{

	}

	CHash::CHash(const char *pString)
	{
		mHashValue = MakeHash(pString);
	}

	CHash &CHash::operator=(const CHash &rhs)
	{
		if (&rhs != this)
		{
			mHashValue = rhs.mHashValue;
#ifndef DONT_RECORD_STRING
			m_HashName = rhs.m_HashName;
#endif
		}
		return *this;
	}

	bool CHash::IsValid() const
	{
		return (mHashValue != INVALID_HASH);
	}

	unsigned int CHash::MakeHash(const char *hashString)
	{
		if (!hashString || !hashString[0])
			return INVALID_HASH;

		const unsigned char* string = (const unsigned char*)hashString;
		unsigned int hash = HASH_INIT;


#ifndef DONT_RECORD_STRING
		m_HashName = std::string(hashString);
#endif

		while (*string)
		{
			hash *= HASH_PRIME;

			char c = *string++;

	#ifdef CONVERT_BACKSLASH
			if (c == '\\')
			{
				c = '/';
			}
	#endif

	#ifdef CASE_INSENSITIVE
			if ((c >= 'a') && (c <= 'z'))
			{
				c -= 'a' - 'A';
			}
	#endif
			hash ^= (unsigned int)c;

			//m_HashName.push_back(c);
		}


		unsigned int newHash = MakeHashTwo(hashString);

		hash = newHash;

		return hash;
	}

	void CHash::Mix(U32& a, U32& b, U32& c)
	{
		a -= b; a -= c; a ^= (c>>13);
		b -= c; b -= a; b ^= (a<<8);
		c -= a; c -= b; c ^= (b>>13);
		a -= b; a -= c; a ^= (c>>12);
		b -= c; b -= a; b ^= (a<<16);
		c -= a; c -= b; c ^= (b>>5);
		a -= b; a -= c; a ^= (c>>3);
		b -= c; b -= a; b ^= (a<<10);
		c -= a; c -= b; c ^= (b>>15);
	}

	unsigned int CHash::MakeHashTwo(const char *k)
	{
		unsigned int a,b,c,len;
		static unsigned int initval = 0;

#ifndef DONT_RECORD_STRING
		m_HashName = std::string(k);
		unsigned int length = m_HashName.size();
#else
		unsigned int length = strlen(k);
#endif

		/* Set up the internal state */
		len = length;
		a = b = 0x9e3779b9;  /* the golden ratio; an arbitrary value */
		c = initval;         /* the previous hash value */

		/*---------------------------------------- handle most of the key */
		while (len >= 12)
		{
			a += (k[0] +((unsigned int)k[1]<<8) +((unsigned int)k[2]<<16) +((unsigned int)k[3]<<24));
			b += (k[4] +((unsigned int)k[5]<<8) +((unsigned int)k[6]<<16) +((unsigned int)k[7]<<24));
			c += (k[8] +((unsigned int)k[9]<<8) +((unsigned int)k[10]<<16)+((unsigned int)k[11]<<24));
			Mix(a,b,c);
			k += 12; len -= 12;
		}

		/*------------------------------------- handle the last 11 bytes */
		c += length;
		switch(len)              /* all the case statements fall through */
		{
		case 11: c+=((unsigned int)k[10]<<24);
		case 10: c+=((unsigned int)k[9]<<16);
		case 9 : c+=((unsigned int)k[8]<<8);
			/* the first byte of c is reserved for the length */
		case 8 : b+=((unsigned int)k[7]<<24);
		case 7 : b+=((unsigned int)k[6]<<16);
		case 6 : b+=((unsigned int)k[5]<<8);
		case 5 : b+=k[4];
		case 4 : a+=((unsigned int)k[3]<<24);
		case 3 : a+=((unsigned int)k[2]<<16);
		case 2 : a+=((unsigned int)k[1]<<8);
		case 1 : a+=k[0];
			/* case 0: nothing left to add */
		}

		Mix(a,b,c);
		/*-------------------------------------------- report the result */
		
		// As we want the same hash key for certain things, we only need initval to be 0;
		//initval = c;

		return c;
	}

	void CHash::Set(const unsigned int hash)
	{
		mHashValue = hash;
	}

	const bool CHash::operator<(const CHash &rhs) const
	{
		return mHashValue < rhs.mHashValue;
	}

	const bool CHash::operator>(const CHash &rhs) const
	{
		return mHashValue > rhs.mHashValue;
	}

	const bool CHash::operator<=(const CHash &rhs) const
	{
		return mHashValue <= rhs.mHashValue;
	}

	const bool CHash::operator>=(const CHash &rhs) const
	{
		return mHashValue >= rhs.mHashValue;
	}

	const bool CHash::operator==(const CHash &rhs) const
	{
		return mHashValue == rhs.mHashValue;
	}

	const bool CHash::operator!=(const CHash &rhs) const
	{
		return (!(*this == rhs));
	}
}

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif


