#include <nexus_render/stdafx.h>
#include <nexus_render/Camera.h>
#include <nexus_render/TargetCamera.h>
#include <nexus_render/GizmoRenderer.h>

using namespace NexusRender;

Tu::Status GizmoRenderer::UpdateScene(U32 numRenderNodes, U32 nGizmoVerts)
{
	m_vbWrapper.Destroy();
	if (numRenderNodes == 0)
		return Tu::Status::SUCCESS;

	m_nGizmoVerts = nGizmoVerts;

	m_vbWrapper.Create(
		sizeof(D3DVertexPosCol), 
		numRenderNodes*nGizmoVerts, 
		nGizmoVerts);

	return Tu::Status::SUCCESS;
}

void GizmoRenderer::Clear()
{
	m_vbWrapper.Destroy();
}

Tu::Status GizmoRenderer::Update(const Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderNodes)
{
	if (renderNodes.Size() == 0)
		return Tu::Status::SUCCESS;

	Tu::Status status;

	m_vbWrapper.Lock();

	U32 indexInVB = 0;

	bool vbLocked = true;
	for (U32 iNode = 0; iNode < (U32)renderNodes.Size(); iNode++)
	{
		RenderNode* pRenderNode = renderNodes[iNode];

		//Only bother zeroing vertices for invisible nodes if they've not been culled.
		if (!pRenderNode->IsVisible() && (!pRenderNode->GetNodeFlags().isCulled))
		{
			for (U32 j = 0; j < m_nGizmoVerts; j++)
			{
				D3DVertexPosCol* pVertex = 
					(D3DVertexPosCol*)m_vbWrapper.GetVertexPtrByVertexIndex(indexInVB);
				pVertex->SetPosition(Maths::HVec3(0,0,0));
				indexInVB++;
			}
			continue;
		}

		ResetGizmo(
			indexInVB,
			m_nGizmoVerts,
			pRenderNode);

		indexInVB += m_nGizmoVerts;
	}
	m_vbWrapper.Unlock();

	return Tu::Status::SUCCESS;
}

void GizmoRenderer::Draw()
{
	m_vbWrapper.Draw(D3DPT_LINELIST);
}
