#ifndef NETFILESTREAM_H
#define NETFILESTREAM_H

#ifdef _MSC_VER
#pragma once
#endif


#include "netStream/netStream.h"

namespace Sys { class I_File; };

BD_REFERENCE(netFileStream);
class netFileStream: public netStream
{
public:
	enum
	{
		ACCESS_READ,
		ACCESS_WRITE,
		ACCESS_APPEND,
	};

	netFileStream(char* filename, int acccess, bool endianSwap);
	~netFileStream();

	virtual bool readBytes_internal(void* ptr, int len);
	virtual bool writeBytes_internal(const void* ptr, int len);

	Sys::I_File* m_file;
	const int	 m_access;
};


#endif //NETFILESTREAM_H