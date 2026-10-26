#ifndef RENDER_DYNAMICEFFECT_LIGHT_GENERIC_SHAPE3DQUAD_H
#define RENDER_DYNAMICEFFECT_LIGHT_GENERIC_SHAPE3DQUAD_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "Dynamic/Light/EffectInterface.h"
#include "Dynamic/Light/Instance.h"
#include "Dynamic/Light/Shape/Quad3d.h"

namespace DynamicEffect
{

class C_Renderer;
class I_DynamicEffectManager;

namespace Light
{

class Shape3dQuad : public I_Effect3d<Instance3d_P, Quad3dPolicy>
{
public:

	typedef I_Effect3d<Instance3d_P, Quad3dPolicy> Base;

	Shape3dQuad(C_Renderer* pRenderer, I_DynamicEffectManager* pManager, const U32 iIndex, const U32 iTypeIndex) : Base(pRenderer, pManager, iIndex, iTypeIndex) {}

	virtual void CreateInstance(const InstanceDefinition* pDefinition, Light* pLight);
	virtual void UpdateExtents(const InstanceDefinition* pDefinition, const Maths::C_QuatTrans& transform, Maths::HVec3& vMin, Maths::HVec3& vMax, Maths::HF32& vCullDistance);

};

}; // namespace Light
}; // namespace DynamicEffect

#endif // RENDER_DYNAMICEFFECT_LIGHT_GENERIC_SHAPE3DQUAD_H
