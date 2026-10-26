#include "MiscSharedPch.h"

#include <system/debuglog.h>
#include "misc_shared\cache\cache.h"
#include "misc_shared\cache\cachestream.h"

namespace DiskCache
{

// compare the contents of two CacheKeys
int CacheKeyObj::Compare(CacheKeyObj &t)
{
	// check sizes
	int x = this->iSize - t.iSize;
	if (x)
		return x;

	// check memory contents
	return memcmp(pData, t.pData, iSize);
}

// destructor
CacheKeyObj::~CacheKeyObj()
{
	delete [] pData;
}

}
