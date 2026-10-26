#ifndef SPOTLIGHTSPECIALFX_H
#define SPOTLIGHTSPECIALFX_H

#ifdef _MSC_VER
#pragma once
#endif

#include <SpecialFX/SpecialFX.h>
#include <TextureResource/TextureResource.h>

namespace SpecialFX
{

class C_SpotLightSpecialFX : public C_BaseSpecialFXComponent
{
public:
	XtArray<C_HermiteKeyFrame>				m_EffectGraph;
	XtF32									m_Time;
	XtArray<C_HermiteKeyFrame>				m_Roll;
	XtArray<C_HermiteKeyFrame>				m_Pitch;
	XtArray<C_HermiteKeyFrame>				m_Heading;
	XtVec3									m_Offset;
	XtRgb									m_Colour;
	XtF32									m_Radius;
	XtF32									m_FieldOfViewX;
	XtF32									m_FieldOfViewY;
	XtF32									m_CullDistance;
	XtSz8									m_TextureName;

	TextureResource::C_TextureHandle		m_TextureHandle;

	// inherited from C_BaseSpecialFXComponent
	virtual void AddToResourceBag( ResourceUtils::I_ResourceBag& pResourceBag ) const; 
private:
	virtual I_SpecialFXComponentInstance* CreateInstance(const Sys::I_ContextProvider* pContextProvider, const ResourceUtils::I_ResourceBag* pResourceBag) const;
};
}

#endif