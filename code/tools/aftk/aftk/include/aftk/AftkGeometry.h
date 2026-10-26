#ifndef AFTK_GEOMETRY_H
#define AFTK_GEOMETRY_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	namespace Geometry
	{
		Vector3F32 GetTriangleNormal(
			Position3 v0,
			Position3 v1,
			Position3 v2,
			bool normalise = false,
			const Matrix44* pMatrix = 0,
			bool* pIsDegenerate = 0);
	}
}

#endif
