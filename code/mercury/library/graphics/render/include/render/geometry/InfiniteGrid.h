#ifndef RENDER_GEOMETRY_INFINITE_GRID_H
#define RENDER_GEOMETRY_INFINITE_GRID_H

#if _MSC_VER > 1000
#pragma once
#endif

namespace Render
{
namespace Geometry
{

class C_InfiniteGrid
{

public:

	C_InfiniteGrid();
	~C_InfiniteGrid();

	bool Create();
	void Destroy();
	void Render();

public:

	U32	m_iWidth;
	U32	m_iHeight;
	F32	m_fHeight;

private:

	Display::C_VertexBuffer	m_VertexBuffer0;
	Display::C_VertexBuffer	m_VertexBuffer1;
	Display::C_VertexBuffer	m_VertexBuffer2;
	Display::C_VertexBuffer	m_VertexBuffer3;
	Display::C_VertexBuffer	m_VertexBuffer4;
	Display::C_IndexBuffer	m_IndexBuffer;
	U32						m_VertexCount;
	U32						m_IndexCount;

};

}; // namespace Geometry
}; // namespace Render

#endif // RENDER_GEOMETRY_INFINITE_GRID_H
