#ifndef RENDER_DYNAMICEFFECT_LIGHT_GENERIC_SHAPE2DQUAD_H
#define RENDER_DYNAMICEFFECT_LIGHT_GENERIC_SHAPE2DQUAD_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "Dynamic/Light/EffectInterface.h"
#include "Dynamic/Light/Instance.h"
#include "Dynamic/Light/Shape/Quad2d.h"

namespace DynamicEffect
{

class C_Renderer;
class I_DynamicEffectManager;

namespace Light
{

class Shape2dQuad : public I_Effect2d<Instance2d, Quad2dPolicy>
{
public:

	typedef I_Effect2d<Instance2d, Quad2dPolicy> Base;

	Shape2dQuad(C_Renderer* pRenderer, I_DynamicEffectManager* pManager, const U32 iIndex, const U32 iTypeIndex) : Base(pRenderer, pManager, iIndex, iTypeIndex) {}

	virtual void CreateInstance(const InstanceDefinition* pDefinition, Light* pLight);
	virtual void UpdateExtents(const InstanceDefinition* pDefinition, const Maths::C_QuatTrans& transform, Maths::HVec3& vMin, Maths::HVec3& vMax, Maths::HF32& vCullDistance);

};

}; // namespace Light
}; // namespace DynamicEffect

#endif // RENDER_DYNAMICEFFECT_LIGHT_GENERIC_SHAPE2DQUAD_H
