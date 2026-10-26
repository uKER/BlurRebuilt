#include "stdafx.h"

#include <SceneGraph/Visible/DepthSorter.h>
#include <SceneGraph/Visible/Nodes.h>
#include <SceneGraph/RenderingData/StaticRenderListNode.h>
#include <SceneGraph/RenderingData/ModelInstanceNode.h>
#include <SceneGraph/View.h>

#include <RenderingData/RenderingData.h>

#include <btl/containers/radix.h>

namespace SceneGraph
{

Debug::Bool	g_DepthSortEnable	("Graphics.SceneGraph.Sorting.DepthSort", true);

C_DepthSorter::C_DepthSorter() :	
	m_Order(E_DepthSortOrder_FrontBack),
	m_Method(E_DepthSortMethod_All),
	m_Primitives(true),
	m_Instances(true),
	m_ModelInstances(true),
	m_Abstracts(true)
{
	// back front sorted needs distance to sort objects
	m_VolatileInfoFlags = E_VolatileNodeInfo_Distance;
}

C_DepthSorter::C_DepthSorter(E_DepthSortOrder order, E_DepthSortMethod method) :
	m_Order(order),
	m_Method(method),
	m_Primitives(true),
	m_Instances(true),
	m_ModelInstances(true),
	m_Abstracts(true)
{
	// back front sorted needs distance to sort objects
	m_VolatileInfoFlags = E_VolatileNodeInfo_Distance;
}

void C_DepthSorter::Sort(const C_View* pView, C_VisibleNodeList* pVisibleNodeList) const
{
	if(!g_DepthSortEnable)
		return;

	PERF_EVENT("SceneGraph::Depth");

	// calculate sort keys for primitives and instances
	if(m_Method == E_DepthSortMethod_All)
	{
		S32 totalNodes =	pVisibleNodeList->m_Nodes.Size() +
							pVisibleNodeList->m_ModelInstances.Size() +
							pVisibleNodeList->m_StaticPrimitives.Size() +
							pVisibleNodeList->m_StaticInstances.Size();

		pVisibleNodeList->m_Nodes.Reserve(totalNodes);

		if(m_Order == E_DepthSortOrder_FrontBack)
		{
			if(m_Abstracts)
			{
				for(S32 i = 0; i < pVisibleNodeList->m_Nodes.Size(); ++i)
				{
					C_VisibleNode& node = pVisibleNodeList->m_Nodes[i];
					node.m_SortKey = node.m_VolatileNodeInfo.distance;
				}
			}

			if(m_Primitives)
			{
				for(S32 i = 0; i < pVisibleNodeList->m_StaticPrimitives.Size(); ++i)
				{
					C_VisibleNode& node = pVisibleNodeList->m_StaticPrimitives[i];
					node.m_SortKey = node.m_VolatileNodeInfo.distance;
					pVisibleNodeList->m_Nodes.PushBack(node);
				}
				pVisibleNodeList->m_StaticPrimitives.Clear();
			}

			if(m_Instances)
			{
				for(S32 i = 0; i < pVisibleNodeList->m_StaticInstances.Size(); ++i)
				{
					C_VisibleNode& node = pVisibleNodeList->m_StaticInstances[i];
					node.m_SortKey = node.m_VolatileNodeInfo.distance;
					pVisibleNodeList->m_Nodes.PushBack(node);
				}
				pVisibleNodeList->m_StaticInstances.Clear();
			}

			if(m_ModelInstances)
			{
				for(S32 i = 0; i < pVisibleNodeList->m_ModelInstances.Size(); ++i)
				{
					C_VisibleNode& node = pVisibleNodeList->m_ModelInstances[i];
					node.m_SortKey = node.m_VolatileNodeInfo.distance;
					pVisibleNodeList->m_Nodes.PushBack(node);
				}
				pVisibleNodeList->m_ModelInstances.Clear();
			}
		}
		else
		{
			U32 maxInt = 0xFFFFFFFF;

			if(m_Abstracts)
			{
				for(S32 i = 0; i < pVisibleNodeList->m_Nodes.Size(); ++i)
				{
					C_VisibleNode& node = pVisibleNodeList->m_Nodes[i];
					node.m_SortKey = maxInt - node.m_VolatileNodeInfo.distance;
				}
			}
			
			if(m_Primitives)
			{
				for(S32 i = 0; i < pVisibleNodeList->m_StaticPrimitives.Size(); ++i)
				{
					C_VisibleNode& node = pVisibleNodeList->m_StaticPrimitives[i];
					node.m_SortKey = maxInt - node.m_VolatileNodeInfo.distance;
					pVisibleNodeList->m_Nodes.PushBack(node);
				}
				pVisibleNodeList->m_StaticPrimitives.Clear();
			}

			if(m_Instances)
			{
				for(S32 i = 0; i < pVisibleNodeList->m_StaticInstances.Size(); ++i)
				{
					C_VisibleNode& node = pVisibleNodeList->m_StaticInstances[i];
					node.m_SortKey = maxInt - node.m_VolatileNodeInfo.distance;
					pVisibleNodeList->m_Nodes.PushBack(node);
				}
				pVisibleNodeList->m_StaticInstances.Clear();
			}

			if(m_ModelInstances)
			{
				for(S32 i = 0; i < pVisibleNodeList->m_ModelInstances.Size(); ++i)
				{
					C_VisibleNode& node = pVisibleNodeList->m_ModelInstances[i];
					node.m_SortKey = maxInt - node.m_VolatileNodeInfo.distance;
					pVisibleNodeList->m_Nodes.PushBack(node);
				}
				pVisibleNodeList->m_ModelInstances.Clear();
			}
		}

		// sort all the types together in the nodes
		m_Sorter.Sort(pVisibleNodeList->m_Nodes);
	}
	else
	{
		if(m_Order == E_DepthSortOrder_FrontBack)
		{
			if(m_Abstracts)
			{
				for(S32 i = 0; i < pVisibleNodeList->m_Nodes.Size(); ++i)
				{
					C_VisibleNode& node = pVisibleNodeList->m_Nodes[i];
					node.m_SortKey = node.m_VolatileNodeInfo.distance;
				}
				m_Sorter.Sort(pVisibleNodeList->m_Nodes);
			}

			if(m_Primitives)
			{
				for(S32 i = 0; i < pVisibleNodeList->m_StaticPrimitives.Size(); ++i)
				{
					C_VisibleNode& node = pVisibleNodeList->m_StaticPrimitives[i];
					node.m_SortKey = node.m_VolatileNodeInfo.distance;
				}
				m_Sorter.Sort(pVisibleNodeList->m_StaticPrimitives);
			}

			if(m_Instances)
			{
				for(S32 i = 0; i < pVisibleNodeList->m_StaticInstances.Size(); ++i)
				{
					C_VisibleNode& node = pVisibleNodeList->m_StaticInstances[i];
					node.m_SortKey = node.m_VolatileNodeInfo.distance;
				}
				m_Sorter.Sort(pVisibleNodeList->m_StaticInstances);
			}

			if(m_ModelInstances)
			{
				for(S32 i = 0; i < pVisibleNodeList->m_ModelInstances.Size(); ++i)
				{
					C_VisibleNode& node = pVisibleNodeList->m_ModelInstances[i];
					node.m_SortKey = node.m_VolatileNodeInfo.distance;
				}
				m_Sorter.Sort(pVisibleNodeList->m_ModelInstances);
			}
		}
		else
		{
			U32 maxInt = 0xFFFFFFFF;

			if(m_Abstracts)
			{
				for(S32 i = 0; i < pVisibleNodeList->m_Nodes.Size(); ++i)
				{
					C_VisibleNode& node = pVisibleNodeList->m_Nodes[i];
					node.m_SortKey = maxInt - node.m_VolatileNodeInfo.distance;
				}
				m_Sorter.Sort(pVisibleNodeList->m_Nodes);
			}

			if(m_Primitives)
			{
				for(S32 i = 0; i < pVisibleNodeList->m_StaticPrimitives.Size(); ++i)
				{
					C_VisibleNode& node = pVisibleNodeList->m_StaticPrimitives[i];
					node.m_SortKey = maxInt - node.m_VolatileNodeInfo.distance;
				}
				m_Sorter.Sort(pVisibleNodeList->m_StaticPrimitives);
			}

			if(m_Instances)
			{
				for(S32 i = 0; i < pVisibleNodeList->m_StaticInstances.Size(); ++i)
				{
					C_VisibleNode& node = pVisibleNodeList->m_StaticInstances[i];
					node.m_SortKey = maxInt - node.m_VolatileNodeInfo.distance;
				}
				m_Sorter.Sort(pVisibleNodeList->m_StaticInstances);
			}

			if(m_ModelInstances)
			{
				for(S32 i = 0; i < pVisibleNodeList->m_ModelInstances.Size(); ++i)
				{
					C_VisibleNode& node = pVisibleNodeList->m_ModelInstances[i];
					node.m_SortKey = maxInt - node.m_VolatileNodeInfo.distance;
				}
				m_Sorter.Sort(pVisibleNodeList->m_ModelInstances);
			}
		}
		
		
		
		
		
	}
}

} // namespace SceneGraph

