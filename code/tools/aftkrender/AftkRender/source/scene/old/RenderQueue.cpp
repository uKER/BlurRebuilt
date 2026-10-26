#include "stdafx.h"
#include "AftkRender.h"

#include <algorithm>

using namespace AftkRender;


C_RenderTask::C_RenderTask()
:
	m_LodLevel(0),
	m_pRenderMeshNode(0),
	m_RenderMeshClusterIndex(0)
{
}


C_RenderTask::C_RenderTask(
	U8 lod,
	I_RenderMeshNode* pRenderMeshNode,
	U32 cluster)
:
	m_LodLevel(lod),
	m_pRenderMeshNode(pRenderMeshNode),
	m_RenderMeshClusterIndex(cluster)
{
}


C_RenderTask::~C_RenderTask()
{
}

U8 C_RenderTask::GetLodLevel() const
{
	return m_LodLevel;
}

void C_RenderTask::SetLodLevel(U8 lodlevel)
{
	m_LodLevel = lodlevel;
}

I_RenderMeshNode* C_RenderTask::GetRenderMeshNode() const
{
	return m_pRenderMeshNode;
}

void C_RenderTask::SetRenderMeshNode(I_RenderMeshNode* pRenderMeshNode)
{
	m_pRenderMeshNode = pRenderMeshNode;
}

C_RenderMeshCluster* C_RenderTask::GetRenderMeshClusterPtr() const
{
	if (!m_pRenderMeshNode)
		return 0;

	if (m_pRenderMeshNode->GetType() == E_RenderNodeType_Mesh)
		return ((C_RenderMeshNode*)m_pRenderMeshNode)->GetMeshClusterPtr(m_LodLevel, m_RenderMeshClusterIndex);
	else if (m_pRenderMeshNode->HasType(E_RenderNodeType_Mesh_Instance))
	{
		C_RenderMeshNode* pSource = ((C_RenderMeshInstanceNode*)m_pRenderMeshNode)->GetSourceNodePtr();
		if (pSource)
			return pSource->GetMeshClusterPtr(m_LodLevel, m_RenderMeshClusterIndex);
	}

	return 0;
}

U32 C_RenderTask::GetRenderMeshClusterIndex() const
{
	return m_RenderMeshClusterIndex;
}

void C_RenderTask::SetRenderMeshClusterIndex(U32 clusterIndex)
{
	m_RenderMeshClusterIndex = clusterIndex;
}


C_RenderQueue::C_RenderQueue()
{
}

C_RenderQueue::~C_RenderQueue()
{
}

void C_RenderQueue::PushTask(const C_RenderTask& task)
{
	m_RenderTasks.PushBack(task);
}

void C_RenderQueue::Clear()
{
	m_RenderTasks.Clear();
}

U32 C_RenderQueue::GetSize() const
{
	return m_RenderTasks.Size();
}

void C_RenderQueue::Sort()
{
	if (m_RenderTasks.Size())
		std::sort(&m_RenderTasks[0], &m_RenderTasks[m_RenderTasks.Size()-1], C_RenderTaskFrontToBackSort());
}

void C_RenderQueue::SortReverse()
{
	if (m_RenderTasks.Size())
		std::sort(&m_RenderTasks[0], &m_RenderTasks[m_RenderTasks.Size()-1], C_RenderTaskBackToFrontSort());
}