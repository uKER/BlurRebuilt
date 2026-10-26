#ifndef RP_DRAW_H
#define RP_DRAW_H
#ifdef _MSC_VER
#pragma once
#endif

#include "colour.h"
#include "btl/containers/vector.h"
#include "display/buffer/vertexdeclarations.h"
#include "racingphysics/physics/geometry.h"

namespace RacingPhysics
{

class DrawPrimitive
{
public:
	static DrawPrimitive& Instance() {return s_instance;}

	void Begin(Display::E_PrimitiveType primitiveType);
	void End();

	void SetVertexDataXYZ(float x, float y, float z); // add new vertex to draw
	void SetVertexDataXYZ(const Point3R& p); // add new vertex to draw
	void SetVertexDataXY(float x, float y); // add new vertex to draw
	void SetVertexDataColor(ColourARGB colour); // set persistent colour
	void SetVertexDataUV(float u, float v); // set persistent uv
	void SetVertexDataXYZ_C(const Point3R& p, const ColourARGB& c);

private:
	DrawPrimitive() {}

	Display::E_PrimitiveType m_primitiveType;
	Btl::Vector<Display::Vertex_XYZ_C_UV> m_vertices;
	Display::Vertex_XYZ_C_UV m_vertex;

	static DrawPrimitive s_instance;
};

////////////////////////////////////////////////////////////////

FORCE_INLINE void DrawPrimitive::SetVertexDataXYZ(float x, float y, float z)
{
	int n = m_vertices.Size();
	m_vertices.PushBack();
	Display::Vertex_XYZ_C_UV& v = m_vertices[n];

	v.x = x;
	v.y = y;
	v.z = z;
	v.c = m_vertex.c;
	v.u = m_vertex.u;
	v.v = m_vertex.v;
}

FORCE_INLINE void DrawPrimitive::SetVertexDataXYZ(const Point3R& p)
{
	int n = m_vertices.Size();
	m_vertices.PushBack();
	Display::Vertex_XYZ_C_UV& v = m_vertices[n];

	Maths::Store(p.X(), &v.x);
	Maths::Store(p.Y(), &v.y);
	Maths::Store(p.Z(), &v.z);
	v.c = m_vertex.c;
	v.u = m_vertex.u;
	v.v = m_vertex.v;
}

FORCE_INLINE void DrawPrimitive::SetVertexDataXYZ_C(const Point3R& p, const ColourARGB& c)
{
	int n = m_vertices.Size();
	m_vertices.PushBack();
	Display::Vertex_XYZ_C_UV& v = m_vertices[n];

	Maths::Store(p.X(), &v.x);
	Maths::Store(p.Y(), &v.y);
	Maths::Store(p.Z(), &v.z);
	v.c = c.Packed();
	v.u = m_vertex.u;
	v.v = m_vertex.v;
}

FORCE_INLINE void DrawPrimitive::SetVertexDataColor(ColourARGB colour)
{
	m_vertex.c = colour.Packed();
}

FORCE_INLINE void DrawPrimitive::SetVertexDataUV(float u, float v)
{
	m_vertex.u = u;
	m_vertex.v = v;
}

FORCE_INLINE void DrawPrimitive::SetVertexDataXY(float x, float y)
{
	m_vertices.PushBack();
	m_vertices.Back().x = x;
	m_vertices.Back().y = y;
	m_vertices.Back().z = 0.f;
	m_vertices.Back().c = m_vertex.c;
	m_vertices.Back().u = m_vertex.u;
	m_vertices.Back().v = m_vertex.v;
}

}

#endif
