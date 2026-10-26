#ifndef DISPLAY_BUFFER_VERTEX_DECLARATIONS_H
#define DISPLAY_BUFFER_VERTEX_DECLARATIONS_H

#ifdef _MSC_VER
#pragma once
#endif

#ifndef PLATFORM_SPU
#include "Display/Buffer/VertexDeclaration.h"
#endif

namespace Display
{

#ifndef PLATFORM_SPU
void CreateVertexDeclarations();
#endif

struct Vertex_XY
{
	F32 x, y;

#ifndef PLATFORM_SPU
	static const U32			m_ElementCount = 2;
	static C_VertexElement		m_Elements[m_ElementCount];
	static C_VertexDeclaration*	m_pDeclaration;
#endif
};

struct Vertex_XY_UV
{
	F32 x, y;
	F32 u, v;

#ifndef PLATFORM_SPU
	static const U32			m_ElementCount = 3;
	static C_VertexElement		m_Elements[m_ElementCount];
	static C_VertexDeclaration*	m_pDeclaration;
#endif
};

struct Vertex_XY_C
{
	F32 x, y;
	U32 c;

#ifndef PLATFORM_SPU
	static const U32			m_ElementCount = 3;
	static C_VertexElement		m_Elements[m_ElementCount];
	static C_VertexDeclaration*	m_pDeclaration;
#endif
};

struct Vertex_XY_C_UV
{
	F32 x, y;
	U32 c;
	F32 u, v;

#ifndef PLATFORM_SPU
	static const U32			m_ElementCount = 4;
	static C_VertexElement		m_Elements[m_ElementCount];
	static C_VertexDeclaration*	m_pDeclaration;
#endif
};

struct Vertex_XY_C_UVW
{
	F32 x, y;
	U32 c;
	F32 u, v, w;

#ifndef PLATFORM_SPU
	static const U32			m_ElementCount = 4;
	static C_VertexElement		m_Elements[m_ElementCount];
	static C_VertexDeclaration*	m_pDeclaration;
#endif
};

struct Vertex_XY_HDR_UV
{
	F32 x, y;
	F32 r, g, b, a;
	F32 u, v;

#ifndef PLATFORM_SPU
	static const U32			m_ElementCount = 4;
	static C_VertexElement		m_Elements[m_ElementCount];
	static C_VertexDeclaration*	m_pDeclaration;
#endif
};

struct Vertex_XYZ
{
	F32 x, y, z;

#ifndef PLATFORM_SPU
	static const U32			m_ElementCount = 2;
	static C_VertexElement		m_Elements[m_ElementCount];
	static C_VertexDeclaration*	m_pDeclaration;
#endif
};

struct Vertex_XYZ_C
{
	F32 x, y, z;
	U32 c;

#ifndef PLATFORM_SPU
	static const U32			m_ElementCount = 3;
	static C_VertexElement		m_Elements[m_ElementCount];
	static C_VertexDeclaration*	m_pDeclaration;
#endif
};

struct Vertex_XYZ_UV
{
	F32 x, y, z;
	F32 u, v;

#ifndef PLATFORM_SPU
	static const U32			m_ElementCount = 3;
	static C_VertexElement		m_Elements[m_ElementCount];
	static C_VertexDeclaration*	m_pDeclaration;
#endif
};

struct Vertex_XYZ_UVW
{
	F32 x, y, z;
	F32 u, v, w;

#ifndef PLATFORM_SPU	
	static const U32			m_ElementCount = 3;
	static C_VertexElement		m_Elements[m_ElementCount];
	static C_VertexDeclaration*	m_pDeclaration;
#endif
};

struct Vertex_XYZ_SUV
{
	F32 x, y, z;
	S16 u, v;

#ifndef PLATFORM_SPU
	static const U32			m_ElementCount = 3;
	static C_VertexElement		m_Elements[m_ElementCount];
	static C_VertexDeclaration*	m_pDeclaration;
#endif
};

struct Vertex_XYZ_UV2
{
	F32 x, y, z;
	F32 u0, v0;
	F32 u1, v1;

#ifndef PLATFORM_SPU
	static const U32			m_ElementCount = 4;
	static C_VertexElement		m_Elements[m_ElementCount];
	static C_VertexDeclaration*	m_pDeclaration;
#endif
};

struct Vertex_XYZ_C_UV
{
	F32 x, y, z;
	U32 c;
	F32 u, v;

#ifndef PLATFORM_SPU
	static const U32			m_ElementCount = 4;
	static C_VertexElement		m_Elements[m_ElementCount];
	static C_VertexDeclaration*	m_pDeclaration;
#endif
};

struct Vertex_XYZ_C_UVW
{
	F32 x, y, z;
	U32 c;
	F32 u, v, w;

#ifndef PLATFORM_SPU
	static const U32			m_ElementCount = 4;
	static C_VertexElement		m_Elements[m_ElementCount];
	static C_VertexDeclaration*	m_pDeclaration;
#endif
};

struct Vertex_XYZ_HDR_UV
{
	F32 x, y, z;
	F32 r, g, b, a;
	F32 u, v;

#ifndef PLATFORM_SPU
	static const U32			m_ElementCount = 4;
	static C_VertexElement		m_Elements[m_ElementCount];
	static C_VertexDeclaration*	m_pDeclaration;
#endif
};

struct Vertex_XYZ_HDR_UVW
{
	F32 x, y, z;
	F32 r, g, b, a;
	F32 u, v, w;

#ifndef PLATFORM_SPU
	static const U32			m_ElementCount = 4;
	static C_VertexElement		m_Elements[m_ElementCount];
	static C_VertexDeclaration*	m_pDeclaration;
#endif
};

struct Vertex_XYZ_HDR_UV2
{
	F32 x, y, z;
	F32 r, g, b, a;
	F32 u0, v0, u1, v1;

#ifndef PLATFORM_SPU
	static const U32			m_ElementCount = 4;
	static C_VertexElement		m_Elements[m_ElementCount];
	static C_VertexDeclaration*	m_pDeclaration;
#endif
};

struct Vertex_XYZ_HDR_SUV
{
	F32 x, y, z;
	F32 r, g, b, a;
	S16 u, v;

#ifndef PLATFORM_SPU
	static const U32			m_ElementCount = 4;
	static C_VertexElement		m_Elements[m_ElementCount];
	static C_VertexDeclaration*	m_pDeclaration;
#endif
};

struct Vertex_XYZ_C_UV_N
{
	F32 x, y, z;
	U32 c;
	F32 u, v;
	F32 nx, ny, nz;

#ifndef PLATFORM_SPU
	static const U32			m_ElementCount = 5;
	static C_VertexElement		m_Elements[m_ElementCount];
	static C_VertexDeclaration*	m_pDeclaration;
#endif
};

struct Vertex_XYZ_HDR_UV_N
{
	F32 x, y, z;
	F32 r, g, b, a;
	F32 u, v;
	F32 nx, ny, nz;

#ifndef PLATFORM_SPU
	static const U32			m_ElementCount = 5;
	static C_VertexElement		m_Elements[m_ElementCount];
	static C_VertexDeclaration*	m_pDeclaration;
#endif
};

struct Vertex_XYZ_C_UV2
{
	F32 x, y, z;
	U32 c;
	F32 u0, v0;
	F32 u1, v1;

#ifndef PLATFORM_SPU
	static const U32			m_ElementCount = 5;
	static C_VertexElement		m_Elements[m_ElementCount];
	static C_VertexDeclaration*	m_pDeclaration;
#endif
};

struct Vertex_XYZ2_C_UV2
{
	F32 x0, y0, z0;
	F32 x1, y1, z1;
	U32 c;
	F32 u0, v0;
	F32 u1, v1;

#ifndef PLATFORM_SPU
	static const U32			m_ElementCount = 6;
	static C_VertexElement		m_Elements[m_ElementCount];
	static C_VertexDeclaration*	m_pDeclaration;
#endif
};

struct Vertex_XYZ_HDR_N2
{
	F32 x, y, z;
	F32 r, g, b, a;
	F32 nx0, ny0, nz0;
	F32 nx1, ny1, nz1;

#ifndef PLATFORM_SPU
	static const U32			m_ElementCount = 5;
	static C_VertexElement		m_Elements[m_ElementCount];
	static C_VertexDeclaration*	m_pDeclaration;
#endif
};

}; // namespace Display

#endif // DISPLAY_BUFFER_VERTEX_DECLARATIONS_H
