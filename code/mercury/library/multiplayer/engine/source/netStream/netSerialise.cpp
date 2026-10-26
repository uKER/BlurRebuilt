#include "stdafx.h"
#include "netStream\netSerialise.h"
#include "netStream\netStreamSizeCalculator.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

netSerialisable::~netSerialisable()
{
}

bool netSerialisable::serialise(class netStream* stream)
{
	return true;
}

bool netSerialisable::deserialise(class netStream* stream)
{
	return true;
}

int netSerialisable::serialiseSize()
{
	netStreamSizeCalculator_NEW calc;
	
	if(!serialise(&calc))
		return 0;

	return calc.getWriteByteCount();
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

