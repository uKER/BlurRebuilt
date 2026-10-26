/*
 * KdTreeTemplate.inl - contains the function declarations of the kd-tree template
 *
 */

#define KD_TREE_TEMPLATE_INL

#include "spatial_hierarchy/LpvsUtility.h"

//#define NO_MAILBOXES

template <typename T>
KdTreeTemplate<T>::KdTreeTemplate()
	: m_pTris(NULL), m_pNodes(NULL), m_pMailboxes(NULL)
{
	m_pMailboxHashTable = (int*)BizAlignedMalloc(MAILBOX_HASH_TABLE_SIZE * sizeof(float), 128);
}

template <typename T>
KdTreeTemplate<T>::~KdTreeTemplate()
{
	if (m_pNodes)		BizAlignedFree(m_pNodes);
	if (m_pMailboxes)	delete m_pMailboxes;
	BizAlignedFree(m_pMailboxHashTable);
}

// intialize this kd tree with the contents of a kd tree builder
// this destroys the kd tree builder
// you will still need to populate the m_Primitives buffer with primitives of <typename T>, which is contained in the derived class
template <typename T>
void KdTreeTemplate<T>::CopyTreeBuilderContents(KdBuilder *pBuilder, T *pPrims)
{
	// copy Nodes vector to a cache-aligned memory block
	m_nNodes = pBuilder->m_Nodes.Size()+1;
	m_pNodes = (KdNode*)BizAlignedMalloc(sizeof(KdNode) * (m_nNodes), 128);
	memcpy(m_pNodes+1, &pBuilder->m_Nodes[0], sizeof(KdNode) * pBuilder->m_Nodes.Size());

	m_pMailboxes = pBuilder->m_pMailboxes;
	pBuilder->m_pMailboxes = NULL;

	m_pTris = pPrims;

	delete pBuilder;
}


#ifndef DONT_USE_SIMD
// clip ray against the world bounding box
// this slightly decreases the number of nodes that get traversed
template <typename T>
void KdTreeTemplate<T>::ClipRayVsBBoxSIMD(const __m128 *vRayOrg, const __m128 *vInvRayDir, const int *iRaydir, __m128 *pfNear, __m128 *pfFar) const
{
	__m128 dmax, dmin;
	__m128 fNear = *pfNear;
	__m128 fFar  = *pfFar;

	for (int iAxis=0; iAxis<3; iAxis++)
	{
		dmax = _mm_load1_ps( &m_bbox.vmax.v[iAxis] );
		dmax = _mm_sub_ps( dmax, vRayOrg[iAxis] );
		dmax = _mm_mul_ps( dmax, vInvRayDir[iAxis] );

		dmin = _mm_load1_ps( &m_bbox.vmin.v[iAxis] );
		dmin = _mm_sub_ps( dmin, vRayOrg[iAxis] );
		dmin = _mm_mul_ps( dmin, vInvRayDir[iAxis] );

		if (iRaydir[iAxis] >= 0)
		{
			fFar = _mm_min_ps( dmax, fFar);
			fNear = _mm_max_ps( dmin, fNear);
		}
		else
		{
			fFar = _mm_min_ps( dmin, fFar);
			fNear = _mm_max_ps( dmax, fNear);
		}
	}

	*pfNear = fNear;
	*pfFar  = fFar;
}
#endif

template <typename T>
void KdTreeTemplate<T>::ClearMailboxHashTable() const
{
#ifndef DONT_USE_SIMD
	float *p = (float*)m_pMailboxHashTable;
	const static int x = 0xffffffff;
	const __m128 f = _mm_load1_ps((float*)&x);
	for (int i=0; i<(MAILBOX_HASH_TABLE_SIZE / 32); i++, p+=32)
	{
		_mm_store_ps(p+0, f );
		_mm_store_ps(p+4, f );
		_mm_store_ps(p+8, f );
		_mm_store_ps(p+12, f );
		_mm_store_ps(p+16, f );
		_mm_store_ps(p+20, f );
		_mm_store_ps(p+24, f );
		_mm_store_ps(p+28, f );
	}
#else
	// TODO: optimize this
	memset(m_pMailboxHashTable, -1, MAILBOX_HASH_TABLE_SIZE * sizeof(int));
#endif
}

// trace a ray through the kd-tree
template <typename T>
bool KdTreeTemplate<T>::Trace(const SD::Ray &ray, SD::RayTraceResult &result) const
{
	KdStack stack[KDNODE_STACK_SIZE];
	int iStackPos = 0;

	float fNear = m_xfEpsilon;
	float fFar	= result.fDistance;
	float d;

	Float3 vRayDir		= ray.direction;
	Float3 vRayOrg		= ray.origin;
	Float3 vInvRayDir = 1.f / ray.direction;

	KdNode *pNode = m_pNodes + 1;
	KdNode *pRootNode = pNode;
	KdNode *pNextNode, *pNextNode1, *pNextNode2;

	// precalculate ray direction signs
	int	iRayDir[3];
	iRayDir[0] = (vRayDir.v[0]>=0) ? 0:-1;
	iRayDir[1] = (vRayDir.v[1]>=0) ? 0:-1;
	iRayDir[2] = (vRayDir.v[2]>=0) ? 0:-1;

	// clip ray against our bounding box
	ClipRayVsBBox(vRayOrg, vInvRayDir, iRayDir, &fNear, &fFar, m_bbox);

	if (fNear > fFar)
		return false;

	ClearMailboxHashTable();

	// main ray-tracing loop
	while (1)
	{
		while (!KDNODE_ISLEAF(pNode))
		{
			int iAxis = KDNODE_DIMENSION(pNode);

			//
			// traverse ’til next leaf
			d = (pNode->fSplitCoordinate - vRayOrg.v[iAxis]) * vInvRayDir.v[iAxis];

			// calculate next near/far node pointers
			int r = iRayDir[iAxis];
			int o = KDNODE_OFFSET(pNode);
			r &= o;
			pNextNode = pNode+1;
			pNextNode1 = KDPTR_OFFSET(pNextNode, r);
			pNextNode2 = KDPTR_OFFSET(pNextNode, r^o);
			//
			if (d < fNear)
			{
				// case one, d <= t_near <= t_far -> cull front side
				pNode = pNextNode2;
				continue;
			}
//			_mm_prefetch((char*)pNextNode1,_MM_HINT_T0); // prefetch next node
			if (d > fFar)
			{
				// case two, t_near <= t_far <= d -> cull back side
				pNode = pNextNode1;
				continue;
			}
			{
				// case three: traverse both sides in turn
				// push back side onto the stack
				stack[iStackPos].pNode = pNextNode2;
				stack[iStackPos].fNear = d;
				stack[iStackPos].fFar  = fFar;
				iStackPos++;

				// get front side
				pNode = pNextNode1;
				fFar  = d;
			}
		}
		// have a leaf now
		if (pNode->iFlagAndNumPrimitives & 0x7fffffff)
		{
			if (ray.bShadowRay)
			{
				if (IntersectLeafS(pNode->iPrimitive, ray, result))
					return true;
			}
			else
			{
				IntersectLeaf(pNode->iPrimitive, ray, result);
				if (fFar > result.fDistance)
					return true; // early ray termination
			}
		}
		if (!iStackPos)
			return false; // noting else to traverse any more...

		// pop
		iStackPos--;
		pNode = stack[iStackPos].pNode;
		fNear = stack[iStackPos].fNear;
		fFar  = stack[iStackPos].fFar;
	}
	return false;
}

extern float xfMinusOne;
extern float xfOne;
extern unsigned int xfMinusOneInv;
#ifndef DONT_USE_SIMD
extern __m128 fOne;
extern __m128 fZero;
extern __m128 fMinusOne;
extern __m128 fMinusOneInv;
//extern __m128 fEpsilon;

// trace 4 rays through the kd-tree simultaneously using SIMD
template <typename T>
void KdTreeTemplate<T>::TraceSIMD(SIMDRay &ray, SIMDResult &result) const
{
	KdSIMDStack stack[KDNODE_STACK_SIZE];
	int iStackPos = 0;

	// setup SIMD variables
	__m128 fSplit;
	__m128 fFar	= result.fDistance;
	__m128 fNear = _mm_load1_ps( &m_xfEpsilon );
	__m128 mask, d;
	__m128 q;
	__m128 vRayDir[3];
	__m128 vRayOrg[3];
	__m128 vInvRayDir[3];
	__m128 hitmask;
	static const __m128 fZero = _mm_setzero_ps();

	ClearMailboxHashTable();

	for (int i=0; i<3; i++)
	{
		vRayDir[i] = ray.direction[i];
		vRayOrg[i] = ray.origin[i];
		// we're not using the SSE reciprocal instruction because it's an
		// approximation and therefore not all that accurate
		vInvRayDir[i] = _mm_div_ps(fOne, vRayDir[i]);
	}

	KdNode *pNode = m_pNodes + 1;
	KdNode *pNextNode, *pNextNode1, *pNextNode2;
	KdNode *pRootNode = pNode;

	// precalculate ray direction signs
	int	iRayDir[3];
	iRayDir[0] = (ray.fDirection[0][0]>=0) ? 0:-1;
	iRayDir[1] = (ray.fDirection[1][0]>=0) ? 0:-1;
	iRayDir[2] = (ray.fDirection[2][0]>=0) ? 0:-1;

	// clip ray against our bounding box
	ClipRayVsBBoxSIMD(vRayOrg, vInvRayDir, iRayDir, &fNear, &fFar);

	// if fNear > fFar for all rays then exit
//	if (!_mm_movemask_ps(fNear <= fFar))
//		return;

	// reset hitmask
	hitmask = _mm_cmplt_ps( result.hit, fZero );

	ClearMailboxHashTable();

	while (1)
	{
		while (!KDNODE_ISLEAF(pNode))
		{
			int iAxis = KDNODE_DIMENSION(pNode);

			// generate active ray mask; -1 = active, 0 = inactive
			mask = _mm_cmpge_ps(fFar, fNear);
			//
			// traverse ’til next leaf
			fSplit = _mm_load1_ps(&pNode->fSplitCoordinate);
			d = _mm_sub_ps( fSplit, vRayOrg[iAxis] );
			d = _mm_mul_ps( d, vInvRayDir[iAxis] );

			// calculate next node pointers
			int r = iRayDir[iAxis];
			int o = KDNODE_OFFSET(pNode);
			r &= o;
			pNextNode = pNode+1;
			pNextNode1 = KDPTR_OFFSET(pNextNode, r);
			pNextNode2 = KDPTR_OFFSET(pNextNode, r^o);

			// if (d < fNear)
			q = _mm_cmpge_ps( d, fNear );
			q = _mm_and_ps( q , mask ); 
			if (!_mm_movemask_ps( q ) )
			{
				// case one, d <= t_near <= t_far -> cull front side
				pNode = pNextNode2;
				continue;
			}
			_mm_prefetch((char*)pNextNode1,_MM_HINT_T0); // prefetch next node

			// if (d > fFar)
			q = _mm_cmple_ps(d, fFar);
			q = _mm_and_ps( q , mask);
			if (!_mm_movemask_ps( q ))
			{
				// case two, t_near <= t_far <= d -> cull back side
				pNode = pNextNode1;
				continue;
			}

			// recurse into both
			{
				// case three: traverse both sides in turn
				// push back side onto the stack
				stack[iStackPos].pNode = pNextNode2;
				stack[iStackPos].fNear = _mm_max_ps( d, fNear );
				stack[iStackPos].fFar  = fFar;
				iStackPos++;

				// get front side
				pNode = pNextNode1;
				fFar  = _mm_min_ps( d, fFar );
			}
		}
		// have a leaf now
		if (pNode->iFlagAndNumPrimitives & 0x7fffffff)
		{
//			if (ray.bShadowRay)
			{
				hitmask = _mm_cmplt_ps( result.hit, fZero ); // mask out the rays that got a hit so they don't get processed again
				IntersectLeafSIMD(pNode->iPrimitive, ray, result, hitmask);
				hitmask = _mm_cmplt_ps( result.hit, fZero ); // recalc hitmask

				// return if all rays have hit something
				int a = _mm_movemask_ps( result.hit );
				if (!a)
					goto stop;
			}
		}

		do
		{
			if (!iStackPos)
				goto stop; // noting else to traverse any more...

			// pop
			iStackPos--;
			pNode = stack[iStackPos].pNode;
			fNear = stack[iStackPos].fNear;
			fFar  = stack[iStackPos].fFar;

			// apply hitmask, mask out previously active rays that have hit something
			fFar = _mm_and_ps(fFar, hitmask);

			// if none of the rays are active any more, skip this node and pop the next one
			mask = _mm_cmpge_ps(fFar, fNear);
		}
		while (!_mm_movemask_ps( mask ));
	}

stop:
	;
}
#endif

// intersect a ray with all the primitives in a leafnode
template <typename T>
void KdTreeTemplate<T>::IntersectLeaf(int iPrimitive, const SD::Ray &ray, SD::RayTraceResult &result) const
{
	int iTri, iPrim = 0;

	// keep processing mailboxes in sequences until we hit one with the high-bit of the primitive index set
	for (KdMailbox *pBox = &m_pMailboxes->At(iPrimitive); iPrim>=0; pBox++)
	{
		iPrim = pBox->iPrimitive;
		iTri = iPrim & 0x7fffffff;
		T *pTri = &m_pTris[ iTri ];

		int *pMailBox = &m_pMailboxHashTable[iTri&(MAILBOX_HASH_TABLE_SIZE-1)];
		if (*pMailBox == iTri)
			continue;
		*pMailBox = iTri;

        // collide against this primitive
		if (pTri->Intersect(ray, result, (void*)this))
			result.iPrimitive = iTri;
	}
}

// intersect a shadow ray with all the primitives in a leafnode
template <typename T>
bool KdTreeTemplate<T>::IntersectLeafS(int iPrimitive, const SD::Ray &ray, SD::RayTraceResult &result) const
{
	int iTri, iPrim = 0;

	// keep processing mailboxes in sequences until we hit one with the high-bit of the primitive index set
	for (KdMailbox *pBox = &m_pMailboxes->At(iPrimitive); iPrim>=0; pBox++)
	{
		iPrim = pBox->iPrimitive;
		iTri = iPrim & 0x7fffffff;
		T *pTri = &m_pTris[ iTri ];
		// TODO: prefetch the next triangle

		int *pMailBox = &m_pMailboxHashTable[iTri&(MAILBOX_HASH_TABLE_SIZE-1)];
		if (*pMailBox == iTri)
			continue;
		*pMailBox = iTri;

		if (pTri->Intersect(ray, result, (void*)this))
		{
			result.iPrimitive = iTri;
			return true;
		}
	}
	return false;
}

// intersect 4 rays with all the triangles in a leaf
#ifndef DONT_USE_SIMD
template <typename T>
void KdTreeTemplate<T>::IntersectLeafSIMD(int iPrimitive, SIMDRay &ray, SIMDResult &result, __m128 mask) const
{
	int iTri, iPrim = 0;

	// keep processing mailboxes in sequences until we hit one with the high-bit of the primitive index set
	for (KdMailbox *pBox = &m_pMailboxes->At(iPrimitive); iPrim>=0; pBox++)
	{
		iPrim = pBox->iPrimitive;
		iTri = iPrim & 0x7fffffff;
		T *pTri = &m_pTris[ iTri ];
		// TODO: prefetch the next triangle

		int *pMailBox = &m_pMailboxHashTable[iTri&(MAILBOX_HASH_TABLE_SIZE-1)];
		if (*pMailBox == iTri)
			continue;
		*pMailBox = iTri;

		pTri->IntersectSIMD(ray, result, mask, (void*)this);
		if (!_mm_movemask_ps( result.hit ))
			return;
	}
}
#endif
/*
// check whether all the directions in a batch of rays have the same sign bits
template <typename T>
bool KdTreeTemplate<T>::DirectionSignsMatch(SD::RayBatch &batch) const
{
	if (batch.m_nRays < 2)
		return true;

	for (int i=1; i<batch.m_nRays; i++)
	{
		int iDontMatch =	DIRECTION_BITS_DONT_MATCH(batch.m_Ray[0].direction.x, batch.m_Ray[i].direction.x) |
							DIRECTION_BITS_DONT_MATCH(batch.m_Ray[0].direction.y, batch.m_Ray[i].direction.y) |
							DIRECTION_BITS_DONT_MATCH(batch.m_Ray[0].direction.z, batch.m_Ray[i].direction.z);
		if (iDontMatch)
			return false;
	}

	// all the bits match
	return true;
}
*/

// call the touch function on all primitives in this leafnode
template <typename T>
void KdTreeTemplate<T>::TraceLeaf(int iPrimitive) const
{
	int iTri, iPrim = 0;

	// keep processing mailboxes in sequences until we hit one with the high-bit of the primitive index set
	for (KdMailbox *pBox = &m_pMailboxes->At(iPrimitive); iPrim>=0; pBox++)
	{
		iPrim = pBox->iPrimitive;
		iTri = iPrim & 0x7fffffff;
		T *pTri = &m_pTris[ iTri ];

		int *pMailBox = &m_pMailboxHashTable[iTri&(MAILBOX_HASH_TABLE_SIZE-1)];
		if (*pMailBox == iTri)
			continue;
		*pMailBox = iTri;

        // collide against this primitive
		pTri->Intersect((void*)this);
	}
}

// box-vs-bsp trace function
template <typename T>
void KdTreeTemplate<T>::Trace(const Box3 &bbox) const
{
	KdNode *pNode = m_pNodes + 1;
	KdNode *pNextNode, *pNextNode1, *pNextNode2;

	KdNode			*stack[KDNODE_STACK_SIZE];
	int				iStackPos = 0;

	ClearMailboxHashTable();

	while (1)
	{
		while (!KDNODE_ISLEAF(pNode))
		{
			// get node data
			int iAxis = KDNODE_DIMENSION(pNode);
			float d = pNode->fSplitCoordinate;

			// calculate next near/far node pointers
			pNextNode = pNode+1;
			pNextNode1 = KDPTR_OFFSET(pNextNode, 0);
			pNextNode2 = KDPTR_OFFSET(pNextNode, KDNODE_OFFSET(pNode));

			// totally on the left side?
			if (bbox.vmax.v[iAxis]<d)
			{
				pNode = pNextNode1;
				continue;
			}
			// totally on the right side?
			if (bbox.vmin.v[iAxis]>d)
			{
				pNode = pNextNode2;
				continue;
			}

			// do both sides
			stack[iStackPos] = pNextNode1;
			iStackPos++;
			pNode = pNextNode2;
		}

		// have a leafnode
		if (pNode->iFlagAndNumPrimitives & 0x7fffffff)
		{
			TraceLeaf(pNode->iPrimitive);
		}
		if (!iStackPos)
			return; // noting else to traverse any more...

		// pop
		iStackPos--;
		pNode = stack[iStackPos];
	}

}

void AddBoundingBox(const Box3 &bbox);

// box-vs-bsp trace function
template <typename T>
void KdTreeTemplate<T>::Draw() const
{
	KdNode *pNode = m_pNodes + 1;
	KdNode *pNextNode, *pNextNode1, *pNextNode2;

	Box3 bbox;

	bbox.vmax = m_bbox.vmin;
	bbox.vmin = m_bbox.vmax;

	struct StackItem
	{
		KdNode *node;
		Box3 bbox;
	};
	StackItem		stack[16*1024];
	int				iStackPos = 0;

	while (1)
	{
		while (!KDNODE_ISLEAF(pNode))
		{
			// get node data
			int iAxis = KDNODE_DIMENSION(pNode);
			float d = pNode->fSplitCoordinate;

			// calculate next near/far node pointers
			pNextNode = pNode+1;
			pNextNode1 = KDPTR_OFFSET(pNextNode, 0);
			pNextNode2 = KDPTR_OFFSET(pNextNode, KDNODE_OFFSET(pNode));

			Box3 bbox1 = bbox;
			Box3 bbox2 = bbox;
			bbox1.vmax[iAxis] = d;
			bbox2.vmin[iAxis] = d;

			// push far side on the stack and process near side
			stack[iStackPos].node = pNextNode1;
			stack[iStackPos].bbox = bbox2;
			iStackPos++;

			pNode = pNextNode2;
			bbox = bbox1;
		}

		// have a leafnode
		if (pNode->iFlagAndNumPrimitives & 0x7fffffff)
		{
			AddBoundingBox(bbox);
		}
		if (!iStackPos)
			return; // noting else to traverse any more...

		// pop
		iStackPos--;
		pNode = stack[iStackPos].node;
		bbox = stack[iStackPos].bbox;
	}

}

