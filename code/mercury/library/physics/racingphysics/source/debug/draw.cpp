#include "stdafx.h"
#include "racingphysics/debug/draw.h"


namespace RacingPhysics
{

DrawPrimitive DrawPrimitive::s_instance;

void DrawPrimitive::Begin(Display::E_PrimitiveType primitiveType)
{
	m_primitiveType = primitiveType;
	RP_ASSERT(m_vertices.Empty());
}

void DrawPrimitive::End()
{
	if (m_vertices.Empty())
		return;

	Display::Device()->SetVertexDeclaration(Display::Vertex_XYZ_C_UV::m_pDeclaration);

//#ifdef PLATFORM_XENON
//	Display::Device()->GetD3DDevice()->DrawVerticesUP(Display::D3D9PrimitiveType(m_primitiveType), 
//														m_vertices.Size(), 
//														&m_vertices[0], sizeof(Display::Vertex_XYZ_C_UV));
//#endif

	Display::Vertex_XYZ_C_UV* pVertex = (Display::Vertex_XYZ_C_UV*) Display::Device()->BeginVertices(
							m_primitiveType, m_vertices.Size(), sizeof(Display::Vertex_XYZ_C_UV), Display::Vertex_XYZ_C_UV::m_pDeclaration, true); // This can fail

	if (pVertex)
	{
		for (int i = 0; i < m_vertices.Size(); i++)
		{
			*pVertex = m_vertices[i];
			pVertex++;
		}

		Display::Device()->EndVertices(pVertex);
	}

	m_vertices.Clear();
}

}
