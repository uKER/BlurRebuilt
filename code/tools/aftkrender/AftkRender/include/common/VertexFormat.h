#ifndef AFTK_RENDER_VERTEX_FORMAT_H
#define AFTK_RENDER_VERTEX_FORMAT_H

#ifdef _MSC_VER
#pragma once
#endif

namespace AftkRender
{
	void CreateAftkRenderVertexDeclarations();

	struct Vertex_XYZ2
	{
		F32 x0, y0, z0;
		F32 x1, y1, z1;

		static const U32						m_ElementCount = 3;
		static Display::C_VertexElement			m_Elements[m_ElementCount];
		static Display::C_VertexDeclaration*	m_pDeclaration;
	};

	struct Vertex_XYZ2_C
	{
		F32 x0, y0, z0;
		F32 x1, y1, z1;
		U8 b, g, r, a;

		static const U32						m_ElementCount = 4;
		static Display::C_VertexElement			m_Elements[m_ElementCount];
		static Display::C_VertexDeclaration*	m_pDeclaration;
	};

	struct Vertex_UV
	{
		F32 u, v;

		static const U32						m_ElementCount = 2;
		static Display::C_VertexElement			m_Elements[m_ElementCount];
		static Display::C_VertexDeclaration*	m_pDeclaration;
	};

	struct Vertex_W4_Idx4
	{
		F32 w[4];
		U8 idx[4];

		static const U32						m_ElementCount = 3;
		static Display::C_VertexElement			m_Elements[m_ElementCount];
		static Display::C_VertexDeclaration*	m_pDeclaration;

	};



}

#endif