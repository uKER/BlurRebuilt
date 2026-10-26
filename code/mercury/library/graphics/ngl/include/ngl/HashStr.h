#ifndef HASHSTR_H
#define HASHSTR_H

#include <ctype.h>

#include "ngl/defs.h"
#include "ngl/system.h"

namespace ActTools
{

class HashStr
{
public:

	typedef u32 hsint;

	hsint *value()
	{
		return reinterpret_cast<hsint *>(&theData->Hash);
	}
	const hsint *value() const
	{
		return reinterpret_cast<const hsint *>(&theData->Hash);
	}

	HashStr() : 
		theNumElements(0),
		theData(0)
	{
	}

	HashStr( const HashStr & s ) :
		theNumElements(s.theNumElements)
	{
		theData = reinterpret_cast<Data*>(new u8[theNumElements * sizeof(hsint)]);
		for ( size_t i = theNumElements; i--; )
			value()[i] = s.value()[i];
	}
	~HashStr()
	{
		delete (u8*)theData;
	}

	HashStr( const char *aString ) :
		theData(0)
	{
		Set( aString );
	}
	void Init()
	{
		theNumElements = 0;
		theData = 0;
	}
	void Destroy()
	{
		delete (u8*)theData;
		theData = 0;
	}
	void Acquire( HashStr* aHashStr )
	{
		theData = aHashStr->theData; aHashStr->theData = 0;
		theNumElements = aHashStr->theNumElements; aHashStr->theNumElements = 0;
	}
	void Set( const char* aString )
	{
		if ( aString == theData->String )
			return;
		const char *eos = aString;
		while( *eos++ );
		size_t len = static_cast<size_t>(eos - aString - 1);
		theNumElements = ( len + 2*sizeof(hsint) ) / sizeof(hsint);
		theData = reinterpret_cast<Data*>(new u8[ theData, theNumElements * sizeof(hsint) ]);

		value()[theNumElements-1] = 0;

		theData->Hash = 0;
		char* d = theData->String;
		for ( const char *c = aString; *c; ++c )
		{
			char lc = isalpha(*c) ? (char)tolower(*c) : *c;
			theData->Hash += (theData->Hash << 5) + u32(lc);
			if ( size_t(d-theData->String) < len+1-1 )
				*d++ = *c;
		}
//		theData->String[len-1] = 0;
	}

	const HashStr &operator=( const HashStr & rhs )
	{
		theNumElements = rhs.theNumElements;
		theData = reinterpret_cast<Data*>(new u8[ theNumElements * sizeof(hsint) ]);
		for ( size_t i = 0; i < theNumElements; ++i )
			value()[i] = rhs.value()[i];

		return *this;
	}

	const char &operator[]( int i ) const
	{
		return theData->String[i];
	}

	char &operator[](int i)
	{
		return theData->String[i];
	}

	bool operator==( const HashStr & rhs ) const
	{
		return theData->Hash == rhs.theData->Hash;
	}

	bool operator!=( const HashStr & rhs ) const
	{
		return !(*this == rhs);
	}

	const char* c_str() const 
	{
		return theData->String; 
	}

	bool operator <( const HashStr & rhs ) const
	{
		return Compare( rhs ) < 0;
	}

	bool operator >= ( const HashStr & rhs ) const
	{
		return Compare( rhs ) >= 0;
	}

	int Compare( const HashStr & rhs ) const
	{
		if (theData->Hash > rhs.theData->Hash)
			return 1;
		else if (theData->Hash == rhs.theData->Hash)
			return 0;
		else
			return -1;
	}

	int Order( const HashStr & rhs ) const
	{
		return Compare(rhs);
	}

	bool IsEmpy() const
	{
		return theNumElements == 0 || (*this)[0] == '\0';
	}

	template<typename Archive>
	void Serialise(Archive &ar)
	{
		if (!theData)
		{
			ar << 0;
			return;
		}

		const char *pStr = c_str();
		int len = strlen(pStr);
		ar << len;
		ar.Write(c_str(), len);
	}

	template<typename Archive>
	void Deserialise(Archive &ar)
	{
		int iLen;
		ar >> iLen;
		if (iLen == 0)
		{
			Destroy();
			theNumElements = 0;
		}
		else
		{
			char *pBuffer = new char[iLen+1];
			ar.Read(pBuffer, iLen);
			pBuffer[iLen] = 0;
			Set(pBuffer);
			delete pBuffer;
		}
	}

private:
	struct Data
	{
		u32  Hash;
		char String[4];
	} *theData;
	size_t theNumElements;
};

}

#endif // HASHSTR_H

