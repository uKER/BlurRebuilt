#include "stdafx.h"

#include <Core/Maths/Hvec/New/IncludeGeneral.h>
#include <Core/Maths/Hvec/New/IncludeGeneral.inl>

#include <system/DebugLog.h>

#include "renderingdata/LodMesh.h"
#include "renderingdata/noninterface/LodMesh.inl"

#include "renderingdata/maths.h"
#include "core/maths/utmaths/mmatrix.h"

//#pragma optimize("", off)

namespace RenderingData {
namespace Build
{

void Solve(Maths::Normal3 normal, Maths::Vector3 r0, Maths::Vector3 r1, 
			float v0, float v1, float v2,
			Maths::Vector3& result)
{
	UtMaths::MMatrix43 m;
	m.AX(UtMaths::MMatrix43::DataType(Maths::GetF32(X(r0)), Maths::GetF32(Y(r0)), Maths::GetF32(Z(r0)), v0 - v1));
	m.AY(UtMaths::MMatrix43::DataType(Maths::GetF32(X(r1)), Maths::GetF32(Y(r1)), Maths::GetF32(Z(r1)), v0 - v2));
	m.AZ(UtMaths::MMatrix43::DataType(Maths::GetF32(X(normal)), Maths::GetF32(Y(normal)), Maths::GetF32(Z(normal)), 0.f));

	Vector3 gradient(0.f, 0.f, 0.f);
	UtMaths::GESolve(m, &gradient);
	result = Maths::MakeVector3(gradient.X(), gradient.Y(), gradient.Z());
}

void LodMesh::CalculateGradients(const LodTriangle& triangle, int vertex, LodTriangleVertexGradients* gradients)
{
	// get next and previous vertices
	int i0 = vertex;
	int i1 = vertex + 1; 
	int i2 = vertex - 1;
	if (i1 >= 3) i1 -= 3;
	if (i2 < 0) i2 += 3;

	Maths::Point3 p0(m_Vertices[triangle.m_VertexIndex[i0]].GetVertex());
	Maths::Point3 p1(m_Vertices[triangle.m_VertexIndex[i1]].GetVertex());
	Maths::Point3 p2(m_Vertices[triangle.m_VertexIndex[i2]].GetVertex());
	Maths::Normal3 normal(triangle.m_SurfaceNormal);
	Maths::Vector3 r0 = p1 - p0;
	Maths::Vector3 r1 = p2 - p0;

	// colour
	{
		Solve(normal, r0, r1,
				Maths::GetF32(X(Load(&triangle.m_Colour[i0]))),
				Maths::GetF32(X(Load(&triangle.m_Colour[i1]))),
				Maths::GetF32(X(Load(&triangle.m_Colour[i2]))),
				gradients->colour[0]);

		Solve(normal, r0, r1,
				Maths::GetF32(Y(Load(&triangle.m_Colour[i0]))),
				Maths::GetF32(Y(Load(&triangle.m_Colour[i1]))),
				Maths::GetF32(Y(Load(&triangle.m_Colour[i2]))),
				gradients->colour[1]);

		Solve(normal, r0, r1,
				Maths::GetF32(Z(Load(&triangle.m_Colour[i0]))),
				Maths::GetF32(Z(Load(&triangle.m_Colour[i1]))),
				Maths::GetF32(Z(Load(&triangle.m_Colour[i2]))),
				gradients->colour[2]);

		Solve(normal, r0, r1,
				Maths::GetF32(W(Load(&triangle.m_Colour[i0]))),
				Maths::GetF32(W(Load(&triangle.m_Colour[i1]))),
				Maths::GetF32(W(Load(&triangle.m_Colour[i2]))),
				gradients->colour[3]);
	}

	for (int i = 0; i < LodTriangle::m_MaxTexCoordCount; i++)
	{
		Solve(normal, r0, r1,
				Maths::GetF32(X(Load(&triangle.m_TexCoord[i][i0]))),
				Maths::GetF32(X(Load(&triangle.m_TexCoord[i][i1]))),
				Maths::GetF32(X(Load(&triangle.m_TexCoord[i][i2]))),
				gradients->texCoord[i][0]);

		Solve(normal, r0, r1,
				Maths::GetF32(Y(Load(&triangle.m_TexCoord[i][i0]))),
				Maths::GetF32(Y(Load(&triangle.m_TexCoord[i][i1]))),
				Maths::GetF32(Y(Load(&triangle.m_TexCoord[i][i2]))),
				gradients->texCoord[i][1]);

		Solve(normal, r0, r1,
				Maths::GetF32(Z(Load(&triangle.m_TexCoord[i][i0]))),
				Maths::GetF32(Z(Load(&triangle.m_TexCoord[i][i1]))),
				Maths::GetF32(Z(Load(&triangle.m_TexCoord[i][i2]))),
				gradients->texCoord[i][2]);

		Solve(normal, r0, r1,
				Maths::GetF32(W(Load(&triangle.m_TexCoord[i][i0]))),
				Maths::GetF32(W(Load(&triangle.m_TexCoord[i][i1]))),
				Maths::GetF32(W(Load(&triangle.m_TexCoord[i][i2]))),
				gradients->texCoord[i][3]);
	}
}

}}