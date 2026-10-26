#ifndef RENDER_DYNAMICEFFECT_FOG_SHAPE_ELLIPSOID_H
#define RENDER_DYNAMICEFFECT_FOG_SHAPE_ELLIPSOID_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "Dynamic/Fog/Shape/Base.h"

namespace DynamicEffect
{
namespace Fog
{

class Ellipsoid : public ShapeBase
{
public:

	Ellipsoid();

	bool Create();
	void Destroy();

public:

	bool		IsInside(const Maths::HVec3& vPoint, const Maths::HMat43& mObject, const Maths::HF32& vEpsilon = 0.0f) const;
	Maths::HF32 EvaluateDensityAt(const Maths::HVec3& vPoint, const Maths::HMat43& mObject, const Maths::HVec2& vSettings) const;

};

}; // namespace Fog
}; // namespace DynamicEffect

#endif // RENDER_DYNAMICEFFECT_FOG_SHAPE_ELLIPSOID_H
