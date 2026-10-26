#ifndef NETVECTOR_H
#define NETVECTOR_H

#ifdef _MSC_VER
#pragma once
#endif

#include "netStream/netBuffer.h"

BD_REFERENCE(netVector);
class netVector: public netBuffer
{
public:
	netVector(int length=8);
	~netVector();

	bool realloc(int size);
	bool writeBytes_internal(const void* ptr, int len);
	bool deserialise(netStream* stream);
};

#endif //NETVECTOR_H
