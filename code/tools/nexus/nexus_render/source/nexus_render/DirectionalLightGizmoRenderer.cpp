#include <nexus_render/stdafx.h>
#include <nexus_render/Camera.h>
#include <nexus_render/TargetCamera.h>
#include <nexus_render/GizmoRenderer.h>
#include <nexus_render/DirectionalLightGizmoRenderer.h>
#include <nexus_render/AftkUtility.h>

using namespace NexusRender;

Tu::Status DirectionalLightGizmoRenderer::UpdateScene(U32 numRenderNodes)
{
	return GizmoRenderer::UpdateScene(
		numRenderNodes, 
		NEXUS_RENDER_PRIMITIVES_STAR_VERTS + 3*NEXUS_RENDER_PRIMITIVES_LINE_VERTS);
}

void DirectionalLightGizmoRenderer::ResetGizmo(
	U32 vbOffset,
	U32 nGizmoVerts,
	RenderNode* pRenderNode)
{
	Maths::HMat44 gizmoMatrix;
	gizmoMatrix = pRenderNode->GetWorldMatrix();
//	gizmoMatrix = pRenderNode->GetGizmoMatrix(pRenderNode->GetWorldMatrix());
//	gizmoMatrix = pRenderNode->GetGizmoMatrix(pRenderNode->GetWorldMatrix(), 4);

	Aftk::C_SceneLightNode* pLight = 
		(Aftk::C_SceneLightNode*)AftkUtility::GetIndirectSourcePtr(pRenderNode->GetAftkSceneNodePtr());
	const Aftk::Vector4F32& colour = pLight->GetColour();

	D3DVertexPosCol* pVertex = 
		(D3DVertexPosCol*)m_vbWrapper.GetVertexPtrByVertexIndex(vbOffset);
	
	F32 radius = 0.8f;
//	F32 radius = 0.01f;
	F32 arrowWidth = radius / 0.5f;
	F32 arrowHeight = radius / 0.3f;
	F32 offset = radius / 1.5f;
	F32 arrowCapRatio = 0.3f;
//	F32 radius = 0.0015f;
//	F32 arrowWidth = 0.003f;
//	F32 arrowHeight = 0.005f;
//	F32 offset = 0.001f;

	Primitives::DrawStar(
		pVertex,
		gizmoMatrix,
		radius,
		Maths::HVec4(colour[0],colour[1],colour[2],colour[3]));
	pVertex += NEXUS_RENDER_PRIMITIVES_STAR_VERTS;

	Primitives::DrawLine(
		pVertex,
		gizmoMatrix,
		Maths::HVec3(0,0,0),
		Maths::HVec3(0,0,-arrowHeight),
		Maths::HVec4(colour[0],colour[1],colour[2],colour[3]));
	pVertex += NEXUS_RENDER_PRIMITIVES_LINE_VERTS;

	//Draw arrow side 1
	Primitives::DrawLine(
		pVertex,
		gizmoMatrix,
		Maths::HVec3(0,0,-arrowHeight),
		Maths::HVec3(offset,0,-arrowHeight+arrowHeight*arrowCapRatio),
		Maths::HVec4(colour[0],colour[1],colour[2],colour[3]));
	pVertex += NEXUS_RENDER_PRIMITIVES_LINE_VERTS;

	//Draw arrow side 2
	Primitives::DrawLine(
		pVertex,
		gizmoMatrix,
		Maths::HVec3(0,0,-arrowHeight),
		Maths::HVec3(-offset,0,-arrowHeight+arrowHeight*arrowCapRatio),
		Maths::HVec4(colour[0],colour[1],colour[2],colour[3]));
}

