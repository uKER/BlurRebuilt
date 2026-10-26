#ifndef SCENE_GRAPH_MODELINSTANCEFLATLISTGATHER_H
#define SCENE_GRAPH_MODELINSTANCEFLATLISTGATHER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <system/scheduler/jobqueue.h>
#include <btl/containers/vector.h>
#include <btl/containers/list.h>
#include <SceneGraph/Visible/VolatileNodeInfo.h>

namespace SceneGraph
{

struct ViewData;
class C_View;
class C_ModelInstanceBasicCullNode;
class C_ModelInstance;

#define MAX_FLATLIST_VIEWS		6
#define MAX_FLATLIST_RESULTS	512

struct ALIGN16 ModelInstanceFlatListGatherData
{
	// max 6 views gathered simultaneously
#ifdef PLATFORM_SPU
	typedef Btl::Vector<ViewData*, Btl::VectorParameters_Static<MAX_FLATLIST_VIEWS> >	Views;
#else
	typedef Btl::Vector<const ViewData*, Btl::VectorParameters_Static<MAX_FLATLIST_VIEWS> >	Views;
#endif

	// input to SPU
	struct ALIGN16 Input
	{
	#ifdef PLATFORM_SPU
		const C_ModelInstanceBasicCullNode**		ppCullNodes;
	#else
		const C_ModelInstanceBasicCullNode* const*	ppCullNodes;
	#endif
		U32											nodesCount;
		
	};

	// output from SPU
	struct ALIGN16 Output
	{
		U32								numResults;
		const C_View*					pView;
		S32								viewIndex;

		struct Result
		{
			const C_ModelInstance*		pModelInstance;
			VolatileNodeInfo			volatileInfo;
			U16							passIndex;
		};

		ALIGN16 Result					results[MAX_FLATLIST_RESULTS];
	};

	typedef Btl::Vector<Output*, Btl::VectorParameters_Static<MAX_FLATLIST_VIEWS> >	Outputs;

	Input		input;
	Outputs		outputs;
	Views		viewData;
};

} // namespace SceneGraph

extern Scheduler::jqModule ModelInstanceFlatListGatherModule;

#endif // SCENE_GRAPH_MODELINSTANCEFLATLISTGATHER_H
