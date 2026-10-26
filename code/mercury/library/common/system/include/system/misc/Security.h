#ifndef CORE_SYSTEM_SECURITY_H
#define CORE_SYSTEM_SECURITY_H

#pragma once

namespace Security
{

class I_Security
{
public:
	virtual					~I_Security()
	{
	}

	virtual bool			Verify(const U8* buffer, size_t bufferLength, const char* name=NULL) const = NULL;
};

}

#endif