#ifndef RENDER_STATE_RENDERSTATE_H
#define RENDER_STATE_RENDERSTATE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core\utility\Singleton.h>

// render library dependancies
#include "Fog.h"
#include "Render/PostProcess/HDR/HDRInterface.h"

namespace Display
{
	class C_Context;
}

namespace Render
{

enum E_LightingModel
{
	E_LightingModel_None,
};

// class provides an interface for storing common scene rendering states
// eg fog, hdr settings, lights, etc

class C_RenderState : public Ut::Singleton<C_RenderState, Ut::CreateStatic>
{
public:
	C_RenderState();
	~C_RenderState() {};

	// misc shader parameters
	MathsStorage::Float4					GetDepthScaleShaderParameter() const;
	MathsStorage::Float4					GetDepthScaleShaderParameter(const F32 nearClip, const F32 farClip) const;
	MathsStorage::Float4					GetBlendShaderParamater() const;

	void			Update();

	// fog
	const C_Fog*							Fog() const { return &m_Fog; }
	C_Fog*									Fog() { return &m_Fog; }

	// hdr
	const PostProcess::I_HighDynamicRange*	HDR() const { return m_pHDR; }
	bool									ShaderHDR() const;

	// fade in
	void									SetFadeIn(const F32 fFadeInBegin, const F32 fFadeInEnd) const;
	void									SetFadeIn(const F32 fFadeInBegin, const F32 fFadeInEnd, const MathsStorage::Float4& vPosition) const;
	void									SetFadeInShaderParameters(Display::C_Context* pContext = 0) const;

	// gain
	void									SetGain(const F32 fGain) const;
	void									SetGainShaderParameters(Display::C_Context* pContext = 0) const;
	MathsStorage::Float4					GetGainShaderParameters() const;

	// view distance
	const F32								GetViewDistance() const { return m_fViewDistance; }
	void									SetViewDistance(const F32 fViewDistance);

	friend class PostProcess::I_HighDynamicRange;

protected:
	C_Fog									m_Fog;
	const PostProcess::I_HighDynamicRange*	m_pHDR;

	// these can change during rendering
	mutable MathsStorage::Float4			m_vFadeInSettings;
	mutable MathsStorage::Float4			m_vFadeInPosition;

	mutable float							m_fGain;

	mutable F32								m_fViewDistance;
};

}

#endif // RENDER_STATE_RENDERSTATE_H
