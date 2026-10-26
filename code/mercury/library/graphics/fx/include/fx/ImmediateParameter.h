#ifndef __FX_IMMEDIATEPARAMETER_H__
#define __FX_IMMEDIATEPARAMETER_H__

#include "fx/FX.h"

namespace FX
{

struct Parameter;
class I_Effect;

#define IMMEDIATEPARAM_NULL		0
#define IMMEDIATEPARAM_EFFECT	1
#define IMMEDIATEPARAM_SHARED	2

#define IMMEDIATEPARAM_CONSTANT 0
#define IMMEDIATEPARAM_TEXTURE	1
/*
struct ImmediateParameter
{
	U8 iIdx;			// parameter index
	U8 cSize;			// size of the parameter in shader constant registers (ignored for textures)
	U8 iParamType;		// NULL, Effect or Shared
	U8 iResourceType;	// constant or texture
	U32 iOffset;		// shared parameter data buffer offset
};
*/

typedef U64 ImmediateParameter;

/// convert a normal FX::Parameter to an FX::ImmediateParameter
ImmediateParameter GetImmediateParameter(const I_Effect *pEffect, const Parameter *pParam, U32 cMaxSize = UINT_MAX, const SharedBindingBits *pUsage = NULL);

}

#endif
