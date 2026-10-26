#ifndef RENDER_DYNAMICEFFECT_LIGHT_GENERIC_SHAPE2DCROSS_H
#define RENDER_DYNAMICEFFECT_LIGHT_GENERIC_SHAPE2DCROSS_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "Dynamic/Light/EffectInterface.h"
#include "Dynamic/Light/Instance.h"
#include "Dynamic/Light/Shape/Cross2d.h"

namespace DynamicEffect
{

class C_Renderer;
class I_DynamicEffectManager;

namespace Light
{

class Shape2dCross : public I_Effect2d<Instance2d, Cross2dPolicy>
{
public:

	typedef I_Effect2d<Instance2d, Cross2dPolicy> Base;

	Shape2dCross(C_Renderer* pRenderer, I_DynamicEffectManager* pManager, const U32 iIndex, const U32 iTypeIndex) : Base(pRenderer, pManager, iIndex, iTypeIndex) {}

	virtual void CreateInstance(const InstanceDefinition* pDefinition, Light* pLight);
	virtual void UpdateExtents(const InstanceDefinition* pDefinition, const Maths::C_QuatTrans& transform, Maths::HVec3& vMin, Maths::HVec3& vMax, Maths::HF32& vCullDistance);

};

}; // namespace Light
}; // namespace DynamicEffect

#endif // RENDER_DYNAMICEFFECT_LIGHT_GENERIC_SHAPE2DCROSS_H
