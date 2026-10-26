#ifndef CROWD_BLOCK_VISIBILITY_TASK_H
#define CROWD_BLOCK_VISIBILITY_TASK_H

#include <crowd_system/CrowdDefines.h>
#include <crowd_system/CrowdEvent.h>
#include <crowd_system/CrowdVisibleBlockData.h>

#include <btl/containers/Vector.h>
#include <btl/utility/Pair.h>

namespace SceneGraph
{
	struct ViewData;
};

namespace AnimatedCrowd
{

//! C_CrowdBlockVisibilityTask
class ALIGN16 C_CrowdBlockVisibilityTask
{
public:
	C_CrowdBlockVisibilityTask();

	static bool		Run(C_CrowdBlockVisibilityTask* pTask);
	static bool		Schedule(C_CrowdBlockVisibilityTask* pTask, S32 group, S32 policy);

	// Inputs
	const C_CrowdPotentiallyVisibleBlock*
								m_PotentiallyVisibleBlocks;
	U16							m_NumPotentiallyVisibleBlocks;
	
	const SceneGraph::ViewData*	m_Views;
	U8							m_NumViews;

	const C_Event*				m_ActiveEvents;
	U8							m_NumActiveEvents;

	const CrowdDelimiter*		m_SortedBlocksXAxis;
	const CrowdDelimiter*		m_SortedBlocksZAxis;
	U16							m_NumSortedBlocks;
	U16							m_NumBlocks;

	// Output
	C_CrowdVisibleBlockData*	m_VisibleBlockData;
};

}; // namespace AnimatedCrowd

#endif // CROWD_BLOCK_VISIBILITY_TASK_H
