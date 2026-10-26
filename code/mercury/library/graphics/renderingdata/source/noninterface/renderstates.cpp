#include "stdafx.h"
#include "renderingdata/renderstates.h"
#include "btl/utility/hash.h"

namespace RenderingData
{

BlendFactor BlendState::GetSrcBlend() const
{
	return (BlendFactor) srcBlend;
}

BlendFactor BlendState::GetDestBlend() const
{
	return (BlendFactor) destBlend;
}

BlendOp BlendState::GetBlendOp() const
{
	return (BlendOp) blendOp;
}

bool BlendState::GetAlphaBlendEnable() const
{
	return alphaBlendEnable;
}

bool BlendState::GetAlphaTestEnable() const
{
	return alphaTestEnable;
}

U8 BlendState::GetAlphaRef() const
{
	return alphaRef;
}

bool BlendState::GetAlphaToMaskEnable() const
{
	return alphaToMaskEnable;
}

CullMode BlendState::GetCullMode() const
{
	return (CullMode) cullMode;
}

S32 BlendState::GetZBias() const
{
	return zBias;
}

void BlendState::SetSrcBlend(BlendFactor p)
{
	srcBlend = p;
}

void BlendState::SetDestBlend(BlendFactor p)
{
	destBlend = p;
}

void BlendState::SetBlendOp(BlendOp p)
{
	blendOp = p;
}

void BlendState::SetAlphaBlendEnable(bool p)
{
	alphaBlendEnable = p;
}

void BlendState::SetAlphaTestEnable(bool p)
{
	alphaTestEnable = p;
}

void BlendState::SetAlphaRef(U8 p)
{
	alphaRef = p;
}

void BlendState::SetAlphaToMaskEnable(bool p)
{
	alphaToMaskEnable = p;
}

void BlendState::SetCullMode(CullMode p)
{
	cullMode = p;
}

void BlendState::SetZBias(S32 p)
{
	zBias = Maths::Min(p, 15);
}

void BlendState::SetInvalid()
{
	data = 0xffffffff;
}

U32 Hash(const BlendState& blendState, U32 initval)
{
	return Btl::Hash(blendState.data, initval);
}

bool operator == (const BlendState& b1, const BlendState& b2)
{
	return b1.data == b2.data;
}

bool operator != (const BlendState& b1, const BlendState& b2)
{
	return !(b1 == b2);
}

bool operator < (const BlendState& b1, const BlendState& b2)
{
	return b1.data < b2.data;
}

////////////////////////////////////////////////////////////////////////

TextureAddress SamplerState::GetAddressU() const
{
	return (TextureAddress) addressU;
}

TextureAddress SamplerState::GetAddressV() const
{
	return (TextureAddress) addressV;
}

TextureAddress SamplerState::GetAddressW() const
{
	return (TextureAddress) addressW;
}

TextureFilter SamplerState::GetMagFilter() const
{
	return (TextureFilter) magFilter;
}

TextureFilter SamplerState::GetMinFilter() const
{
	return (TextureFilter) minFilter;
}

TextureFilter SamplerState::GetMipFilter() const
{
	return (TextureFilter) mipFilter;
}

void SamplerState::SetAddressU(TextureAddress p)
{
	addressU = p;
}

void SamplerState::SetAddressV(TextureAddress p)
{
	addressV = p;
}

void SamplerState::SetAddressW(TextureAddress p)
{
	addressW = p;
}

void SamplerState::SetMagFilter(TextureFilter p)
{
	magFilter = p;
}

void SamplerState::SetMinFilter(TextureFilter p)
{
	minFilter = p;
}

void SamplerState::SetMipFilter(TextureFilter p)
{
	mipFilter = p;
}

void SamplerState::SetMaxAnisotropy(S32 p)
{
	maxAnisotropy = p;
}

void SamplerState::SetMaxMipLevel(S32 p)
{
	maxMipLevel = p;
}

void SamplerState::SetMipMapLodBias(S32 p)
{
	mipMapLodBias = p;
}

void SamplerState::SetSRGB(U32 p)
{
	sRGB = p;
}

S32 SamplerState::GetMaxAnisotropy() const
{
	return maxAnisotropy;
}

S32 SamplerState::GetMaxMipLevel() const
{
	return maxMipLevel;
}

S32 SamplerState::GetMipMapLodBias() const
{
	return mipMapLodBias;
}

U32 SamplerState::GetSRGB() const
{
	return sRGB;
}

bool operator == (const SamplerState& s1, const SamplerState& s2)
{
	return s1.data == s2.data;
}

bool operator != (const SamplerState& s1, const SamplerState& s2)
{
	return !(s1 == s2);
}

bool operator < (const SamplerState& s1, const SamplerState& s2)
{
	return s1.data < s2.data;
}

U32 Hash(const SamplerState& samplerState)
{
	return Btl::Hash(samplerState.data);
}

}
