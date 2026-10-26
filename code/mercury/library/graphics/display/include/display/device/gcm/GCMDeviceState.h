#ifndef DISPLAY_GCMDEVICESTATE_H
#define DISPLAY_GCMDEVICESTATE_H

#pragma once

#include "Display/Device/GCM/GCM.h"

namespace NewDisplay
{
namespace GCM
{

class C_State
{
public:

	C_State();

	~C_State();

	/// State qureying
	
	const F32 GetClearDepth() const;
	const U32 GetClearStencil() const;
	const bool GetFlipZ() const;

	// State settingg
	void SetAlphaEnable(const bool alphaEnable);
	void SetAlphaFunc(const E_CmpFunc alphaFunc);
	void SetAlphaRef(const U32 alphaRef);
	void SetBlendDst(const E_Blend blendDst);
    void SetBlendEnable(const bool blendEnable);
    void SetBlendMode(const E_BlendMode blendMode);
	void SetBlendOp(const E_BlendOp blendOp);
	void SetBlendSrc(const E_Blend blendSrc);
	void SetCullEnable(const bool cullEnable);
	void SetCullMode(const E_CullMode cullMode);
	void SetFillMode(const E_FillMode fillMode);
	void SetFlipZ(const bool flipZ);
	void SetZEnable(const bool zEnable);
	void SetZFunc(const E_CmpFunc zFunc);
	void SetZWriteEnable(const bool zWriteEnable);

protected:

	bool								m_AlphaEnable;
	E_CmpFunc							m_AlphaFunc;
	U32									m_AlphaRef;
	bool								m_BlendEnable;
	E_Blend								m_BlendDst;
	E_BlendMode							m_BlendMode;
	E_BlendOp							m_BlendOp;
	E_Blend								m_BlendSrc;
	F32									m_ClearDepth;
	U32									m_ClearStencil;
	U32									m_ColourWriteEnable;
	bool								m_CullEnable;
	E_CullMode							m_CullMode;
	F32									m_DepthBias;
	E_FillMode							m_FillMode;
	bool								m_FlipZ;
	F32									m_SlopeScaledBias;
	bool								m_ZEnable;
	E_CmpFunc							m_ZFunc;
	bool								m_ZWriteEnable;
};

#include "GCMDeviceState.inl"

}; // namespace GCM
}; // namespace NewDisplay

#endif // DISPLAY_GCMDEVICESTATE_H
