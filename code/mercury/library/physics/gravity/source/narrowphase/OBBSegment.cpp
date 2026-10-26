#include "StdAfx.h"

namespace Collision
{

static U32 _frontIndices[] = {1, 2, 3};						//-x, -y, -z
static U32 _backIndices[] = {4, 5, 6};						//x, y, z

const U32* const __restrict FrontIndices()
{
	return &_frontIndices[0];
}
const U32* const __restrict BackIndices()
{
	return &_backIndices[0];
}

};

