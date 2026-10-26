#ifndef RENDER_DYNAMICEFFECT_FOG_GENERIC_BASICSHAPES_H
#define RENDER_DYNAMICEFFECT_FOG_GENERIC_BASICSHAPES_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "Dynamic/Fog/EffectInterface.h"
#include "Dynamic/Fog/Shape/Cuboid.h"
#include "Dynamic/Fog/Shape/Cylinder.h"
#include "Dynamic/Fog/Shape/Ellipsoid.h"

namespace DynamicEffect
{
namespace Fog
{

class C_ShapeCuboid : public C_VolumeEffect<Cuboid>
{
public:

	typedef C_VolumeEffect<Cuboid> Base;

	C_ShapeCuboid(C_Renderer* pRenderer, I_DynamicEffectManager* pManager, const U32 iIndex, const U32 iTypeIndex) : Base(pRenderer, pManager, iIndex, iTypeIndex) {};
	~C_ShapeCuboid() {};

};

class C_ShapeCylinder : public C_VolumeEffect<Cylinder>
{
public:

	typedef C_VolumeEffect<Cylinder> Base;

	C_ShapeCylinder(C_Renderer* pRenderer, I_DynamicEffectManager* pManager, const U32 iIndex, const U32 iTypeIndex) : Base(pRenderer, pManager, iIndex, iTypeIndex) {};
	~C_ShapeCylinder() {};

};

class C_ShapeEllipsoid : public C_VolumeEffect<Ellipsoid>
{
public:

	typedef C_VolumeEffect<Ellipsoid> Base;

	C_ShapeEllipsoid(C_Renderer* pRenderer, I_DynamicEffectManager* pManager, const U32 iIndex, const U32 iTypeIndex) : Base(pRenderer, pManager, iIndex, iTypeIndex) {};
	~C_ShapeEllipsoid() {};

};

}; // namespace Fog
}; // namespace DynamicEffect

#endif // RENDER_DYNAMICEFFECT_FOG_GENERIC_BASICSHAPES_H
