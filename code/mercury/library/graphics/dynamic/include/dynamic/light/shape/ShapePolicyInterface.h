#ifndef RENDER_DYNAMICEFFECT_LIGHT_SHAPEPOLICYINTERFACE_H
#define RENDER_DYNAMICEFFECT_LIGHT_SHAPEPOLICYINTERFACE_H

#if _MSC_VER > 1000
#pragma once
#endif

namespace DynamicEffect
{
namespace Light
{

class I_EffectBase;

class I_ShapePolicy
{
public:

	I_ShapePolicy() {};
	virtual ~I_ShapePolicy() {}

public:
	const I_EffectBase*					m_pEffect;

};

}; // namespace Light
}; // namespace DynamicEffect



#endif // RENDER_DYNAMICEFFECT_LIGHT_SHAPEPOLICYINTERFACE_H
