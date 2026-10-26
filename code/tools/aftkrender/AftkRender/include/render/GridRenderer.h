#ifndef AFTK_RENDER_GRID_RENDERER_H
#define AFTK_RENDER_GRID_RENDERER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace AftkRender
{
	class C_GridRenderer
	{
	public:

		enum E_GridPlane
		{
			E_GridPlane_XZ,
			E_GridPlane_XY,
			E_GridPlane_YZ

		};

		//render along from origin
		static void Render(U32 cellsU, U32 cellsV, F32 spacing, const Maths::RGBA& col, E_GridPlane plane=E_GridPlane_XZ);

		// render arbitrary section of grid.  Can offset position along vector perpendicular to grid plane
		static void Render(const Maths::HVec2& minUV, const Maths::HVec2& maxUV, const Maths::RGBA& col, F32 spacing, F32 offset=0.f, E_GridPlane plane=E_GridPlane_XZ);
	};
}

#endif