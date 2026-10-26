#ifndef SCENE_GRAPH_CACHEDRENDERLISTNODEGATHER_H
#define SCENE_GRAPH_CACHEDRENDERLISTNODEGATHER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <system/scheduler/jobqueue.h>
#include <btl/containers/vector.h>
#include <btl/containers/bitvector.h>
#include <btl/containers/list.h>
#include <SceneGraph/Visible/VolatileNodeInfo.h>

#if !defined(PLATFORM_PS3) && !defined(PLATFORM_SPU)
//#define USE_DYNAMIC_RINGBUFFER
#endif

namespace SceneGraph
{

struct ViewData;
class C_Bounds;
class C_View;
class C_StaticRenderListNode;
class C_CachedRenderListNode;

#define NUM_RESULTS	896
#define MAX_VIEWS 6

//#define USE_DYNAMIC_RINGBUFFER

// results of processing the node on an SPU/MT - store this on the view?
struct ALIGN16 CachedRenderListNodeData
{
	// max 6 views gathered simultaneously
#ifdef PLATFORM_SPU
	typedef Btl::Vector<ViewData*, Btl::VectorParameters_Static<MAX_VIEWS> >	Views;
#else
	typedef Btl::Vector<const ViewData*, Btl::VectorParameters_Static<MAX_VIEWS> >	Views;
#endif

	// 2 per cache line
	struct ALIGN16 PassData
	{
		// input to SPU
		struct ALIGN16 Input
		{
			const C_Bounds*		pBounds;
			const U16*			pIndices;
			const U16*			pFlags;
			U32					boundsCount;
			U32					renderPassType;
			U32					volatileInfoFlags;
		};

		// output from SPU
		struct ALIGN16 Output
		{
			U32					numResults;
			const C_View*		pView;
			S32					viewIndex;

			struct Result
			{
				// this should now be 16 byte aligned after the 3 pts in base class and member above
				U16					index;
				VolatileNodeInfo	volatileInfo;
			};

			ALIGN16 Result			results[NUM_RESULTS];
		};

		typedef Btl::Vector<Output*, Btl::VectorParameters_Static<MAX_VIEWS> >			Outputs;

		Input		input;
		Outputs		outputs;	// number of outputs should be equal to size of viewData
	};

	// pointers to elements of that list
	PassData::Output*												pFirstOutput;
	Btl::Vector<PassData*, Btl::VectorParameters_Static<5> >		primitiveData;
	const U8*														pPrimitivePvs;
	U32																primitivePvsCount;

	Btl::Vector<PassData*, Btl::VectorParameters_Static<5> >		instanceData;
	const U8*														pInstancePvs;
	U32																instancePvsCount;

	const U8*														pPrimitivePortal;
	U32																primitivePortalCount;
	const U8*														pInstancePortal;
	U32																instancePortalCount;
	Views															viewData;

	const C_CachedRenderListNode*									pCachedRenderListNode;
};

struct ALIGN16 CachedRenderListNodeDataBatch
{
	Btl::Vector<CachedRenderListNodeData*, Btl::VectorParameters_Static<16> >		cachedRenderListNodeData;
};

} // namespace SceneGraph

extern Scheduler::jqModule CachedRenderListNodeGatherModule;

FORCE_INLINE U32 GetBitValue(const U8* pData, int i)
{
	int index = i / 8;
	int bit = i % 8;
	return (pData[index] & (1 << bit)) != 0;
}

#endif // SCENE_GRAPH_CACHEDRENDERLISTNODEJOBQUEUE_H
