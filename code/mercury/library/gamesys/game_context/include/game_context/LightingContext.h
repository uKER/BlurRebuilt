#ifndef GAMECONTEXT_LIGHTINGCONTEXT_H
#define GAMECONTEXT_LIGHTINGCONTEXT_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/context/I_Anything.h>
#include <misc_shared/uid/FourCC.h>


namespace DynamicEffect 
{
	namespace  Light
	{
		class C_Manager;
	}
}

namespace GameContext
{

class I_LightingContext : public Sys::C_Anything<FOURCC_U32('IALC')>
{
public:
	virtual DynamicEffect::Light::C_Manager* GetLightManager() const = 0;

protected:
	explicit I_LightingContext(const Sys::C_AnythingUid typeUid) : C_Super(typeUid) {}
};

class C_LightingContext : public Sys::C_Anything<FOURCC_U32('CALC'), I_LightingContext>
{
public:
	explicit C_LightingContext(DynamicEffect::Light::C_Manager* pLightManager) : m_pLightManager(pLightManager) {}

	virtual DynamicEffect::Light::C_Manager* GetLightManager() const {return m_pLightManager;} // from I_LightingContext

private:
	DynamicEffect::Light::C_Manager* m_pLightManager;
};

} // namespace GameContext


#endif // GAMECONTEXT_LIGHTINGCONTEXT_H
