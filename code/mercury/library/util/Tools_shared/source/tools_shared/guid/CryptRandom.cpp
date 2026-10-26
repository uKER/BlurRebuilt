#include "stdafx.h"
#include <wincrypt.h>
#include <tools_shared/guid/CryptRandom.h>

namespace Tools
{

C_CryptRandom::C_CryptRandom() :
	_provider(0)
{
}

C_CryptRandom::~C_CryptRandom()
{
	Finalise();
}

bool C_CryptRandom::Initialise()
{
	if (_provider)
		return true;

	if (CryptAcquireContext(&_provider, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) == FALSE)
		return false;

	return true;
}

void C_CryptRandom::Finalise()
{
	if (_provider != 0)
	{
		CryptReleaseContext(_provider, 0);
		_provider = 0;
	}
}

bool C_CryptRandom::Generate(void* data, int length)
{
	if (_provider == 0)
		return false;

	if (CryptGenRandom(_provider, length, (BYTE*)data) == FALSE)
		return false;

	return true;
}

U64 C_CryptRandom::operator()()
{
	U64 result;
	
	if (!Generate(&result, sizeof(result)))
		return 0;

	return result;

}

}
