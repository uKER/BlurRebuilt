#pragma once

struct KdNode;

#define DIRECTION_BITS_DONT_MATCH(X,Y) ((((*((int*)&(X))) ^ (*((int*)&(Y))))) & 0x80000000)

#ifndef DONT_USE_SIMD
struct SIMDRay
{
	union
	{
		struct 
		{
			float fOrigin[3][4];
			float fDirection[3][4];
			float fLength[4];
		};
		struct
		{
			__m128 origin[3];
			__m128 direction[3];
			__m128 length;
		};
	};
	bool  bShadowRay;
};

struct SIMDResult
{
	union
	{
		__m128	hit;
		int		bHit[4];
	};
	__m128 fDistance;
#ifdef STATS
	int nMailboxesVisited;
	int	nMailboxesChecked;
#endif
};

// kd stack structure used to recurse through the kd-tree
struct KdSIMDStack
{
	KdNode *pNode;
	__m128	fNear;
	__m128	fFar;
};

#endif
