#ifndef SCENE_GRAPH_PORTALCULLNODE_CLASSIFY_H
#define SCENE_GRAPH_PORTALCULLNODE_CLASSIFY_H

#ifdef _MSC_VER
#pragma once
#endif

#include <System/scheduler/jobqueue.h>
#include <Btl/Containers/vector.h>
#include <Btl/Containers/list.h>


// Call this forward:
namespace Portal
{
	class C_BSPContainer;
};


namespace SceneGraph
{
	// Call these forward.
	struct	ViewData;
	class	C_View;
	class	C_ModelInstanceBasicCullNode;
	class	C_ModelInstance;

	/// Scheduler task I/O structure.
	struct ALIGN16 PortalCullNodeClassifyData
	{
		static const U32		MaxBatchSize	= 256;		// Maximum number of spatial probes to classify per job -- double this must be an even 16byte multiple.
		static const U32		MaxSystems		= 8;		// Maximum number of portal system objects.

		/// Input to SPU task.
		struct ALIGN16 Input
		{
			Maths::HVec3					m_vSpatialProbes[PortalCullNodeClassifyData::MaxBatchSize];	// List of spatial probes to classify.
			U16								m_vLastRoom[PortalCullNodeClassifyData::MaxBatchSize];		// List of room indicies that the spatial probe was last seen.
			U16								m_vLastSystem[PortalCullNodeClassifyData::MaxBatchSize];	// List of portal system indicies that the spatial probe was last seen.
			const Portal::C_BSPContainer*	m_vTreeEAs[PortalCullNodeClassifyData::MaxSystems];			// List of effective addresses of portal system BSP objects.
			const ViewData*					m_pViewData;												// Effective address of ViewData object for the view we are gathering.
			const U16*						m_pOutRoomVec;												// Effective address of output vector of room indices.
			const U16*						m_pOutPortalVec;											// Effective address of output vector of portal indices.
			U16								m_iProbeCount;												// Number of spatial probes in the list.
			U16								m_iSystemCount;												// Number of portal system objects currently active.
		};

		// Output from SPU task.
		struct ALIGN16 Output
		{
			ALIGN16 C_ModelInstance*		m_vModelInstances[PortalCullNodeClassifyData::MaxBatchSize];// Effective addresses of model instances -- don't Dma this back!!
			ALIGN16 U16						m_vRoomIndices[PortalCullNodeClassifyData::MaxBatchSize];	// Room indicies for spatial probes.
			ALIGN16 U16						m_vPortalIndices[PortalCullNodeClassifyData::MaxBatchSize];	// Portal system indicies for spatial probes.
		};

		Input		m_Input;
		Output		m_Output;
	};

	/// SPU job input structure.
	struct ALIGN16 PortalCullNodeClassifyInput
	{
		PortalCullNodeClassifyData*	pData;
	};

} // namespace SceneGraph

extern Scheduler::jqModule PortalCullNodeClassifyModule;

#endif // SCENE_GRAPH_PORTALCULLNODE_CLASSIFY_H
