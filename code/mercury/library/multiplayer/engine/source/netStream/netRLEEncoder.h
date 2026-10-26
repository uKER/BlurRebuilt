#ifndef NETRLEENCODER_H
#define NETRLEENCODER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "netStream/netSerialise.h"
#include "netStream/netArray.h"

class BitfieldRLE: public netSerialisable, public netArray<2000>
{
public:

	enum
	{
		HEADER_RLE_ENCODED   = (1 << 7),
		HEADER_FIRST_BIT_SET = (1 << 6),
		HEADER_SHORT_LENGTH	 = (1 << 5), 
		HEADER_SHORT_LENGTH_MAX = (1 << 5)-1, 

	};

	BitfieldRLE(char* bitfield, unsigned int bitCount);

	unsigned int byteCount() const;

	bool serialise(class netStream* stream);
	bool deserialise(class netStream* stream);

	bool getBit(int i);
	void setBit(int i, bool onOff);

	bool encode();
	bool encodeRAW();
	bool encodeRLE();

	bool decode();
	bool decodeRAW();
	bool decodeRLE();

	void debugOutput();

	char			m_header;
	char*			m_bitfield;
	unsigned int	m_bitCount;
};

//#include "netRLEEncoder.inl"

#endif //NETRLEENCODER_H