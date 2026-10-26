#include <nexus_render/stdafx.h>
#include <nexus_render/Camera.h>
#include <nexus_render/TargetCamera.h>
#include <nexus_render/GizmoRenderer.h>
#include <nexus_render/AreaLightGizmoRenderer.h>
#include <nexus_render/AftkUtility.h>
#include <nexus_render/Utility.h>

using namespace NexusRender;

Tu::Status AreaLightGizmoRenderer::UpdateScene(U32 numRenderNodes)
{
	return GizmoRenderer::UpdateScene(
		numRenderNodes, 
		NEXUS_RENDER_PRIMITIVES_LINE_VERTS*7);
}

void AreaLightGizmoRenderer::ResetGizmo(
	U32 vbOffset,
	U32 nGizmoVerts,
	RenderNode* pRenderNode)
{
	Maths::HMat44 gizmoMatrix;
	gizmoMatrix = pRenderNode->GetWorldMatrix();
	//gizmoMatrix = pRenderNode->GetGizmoMatrix(pRenderNode->GetWorldMatrix(), 4);

	Aftk::C_SceneSpotLightNode* pLight = 
		(Aftk::C_SceneSpotLightNode*)AftkUtility::GetIndirectSourcePtr(pRenderNode->GetAftkSceneNodePtr());
	const Aftk::Vector4F32& colour = pLight->GetColour();

	D3DVertexPosCol* pVertex = 
		(D3DVertexPosCol*)m_vbWrapper.GetVertexPtrByVertexIndex(vbOffset);

	F32 widthHeight = 0.8f;

	//Bottom
	Primitives::DrawLine(
		pVertex,
		gizmoMatrix,
		Maths::HVec3(widthHeight,-widthHeight,0),
		Maths::HVec3(-widthHeight,-widthHeight,0),
		Maths::HVec4(colour[0],colour[1],colour[2],colour[3]));
	pVertex += NEXUS_RENDER_PRIMITIVES_LINE_VERTS;

	//Top
	Primitives::DrawLine(
		pVertex,
		gizmoMatrix,
		Maths::HVec3(widthHeight,widthHeight,0),
		Maths::HVec3(-widthHeight,widthHeight,0),
		Maths::HVec4(colour[0],colour[1],colour[2],colour[3]));
	pVertex += NEXUS_RENDER_PRIMITIVES_LINE_VERTS;

	//Left
	Primitives::DrawLine(
		pVertex,
		gizmoMatrix,
		Maths::HVec3(-widthHeight,-widthHeight,0),
		Maths::HVec3(-widthHeight,widthHeight,0),
		Maths::HVec4(colour[0],colour[1],colour[2],colour[3]));
	pVertex += NEXUS_RENDER_PRIMITIVES_LINE_VERTS;

	//Right
	Primitives::DrawLine(
		pVertex,
		gizmoMatrix,
		Maths::HVec3(widthHeight,-widthHeight,0),
		Maths::HVec3(widthHeight,widthHeight,0),
		Maths::HVec4(colour[0],colour[1],colour[2],colour[3]));
	pVertex += NEXUS_RENDER_PRIMITIVES_LINE_VERTS;

	//Cross 1
	Primitives::DrawLine(
		pVertex,
		gizmoMatrix,
		Maths::HVec3(-widthHeight,-widthHeight,0),
		Maths::HVec3(widthHeight,widthHeight,0),
		Maths::HVec4(colour[0],colour[1],colour[2],colour[3]));
	pVertex += NEXUS_RENDER_PRIMITIVES_LINE_VERTS;

	//Cross 2
	Primitives::DrawLine(
		pVertex,
		gizmoMatrix,
		Maths::HVec3(-widthHeight,widthHeight,0),
		Maths::HVec3(widthHeight,-widthHeight,0),
		Maths::HVec4(colour[0],colour[1],colour[2],colour[3]));
	pVertex += NEXUS_RENDER_PRIMITIVES_LINE_VERTS;

	//Direction
	Primitives::DrawLine(
		pVertex,
		gizmoMatrix,
		Maths::HVec3(0,0,0),
		Maths::HVec3(0,0,-widthHeight),
		Maths::HVec4(colour[0],colour[1],colour[2],colour[3]));
}

