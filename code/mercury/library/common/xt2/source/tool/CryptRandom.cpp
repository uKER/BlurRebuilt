#include "Common/XtPch.h"
#include "Xt/XtTool.h"
#include "Xt/Impl/CryptRandom.h"

CryptRandom::CryptRandom() :
	_provider(0)
{
}

CryptRandom::~CryptRandom()
{
	Finalise();
}

bool CryptRandom::Initialise()
{
	if (_provider)
		return true;

	if (CryptAcquireContext(&_provider, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) == FALSE)
		return false;

	return true;
}

void CryptRandom::Finalise()
{
	if (_provider != 0)
	{
		CryptReleaseContext(_provider, 0);
		_provider = 0;
	}
}

bool CryptRandom::Generate(void* data, int length)
{
	if (_provider == 0)
		return false;

	if (CryptGenRandom(_provider, length, (BYTE*)data) == FALSE)
		return false;

	return true;
}

XtU64 CryptRandom::operator()()
{
	XtU64 result;
	
	if (!Generate(&result, sizeof(result)))
		return 0;

	return result;

}