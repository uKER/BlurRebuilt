#ifndef __COBJECTID_H
#define __COBJECTID_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ComponentSystem/CHash.h>

namespace airacingtech
{
	// An object id is strictly just a CHash, but it's nice to be able to refer to them as CObjectId
	typedef	CHash CObjectId;
}

#endif //__COBJECTID_H
