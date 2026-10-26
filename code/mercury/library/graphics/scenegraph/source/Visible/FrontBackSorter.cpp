#include "stdafx.h"

#include <SceneGraph/Visible/FrontBackSorter.h>
#include <SceneGraph/Visible/Nodes.h>
#include <SceneGraph/RenderingData/StaticRenderListNode.h>
#include <SceneGraph/RenderingData/ModelInstanceNode.h>
#include <SceneGraph/View.h>

#include <RenderingData/RenderingData.h>

#include <btl/containers/radix.h>

namespace SceneGraph
{

Debug::Bool	g_SortFrontBack	("Graphics.SceneGraph.Sorting.UseFrontBack", true);

C_FrontBackSorter::C_FrontBackSorter()
{
	// back front sorted needs distance to sort objects
	m_VolatileInfoFlags = E_VolatileNodeInfo_Distance;
}

void C_FrontBackSorter::Sort(const C_View* pView, C_VisibleNodeList* pVisibleNodeList) const
{
	if(!g_SortFrontBack)
		return;

	PERF_EVENT("SceneGraph::FrontBackSorter");

	S32 totalNodes =	pVisibleNodeList->m_Nodes.Size() +
						pVisibleNodeList->m_ModelInstances.Size() +
						pVisibleNodeList->m_StaticPrimitives.Size() +
						pVisibleNodeList->m_StaticInstances.Size();

	pVisibleNodeList->m_Nodes.Reserve(totalNodes);

	// calculate sort keys for primitives and instances
	{
		for(S32 i = 0; i < pVisibleNodeList->m_StaticPrimitives.Size(); ++i)
		{
			C_VisibleNode& node = pVisibleNodeList->m_StaticPrimitives[i];
			node.m_SortKey = node.m_VolatileNodeInfo.distance;
			
			pVisibleNodeList->m_Nodes.PushBack(node);
		}

		for(S32 i = 0; i < pVisibleNodeList->m_StaticInstances.Size(); ++i)
		{
			C_VisibleNode& node = pVisibleNodeList->m_StaticInstances[i];
			node.m_SortKey = node.m_VolatileNodeInfo.distance;

			pVisibleNodeList->m_Nodes.PushBack(node);
		}

		for(S32 i = 0; i < pVisibleNodeList->m_ModelInstances.Size(); ++i)
		{
			C_VisibleNode& node = pVisibleNodeList->m_ModelInstances[i];
			node.m_SortKey = node.m_VolatileNodeInfo.distance;

			pVisibleNodeList->m_Nodes.PushBack(node);
		}
	}

	m_Sorter.Sort(pVisibleNodeList->m_Nodes);

	pVisibleNodeList->m_StaticPrimitives.Clear();
	pVisibleNodeList->m_StaticInstances.Clear();
	pVisibleNodeList->m_ModelInstances.Clear();
}

} // namespace SceneGraph

