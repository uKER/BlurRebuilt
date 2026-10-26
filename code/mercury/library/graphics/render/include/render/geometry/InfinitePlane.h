#ifndef RENDER_GEOMETRY_INFINITE_PLANE_H
#define RENDER_GEOMETRY_INFINITE_PLANE_H

#if _MSC_VER > 1000
#pragma once
#endif

namespace Render
{
namespace Geometry
{

class C_InfinitePlane
{
public:

	C_InfinitePlane();
	~C_InfinitePlane();

	bool Create();
	void Destroy();
	void Render();

public:

	Maths::C_Plane	m_vPlane;

};

}; // namespace Geometry
}; // namespace Render

#endif // RENDER_GEOMETRY_INFINITE_PLANE_H
