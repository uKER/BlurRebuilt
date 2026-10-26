#ifndef EDITOR_PRIMITIVES_H
#define EDITOR_PRIMITIVES_H

#ifdef _MSC_VER
#pragma once
#endif

#include "common/ResourcePtr.h"

namespace Editor
{
	typedef AftkRender::C_ResourcePtr<AftkRender::C_MeshClusterResource> PrimitiveResourcePtr;

	namespace Primitive
	{
		extern PrimitiveResourcePtr Axis(F32 sizeX, F32 sizeY, F32 sizeZ);
		extern PrimitiveResourcePtr AxisPositive(F32 sizeX, F32 sizeY, F32 sizeZ);
		extern PrimitiveResourcePtr Locator(F32 sizeX, F32 sizeY, F32 sizeZ);
		extern PrimitiveResourcePtr Box(F32 sizeX, F32 sizeY, F32 sizeZ);
		extern PrimitiveResourcePtr BoxOutline(F32 sizeX, F32 sizeY, F32 sizeZ);
		extern PrimitiveResourcePtr Rectangle(F32 width, F32 height);
		extern PrimitiveResourcePtr Sphere(F32 radius, U32 tessU, U32 tessV);
		extern PrimitiveResourcePtr SphereWireframe(F32 radius, U32 tessU, U32 tessV);
		extern PrimitiveResourcePtr SphereOutline(F32 radius);
		extern PrimitiveResourcePtr Cone(F32 baseRadius, F32 height, U32 tess, bool base=false);
		extern PrimitiveResourcePtr ConeOutline(F32 baseRadius, F32 height);

		extern PrimitiveResourcePtr Tetrahedron(F32 sizeX, F32 sizeY, F32 sizeZ);
		extern PrimitiveResourcePtr Octahedron(F32 sizeX, F32 sizeY, F32 sizeZ);
		extern PrimitiveResourcePtr Icosahedron(F32 sizeX, F32 sizeY, F32 sizeZ);

		enum E_SolidType
		{
			E_SolidType_Tetrahedron,
			E_SolidType_Octahedron,
			E_SolidType_Icosahedron,
		};
		extern PrimitiveResourcePtr GeodesicSphere(F32 radius, E_SolidType baseSolid, U32 subdivisions);
	}
}

#endif