#ifndef MERCURY_UTIL_SECURITY_H
#define MERCURY_UTIL_SECURITY_H

#pragma once

#include <System/Misc/Security.h>

namespace Security
{

class RSASecurity : public I_Security
{
protected:
	U8		*m_key, *m_signature;
	size_t	m_keyLength, m_sigLength;
	U8		m_rsaKeyLength;

public:
	RSASecurity(unsigned int rsaKeyLength);
	~RSASecurity();

	void InitKey(const U8* key, size_t keyLength);
	void InitSignature(const Btl::String& sigFilename);

	bool Verify(const U8* buffer, size_t bufferLength, const char* name=NULL) const;
};

}

#endif
