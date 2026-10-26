#include <nexus_render/stdafx.h>
#include <nexus_render/Camera.h>
#include <nexus_render/TargetCamera.h>
#include <nexus_render/GizmoRenderer.h>
#include <nexus_render/SpotLightGizmoRenderer.h>
#include <nexus_render/AftkUtility.h>
#include <nexus_render/Utility.h>

using namespace NexusRender;

Tu::Status SpotLightGizmoRenderer::UpdateScene(U32 numRenderNodes)
{
	return GizmoRenderer::UpdateScene(
		numRenderNodes, 
		NEXUS_RENDER_PRIMITIVES_CONE_VERTS);
}

void SpotLightGizmoRenderer::ResetGizmo(
	U32 vbOffset,
	U32 nGizmoVerts,
	RenderNode* pRenderNode)
{
	Maths::HMat44 gizmoMatrix;
	gizmoMatrix = pRenderNode->GetWorldMatrix();
	//gizmoMatrix = pRenderNode->GetGizmoMatrix(pRenderNode->GetWorldMatrix(), 4);

	Aftk::C_SceneSpotLightNode* pLight = 
		(Aftk::C_SceneSpotLightNode*)AftkUtility::GetIndirectSourcePtr(
			pRenderNode->GetAftkSceneNodePtr());
	const Aftk::Vector4F32& colour = pLight->GetColour();

	D3DVertexPosCol* pVertex = 
		(D3DVertexPosCol*)m_vbWrapper.GetVertexPtrByVertexIndex(vbOffset);

	F32 radius = tanf(pLight->GetConeAngle()) * .5f;

	#pragma message("Need to add utility code in the aftk to apply world scale to radius.")

//	Aftk::Vector3F32 scale;
//	pLight->GetScale(scale);

//	radius *= (scale[0] + scale[1] + scale[2]) * 0.33f

	//F32 height = -0.0060f;
	F32 height = -1.3f;

	Maths::HVec4 zAxis = gizmoMatrix.AZ();
	F32 movement = height;
	Maths::HVec4 trans = gizmoMatrix.AW();
	trans = trans + (zAxis * movement);
	gizmoMatrix.AW(trans);

	Primitives::DrawCone(
		pVertex,
		gizmoMatrix,
		radius,
		-height,
		Maths::HVec4(colour[0],colour[1],colour[2],colour[3]));
}

