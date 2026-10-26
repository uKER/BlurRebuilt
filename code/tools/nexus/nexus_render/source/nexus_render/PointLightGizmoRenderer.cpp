#include <nexus_render/stdafx.h>
#include <nexus_render/Camera.h>
#include <nexus_render/TargetCamera.h>
#include <nexus_render/GizmoRenderer.h>
#include <nexus_render/PointLightGizmoRenderer.h>
#include <nexus_render/AftkUtility.h>

using namespace NexusRender;

Tu::Status PointLightGizmoRenderer::UpdateScene(U32 numRenderNodes)
{
	return GizmoRenderer::UpdateScene(
		numRenderNodes, 
		NEXUS_RENDER_PRIMITIVES_STAR_VERTS);
}

void PointLightGizmoRenderer::ResetGizmo(
	U32 vbOffset,
	U32 nGizmoVerts,
	RenderNode* pRenderNode)
{
	Maths::HMat44 gizmoMatrix;
//	gizmoMatrix = pRenderNode->GetGizmoMatrix(pRenderNode->GetWorldMatrix());
	gizmoMatrix = pRenderNode->GetWorldMatrix();

	Aftk::C_SceneLightNode* pLight = 
		(Aftk::C_SceneLightNode*)AftkUtility::GetIndirectSourcePtr(pRenderNode->GetAftkSceneNodePtr());
	const Aftk::Vector4F32& colour = pLight->GetColour();

	D3DVertexPosCol* pVertex = 
		(D3DVertexPosCol*)m_vbWrapper.GetVertexPtrByVertexIndex(vbOffset);

	Primitives::DrawStar(
		pVertex,
		gizmoMatrix,
		0.8f, //0.0015f,
		Maths::HVec4(colour[0],colour[1],colour[2],colour[3]));
}

