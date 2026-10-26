#include "AftkRenderPch.h"
#include "Render/GridRenderer.h"

#include "Display/Buffer/VertexDeclarations.h"

using namespace AftkRender;


template<typename T>
void SetVertex(T* pVertex, F32 x, F32 y, F32 z, U32 c)
{
	pVertex->x = x;
	pVertex->y = y;
	pVertex->z = z;
	pVertex->c = c;
}


void C_GridRenderer::Render(U32 cellsU, U32 cellsV, F32 spacing, const Maths::RGBA& col, C_GridRenderer::E_GridPlane plane)
{
	typedef UtMaths::MVector<UtMaths::VectorXYZ<F32> > Vector3;

	if (!Display::BasicRenderExists())
		return;

	Display::BasicRender()->BindColoured3D();

	U32 nVertices = 2*(cellsU+cellsV+2)+4;
	Display::Vertex_XYZ_C *pVertex = (Display::Vertex_XYZ_C*) Display::Device()->BeginVertices(
		Display::E_PrimitiveType_LineList, 
		nVertices, 
		sizeof(Display::Vertex_XYZ_C), 
		Display::Vertex_XYZ_C::m_pDeclaration, true);

	if (!pVertex)
	{
		//HACK:
		//We'll need to call Display::C_PCD3D9Device::Present() if we're rendering from a swap chain (see C_ViewContext::EndRender()).
		//Display::C_PCD3D9Device's dynamic vertex buffer is swapped in this call, as well as 
		//Display::C_PCD3D9Device::m_UserBeginVerticesUsage & Display::C_PCD3D9Device::m_BeginVerticesUsage being reset
		Display::Device()->Present();
		return;
	}

	F32 halfU = spacing*cellsU*0.5f;
	F32 halfV = spacing*cellsV*0.5f;


	Vector3 p0, p1, p2, p3, dU, dV, a0, a1, a2, a3;
	switch (plane)
	{
	case C_GridRenderer::E_GridPlane_XY:
		p0 = Vector3(-halfU, -halfV, 0.f);
		p1 = Vector3(-halfU, halfV, 0.f);
		p2 = Vector3(-halfU, -halfV, 0.f);
		p3 = Vector3( halfU, -halfV, 0.f);
		dU = Vector3(spacing, 0.f, 0.f);
		dV = Vector3(0.f, spacing, 0.f);
		a0 = Vector3(-halfU, 0.f, 0.f);
		a1 = Vector3(halfU, 0.f, 0.f);
		a2 = Vector3(0.f, -halfV, 0.f);
		a3 = Vector3(0.f, halfV, 0.f);
		break;
	
	case C_GridRenderer::E_GridPlane_YZ:
		p0 = Vector3(0.f, -halfU, -halfV);
		p1 = Vector3(0.f, -halfU,  halfV);
		p2 = Vector3(0.f, -halfU, -halfV);
		p3 = Vector3(0.f,  halfU, -halfV);
		dU = Vector3(0.f, spacing, 0.f);
		dV = Vector3(0.f, 0.f, spacing);
		a0 = Vector3(0.f, -halfU, 0.f);
		a1 = Vector3(0.f, halfU, 0.f);
		a2 = Vector3(0.f, 0.f, -halfV);
		a3 = Vector3(0.f, 0.f, halfV);
		break;

	case C_GridRenderer::E_GridPlane_XZ:
	default:
		p0 = Vector3(-halfU, 0.f, -halfV);
		p1 = Vector3(-halfU, 0.f,  halfV);
		p2 = Vector3(-halfU, 0.f, -halfV);
		p3 = Vector3( halfU, 0.f, -halfV);
		dU = Vector3(spacing, 0.f, 0.f);
		dV = Vector3(0.f, 0.f, spacing);
		a0 = Vector3(-halfU, 0.f, 0.f);
		a1 = Vector3(halfU, 0.f, 0.f);
		a2 = Vector3(0.f, 0.f, -halfV);
		a3 = Vector3(0.f, 0.f, halfV);
		break;
	}

	for (U32 i=0; i<cellsU+1; ++i)
	{
		SetVertex(pVertex, p0[0], p0[1], p0[2], col.Colour());
		pVertex++;
		SetVertex(pVertex, p1[0], p1[1], p1[2], col.Colour());
		pVertex++;

		p0 += dU;
		p1 += dU;
	}

	for (U32 i=0; i<cellsV+1; ++i)
	{
		SetVertex(pVertex, p2[0], p2[1], p2[2], col.Colour());
		pVertex++;
		SetVertex(pVertex, p3[0], p3[1], p3[2], col.Colour());
		pVertex++;

		p2 += dV;
		p3 += dV;	
	}

	//axii
	SetVertex(pVertex, a0[0], a0[1], a0[2], 0xff000000);
	pVertex++;
	SetVertex(pVertex, a1[0], a1[1], a1[2], 0xff000000);
	pVertex++;

	SetVertex(pVertex, a2[0], a2[1], a2[2], 0xff000000);
	pVertex++;
	SetVertex(pVertex, a3[0], a3[1], a3[2], 0xff000000);
	pVertex++;

	Display::Device()->EndVertices(pVertex);

}

void C_GridRenderer::Render(const Maths::HVec2& minUV, const Maths::HVec2& maxUV, const Maths::RGBA& col, F32 spacing, F32 offset, E_GridPlane plane)
{
	typedef UtMaths::MVector<UtMaths::VectorXYZ<F32> > Vector3;

	if (!Display::BasicRenderExists())
		return;

	Display::BasicRender()->BindColoured3D();

	assert (maxUV[0] >= minUV[0]);
	assert (maxUV[1] >= minUV[1]);

	F32 u0 = Maths::Ceil(minUV[0]/spacing)*spacing;
	F32 v0 = Maths::Ceil(minUV[1]/spacing)*spacing;
	F32 u1 = Maths::Floor(maxUV[0]/spacing)*spacing;
	F32 v1 = Maths::Floor(maxUV[1]/spacing)*spacing;


	S32 cellsU = (S32)((u1-u0)/spacing);
	S32 cellsV = (S32)((v1-v0)/spacing);

	if (cellsU > 1000 || cellsV > 1000)
		//only render axes
		cellsU = cellsV = 0;

	U32 nVertices = 2*(cellsU+cellsV+2)+4;
	Display::Vertex_XYZ_C *pVertex = 
		(Display::Vertex_XYZ_C*) Display::Device()->BeginVertices(
		Display::E_PrimitiveType_LineList, 
		nVertices, 
		sizeof(Display::Vertex_XYZ_C), 
		Display::Vertex_XYZ_C::m_pDeclaration, 
		true);

	if (!pVertex)
	{
		//HACK:
		//We'll need to call Display::C_PCD3D9Device::Present() if we're rendering from a swap chain (see C_ViewContext::EndRender()).
		//Display::C_PCD3D9Device's dynamic vertex buffer is swapped in this call, as well as 
		//Display::C_PCD3D9Device::m_UserBeginVerticesUsage & Display::C_PCD3D9Device::m_BeginVerticesUsage being reset
		Display::Device()->Present();
		return;
	}

	Vector3 p0, p1, p2, p3, dU, dV, a0, a1, a2, a3;
	switch (plane)
	{
	case C_GridRenderer::E_GridPlane_XY:
		p0 = Vector3(u0, v0, offset);
		p1 = Vector3(u0, v1, offset);
		p2 = Vector3(u0, v0, offset);
		p3 = Vector3(u1, v0, offset);
		dU = Vector3(spacing, 0.f, 0.f);
		dV = Vector3(0.f, spacing, 0.f);
		a0 = Vector3(minUV[0], 0.f, offset);
		a1 = Vector3(maxUV[0], 0.f, offset);
		a2 = Vector3(0.f, minUV[1], offset);
		a3 = Vector3(0.f, maxUV[1], offset);
		break;
	
	case C_GridRenderer::E_GridPlane_YZ:
		p0 = Vector3(offset, u0, v0);
		p1 = Vector3(offset, u0, v1);
		p2 = Vector3(offset, u0, v0);
		p3 = Vector3(offset, u1, v0);
		dU = Vector3(0.f, spacing, 0.f);
		dV = Vector3(0.f, 0.f, spacing);
		a0 = Vector3(offset, minUV[0], 0.f);
		a1 = Vector3(offset, maxUV[0], 0.f);
		a2 = Vector3(offset, 0.f, minUV[1]);
		a3 = Vector3(offset, 0.f, maxUV[1]);
		break;

	case C_GridRenderer::E_GridPlane_XZ:
	default:
		p0 = Vector3(u0, offset, v0);
		p1 = Vector3(u0, offset, v1);
		p2 = Vector3(u0, offset, v0);
		p3 = Vector3(u1, offset, v0);
		dU = Vector3(spacing, 0.f, 0.f);
		dV = Vector3(0.f, 0.f, spacing);
		a0 = Vector3(minUV[0], offset, 0.f);
		a1 = Vector3(maxUV[0], offset, 0.f);
		a2 = Vector3(0.f, offset, minUV[1]);
		a3 = Vector3(0.f, offset, maxUV[1]);
		break;
	}

	for (S32 i=0; i<cellsU+1; ++i)
	{
		SetVertex(pVertex, p0[0], p0[1], p0[2], col.Colour());
		pVertex++;
		SetVertex(pVertex, p1[0], p1[1], p1[2], col.Colour());
		pVertex++;

		p0 += dU;
		p1 += dU;
	}

	for (S32 i=0; i<cellsV+1; ++i)
	{
		SetVertex(pVertex, p2[0], p2[1], p2[2], col.Colour());
		pVertex++;
		SetVertex(pVertex, p3[0], p3[1], p3[2], col.Colour());
		pVertex++;

		p2 += dV;
		p3 += dV;	
	}

	//axii
	SetVertex(pVertex, a0[0], a0[1], a0[2], 0xff000000);
	pVertex++;
	SetVertex(pVertex, a1[0], a1[1], a1[2], 0xff000000);
	pVertex++;

	SetVertex(pVertex, a2[0], a2[1], a2[2], 0xff000000);
	pVertex++;
	SetVertex(pVertex, a3[0], a3[1], a3[2], 0xff000000);
	pVertex++;

	Display::Device()->EndVertices(pVertex);
}