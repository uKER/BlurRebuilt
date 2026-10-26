#ifndef RENDER_DYNAMICEFFECT_TRACER_MANAGER_H
#define RENDER_DYNAMICEFFECT_TRACER_MANAGER_H

#if _MSC_VER > 1000
#pragma once
#endif
#include <core\containers\radix.h>
#include <core\containers\bitvector.h>

#include "Dynamic/ManagerInterface.h"
#include "Dynamic/Tracer/Enums.h"

namespace DynamicEffect
{
namespace Tracer
{
class I_EffectBase;
typedef Ut::UtMap<U32, I_EffectBase*>	EffectsMap;

class C_Manager : public I_DynamicEffectManager
{
public:

	C_Manager(C_Renderer* pRenderer, const U32 iTypeIndex);
	virtual ~C_Manager();

	void		SetVertexShader();
	void		SetPixelShader(const E_PixelShader ePixelShader);

	void		AddEffect(I_EffectBase* pEffect);
	void		RemoveEffect(I_EffectBase* pEffect);

	//void		AddActiveEffect(I_EffectBase* pEffect);
	//void		RemoveActiveEffect(I_EffectBase* pEffect);

protected:

	virtual void				CleanNode(const C_Node* pNode);
	virtual void				CleanEffect(const I_DynamicEffect* pEffect);

	virtual bool				Create();
	virtual bool				CreateVolatile();
	virtual bool				Initialise();
	virtual void				Destroy();
	virtual void				DestroyVolatile();
	virtual void				Shutdown();
	virtual void				Reset();
	virtual void				SetShaderConstants() const;

	// 3D debug graphics
	virtual void				DisplayDebugGraphics() const;
	// 2D overlays etc
	virtual MathsStorage::Int2	DisplayDebugInfo(MathsStorage::Int2 position) const;

private:

	Display::C_VertexShader			m_VertexShader;
	Display::C_PixelShader			m_PixelShader[E_PixelShader_Count];
	Display::C_PixelShaderParameter	m_LuminanceParam[E_PixelShader_Count];


	Btl::Vector<I_EffectBase*>		m_Effects;
	EffectsMap						m_EffectsMap;
};



}; // namespace Tracer
}; // namespace DynamicEffect

#endif // RENDER_DYNAMICEFFECT_TRACER_MANAGER_H
