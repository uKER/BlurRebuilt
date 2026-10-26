#include "stdafx.h"

using namespace Aftk;

Vector3F32 Geometry::GetTriangleNormal(
	Position3 v0,
	Position3 v1,
	Position3 v2,
	bool normalise,
	const Matrix44* pMatrix,
	bool* pIsDegenerate)
{
	Vector3F32 normal;

	if (pMatrix)
	{
		v0 = ApplyMatrix(*pMatrix, v0);
		v1 = ApplyMatrix(*pMatrix, v1);
		v2 = ApplyMatrix(*pMatrix, v2);
	}
	(Vector3F32&)v1 = (Vector3F32&)v1 - (Vector3F32&)v0;
	(Vector3F32&)v2 = (Vector3F32&)v2 - (Vector3F32&)v0;

	//Exact check, no epsilon.
	if (pIsDegenerate)
	{
		if (v1 == v2)
			*pIsDegenerate = true;
		else
			*pIsDegenerate = false;
	}

	normal = (Vector3F32&)v2 ^ (Vector3F32&)v1;

	if (normalise)
		UtMaths::SafeNormalise(&normal);
	return (Vector3F32&)normal;
}
