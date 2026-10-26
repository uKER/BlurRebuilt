#ifndef RENDER_DYNAMICEFFECT_STRIP_EFFECTINTERFACE_H
#define RENDER_DYNAMICEFFECT_STRIP_EFFECTINTERFACE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Block.h"
#include "../Interface.h"

namespace DynamicEffect
{
class C_Renderer;

namespace Strip
{

class C_Manager;
class C_Emitter;

class I_StripEffectBase : public DynamicEffect::I_DynamicEffect
{
public:
	I_StripEffectBase(C_Renderer* pRenderer, C_Manager* pManager, const U32 iIndex, const U32 iTypeIndex, const U32 iElementSize);
	virtual ~I_StripEffectBase();

	// virtual methods
	virtual C_RenderNode*		Spawn(const C_Emitter* pEmitter) = 0;
	const C_Manager*			GetManager() const { return m_pManager; }

	friend class 				C_Emitter;

protected:
	
	C_Manager*					m_pManager;
	const U32					m_iElementSize;
};

}
}

#endif // RENDER_DYNAMICEFFECT_PARTICLE_EFFECTINTERFACE_H
