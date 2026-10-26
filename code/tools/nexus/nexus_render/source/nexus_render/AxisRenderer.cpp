#include <nexus_render/stdafx.h>
#include <nexus_render/Camera.h>
#include <nexus_render/TargetCamera.h>
#include <nexus_render/GizmoRenderer.h>
#include <nexus_render/AxisRenderer.h>

using namespace NexusRender;

#define NEXUS_RENDER_AXIS_RENDERER_NUM_VERTS_PER_NODE	22
#define NEXUS_RENDER_AXIS_RENDERER_SCALE				0.0015f

Tu::Status AxisRenderer::UpdateScene(U32 numRenderNodes)
{
	return GizmoRenderer::UpdateScene(numRenderNodes, NEXUS_RENDER_AXIS_RENDERER_NUM_VERTS_PER_NODE);
}

void AxisRenderer::ResetGizmo(
	U32 vbOffset,
	U32 nGizmoVerts,
	RenderNode* pRenderNode)
{
	Maths::HMat44 gizmoMatrix;
	gizmoMatrix.SetIdentity();

	gizmoMatrix = pRenderNode->GetGizmoMatrix(pRenderNode->GetWorldMatrix());

	// initialise the static vertex buffer for matrix render
	Maths::HVec3 p;

	D3DVertexPosCol* pVertex = 
		(D3DVertexPosCol*)m_vbWrapper.GetVertexPtrByVertexIndex(vbOffset);

	//v0
	// z axis 'y' arrowhead
	p = Maths::HVec3(0.0f, 0.0f, 1.0f * NEXUS_RENDER_AXIS_RENDERER_SCALE);
	p = p * gizmoMatrix;
	pVertex->SetPosition(p);
	pVertex->SetColour(0,0,255,255);
	pVertex++;

	//v1
	p = Maths::HVec3(0.0f, 0.2f * NEXUS_RENDER_AXIS_RENDERER_SCALE, 0.7f  * NEXUS_RENDER_AXIS_RENDERER_SCALE);
	p = p * gizmoMatrix;
	pVertex->SetPosition(p);
	pVertex->SetColour(0,0,255,255);
	pVertex++;

	//v2
	p = Maths::HVec3(0.0f, 0.0f, 1.0f * NEXUS_RENDER_AXIS_RENDERER_SCALE);
	p = p * gizmoMatrix;
	pVertex->SetPosition(p);
	pVertex->SetColour(0,0,255,255);
	pVertex++;

	//v3
	p = Maths::HVec3(0.0f, -0.2f * NEXUS_RENDER_AXIS_RENDERER_SCALE, 0.7f * NEXUS_RENDER_AXIS_RENDERER_SCALE);
	p = p * gizmoMatrix;
	pVertex->SetPosition(p);
	pVertex->SetColour(0,0,255,255);
	pVertex++;

	//v4
	// z axis 'x' arrowhead
	p = Maths::HVec3(0.0f, 0.0f, 1.0f * NEXUS_RENDER_AXIS_RENDERER_SCALE);
	p = p * gizmoMatrix;
	pVertex->SetPosition(p);
	pVertex->SetColour(0,0,255,255);
	pVertex++;

	//v5
	p = Maths::HVec3(0.2f * NEXUS_RENDER_AXIS_RENDERER_SCALE, 0.0f, 0.7f * NEXUS_RENDER_AXIS_RENDERER_SCALE);
	p = p * gizmoMatrix;
	pVertex->SetPosition(p);
	pVertex->SetColour(0,0,255,255);
	pVertex++;

	//v6
	p = Maths::HVec3(0.0f, 0.0f, 1.0f * NEXUS_RENDER_AXIS_RENDERER_SCALE);
	p = p * gizmoMatrix;
	pVertex->SetPosition(p);
	pVertex->SetColour(0,0,255,255);
	pVertex++;

	//v7
	p = Maths::HVec3(-0.2f * NEXUS_RENDER_AXIS_RENDERER_SCALE, 0.0f, 0.7f * NEXUS_RENDER_AXIS_RENDERER_SCALE);
	p = p * gizmoMatrix;
	pVertex->SetPosition(p);
	pVertex->SetColour(0,0,255,255);
	pVertex++;

	//v8
	//////////
	// x axis
	//////////
	p = Maths::HVec3(0.0f, 0.0f, 0.0f);
	p = p * gizmoMatrix;
	pVertex->SetPosition(p);
	pVertex->SetColour(255,0,0,255);
	pVertex++;

	//v9
	p = Maths::HVec3(1.0f * NEXUS_RENDER_AXIS_RENDERER_SCALE, 0.0f, 0.0f);
	p = p * gizmoMatrix;
	pVertex->SetPosition(p);
	pVertex->SetColour(255,0,0,255);
	pVertex++;

	//v10
	//////////
	// y axis
	//////////
	p = Maths::HVec3(0.0f, 0.0f, 0.0f);
	p = p * gizmoMatrix;
	pVertex->SetPosition(p);
	pVertex->SetColour(0,255,0,255);
	pVertex++;

	//v11
	p = Maths::HVec3(0.0f, 1.0f * NEXUS_RENDER_AXIS_RENDERER_SCALE, 0.0f);
	p = p * gizmoMatrix;
	pVertex->SetPosition(p);
	pVertex->SetColour(0,255,0,255);
	pVertex++;

	//v12
	//////////
	// y axis
	//////////
	vbOffset++;
	p = Maths::HVec3(0.0f, 0.0f, 0.0f);
	p = p * gizmoMatrix;
	pVertex->SetPosition(p);
	pVertex->SetColour(0,0,255,255);
	pVertex++;

	//v13
	p = Maths::HVec3(0.0f, 0.0f, 1.0f * NEXUS_RENDER_AXIS_RENDERER_SCALE);
	p = p * gizmoMatrix;
	pVertex->SetPosition(p);
	pVertex->SetColour(0,0,255,255);
	pVertex++;

	//v14
	// y axis 'x' arrowhead
	p = Maths::HVec3(0.0f, 1.0f * NEXUS_RENDER_AXIS_RENDERER_SCALE, 0.0f);
	p = p * gizmoMatrix;
	pVertex->SetPosition(p);
	pVertex->SetColour(0,255,0,255);
	pVertex++;

	//v15
	p = Maths::HVec3(0.2f * NEXUS_RENDER_AXIS_RENDERER_SCALE, 0.7f * NEXUS_RENDER_AXIS_RENDERER_SCALE, 0.0f);
	p = p * gizmoMatrix;
	pVertex->SetPosition(p);
	pVertex->SetColour(0,255,0,255);
	pVertex++;

	//v16
	p = Maths::HVec3(0.0f, 1.0f * NEXUS_RENDER_AXIS_RENDERER_SCALE, 0.0f);
	p = p * gizmoMatrix;
	pVertex->SetPosition(p);
	pVertex->SetColour(0,255,0,255);
	pVertex++;

	//v17
	p = Maths::HVec3(-0.2f * NEXUS_RENDER_AXIS_RENDERER_SCALE, 0.7f * NEXUS_RENDER_AXIS_RENDERER_SCALE, 0.0f);
	p = p * gizmoMatrix;
	pVertex->SetPosition(p);
	pVertex->SetColour(0,255,0,255);
	pVertex++;

	//v18
	// x axis 'x' arrowhead
	p = Maths::HVec3(1.0f * NEXUS_RENDER_AXIS_RENDERER_SCALE, 0.0f, 0.0f);
	p = p * gizmoMatrix;
	pVertex->SetPosition(p);
	pVertex->SetColour(255,0,0,255);
	pVertex++;

	//v19
	p = Maths::HVec3(0.7f * NEXUS_RENDER_AXIS_RENDERER_SCALE, 0.0f , 0.2f * NEXUS_RENDER_AXIS_RENDERER_SCALE);
	p = p * gizmoMatrix;
	pVertex->SetPosition(p);
	pVertex->SetColour(255,0,0,255);
	pVertex++;

	//v20
	p = Maths::HVec3(1.0f * NEXUS_RENDER_AXIS_RENDERER_SCALE, 0.0f, 0.0f);
	p = p * gizmoMatrix;
	pVertex->SetPosition(p);
	pVertex->SetColour(255,0,0,255);
	pVertex++;

	//v21
	p = Maths::HVec3(0.7f * NEXUS_RENDER_AXIS_RENDERER_SCALE, 0.0f , -0.2f * NEXUS_RENDER_AXIS_RENDERER_SCALE);
	p = p * gizmoMatrix;
	pVertex->SetPosition(p);
	pVertex->SetColour(255,0,0,255);
	pVertex++;
}

