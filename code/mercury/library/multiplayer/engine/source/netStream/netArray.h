#ifndef NETARRAY_H
#define NETARRAY_H

#ifdef _MSC_VER
#pragma once
#endif

#include "netStream/netBuffer.h"

template<int SIZE>
class netArray: public netBuffer
{
public:
	netArray() 
	: netBuffer(m_buf, sizeof(m_buf)) 
	{}

	char m_buf[SIZE];
};

#endif //NETARRAY_H
