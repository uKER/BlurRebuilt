#ifndef DISPLAY_DEVICE_GCM_GCM_DEVICE_STATES_INL
#define DISPLAY_DEVICE_GCM_GCM_DEVICE_STATES_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{
namespace GCM
{

// C_TextureAddress

INLINE C_TextureAddress::C_TextureAddress()
{
	Reset();
}

INLINE C_TextureAddress::C_TextureAddress(const U8 wraps, const U8 wrapt, const U8 wrapr, const U8 unsignedRemap, const U8 zfunc, const U8 gamma) :
	m_wraps(wraps),
	m_wrapt(wrapt),
	m_wrapr(wrapr),
	m_unsignedRemap(unsignedRemap),
	m_zfunc(zfunc),
	m_gamma(gamma)	
{
}

INLINE const U8 C_TextureAddress::GetWrapS() const
{
	return m_wraps;
}

INLINE const U8 C_TextureAddress::GetWrapT() const
{
	return m_wrapt;
}

INLINE const U8 C_TextureAddress::GetWrapR() const
{
	return m_wrapr;
}

INLINE const U8 C_TextureAddress::GetUnsignedRemap() const
{
	return m_unsignedRemap;
}

INLINE const U8 C_TextureAddress::GetZFunc() const
{
	return m_zfunc;
}

INLINE const U8 C_TextureAddress::GetGamma() const
{
	return m_gamma;
}

INLINE void C_TextureAddress::SetWrapS(const U8 wraps)
{
	m_wraps = wraps;
}

INLINE void C_TextureAddress::SetWrapT(const U8 wrapt)
{
	m_wrapt = wrapt;
}

INLINE void C_TextureAddress::SetWrapR(const U8 wrapr)
{
	m_wrapr = wrapr;
}

INLINE void C_TextureAddress::SetUnsignedRemap(const U8 unsignedRemap)
{
	m_unsignedRemap = unsignedRemap;
}

INLINE void C_TextureAddress::SetZFunc(const U8 zfunc)
{
	m_zfunc = zfunc;
}

INLINE void C_TextureAddress::SetGamma(const U8 gamma)
{
	m_gamma = gamma;
}

INLINE void C_TextureAddress::SetAddressU(const E_TextureAddress addressU)
{
	m_wraps = GCMTextureAddress(addressU);
}

INLINE void C_TextureAddress::SetAddressV(const E_TextureAddress addressV)
{
	m_wrapt = GCMTextureAddress(addressV);
}

INLINE void C_TextureAddress::SetAddressW(const E_TextureAddress addressW)
{
	m_wrapr = GCMTextureAddress(addressW);
}

INLINE C_TextureAddress& C_TextureAddress::operator=(const C_TextureAddress& other)
{
	m_bits = other.m_bits;
	return *this;
}

INLINE bool C_TextureAddress::operator==(const C_TextureAddress& other) const
{
	return m_bits == other.m_bits;
}

INLINE bool C_TextureAddress::operator!=(const C_TextureAddress& other) const
{
	return m_bits != other.m_bits;
}

INLINE void C_TextureAddress::Reset()
{
	m_wraps				= CELL_GCM_TEXTURE_WRAP;
	m_wrapt				= CELL_GCM_TEXTURE_WRAP;
	m_wrapr				= CELL_GCM_TEXTURE_CLAMP_TO_EDGE;
	m_unsignedRemap		= CELL_GCM_TEXTURE_UNSIGNED_REMAP_NORMAL;
	m_zfunc				= CELL_GCM_TEXTURE_ZFUNC_NEVER;
	m_gamma				= 0;
}

INLINE void C_TextureAddress::Set(
		const E_TextureAddress addressU,
		const E_TextureAddress addressV,
		const E_TextureAddress addressW,
		const E_Bool gamma)
{
	m_wraps				= GCMTextureAddress(addressU);
	m_wrapt				= GCMTextureAddress(addressV);
	m_wrapr				= GCMTextureAddress(addressW);
	m_unsignedRemap		= CELL_GCM_TEXTURE_UNSIGNED_REMAP_NORMAL;
	m_zfunc				= CELL_GCM_TEXTURE_ZFUNC_NEVER;
	m_gamma				= GCMGamma(gamma);
}

// C_TextureControl

INLINE C_TextureControl::C_TextureControl()
{
	Reset();
}

INLINE C_TextureControl::C_TextureControl(const U32 enable, const U16 minLod, const U16 maxLod, const U8 maxAniso) :
	m_enable(enable),
	m_minLod(minLod),
	m_maxLod(maxLod),
	m_maxAniso(maxAniso)
{
}

INLINE const U8 C_TextureControl::C_TextureControl::GetEnable() const
{
	return m_enable;
}

INLINE const U16 C_TextureControl::C_TextureControl::GetMinLod() const
{
	return m_minLod;
}

INLINE const U16 C_TextureControl::C_TextureControl::GetMaxLod() const
{
	return m_maxLod;
}

INLINE const U8 C_TextureControl::C_TextureControl::GetMaxAniso() const
{
	return m_maxAniso;
}

INLINE void C_TextureControl::SetEnable(const U8 enable)
{
	m_enable = enable;
}

INLINE void C_TextureControl::SetMinLod(const U16 minLod)
{
	m_minLod = minLod;
}

INLINE void C_TextureControl::SetMaxLod(const U16 maxLod)
{
	m_maxLod = maxLod;
}

INLINE void C_TextureControl::SetMaxAniso(const U8 maxAniso)
{
	m_maxAniso = maxAniso;
}

INLINE C_TextureControl& C_TextureControl::operator=(const C_TextureControl& other)
{
	m_bits = other.m_bits;
	return *this;
}

INLINE bool C_TextureControl::operator==(const C_TextureControl& other) const
{
	return m_bits == other.m_bits;
}

INLINE bool C_TextureControl::operator!=(const C_TextureControl& other) const
{
	return m_bits != other.m_bits;
}

INLINE void C_TextureControl::Reset()
{
	m_enable	= CELL_GCM_FALSE;
	m_minLod	= 0<<8;
	m_maxLod	= 12<<8;
	m_maxAniso	= CELL_GCM_TEXTURE_MAX_ANISO_1;
}

INLINE void C_TextureControl::Set(
		const U32 enable,
		const U16 minLod,
		const U16 maxLod,
		const U8 maxAniso)
{
	m_enable	= enable;	
	m_minLod	= minLod;
	m_maxLod	= maxLod;
	m_maxAniso	= maxAniso;
}

// C_TextureFilter

INLINE C_TextureFilter::C_TextureFilter() 
{
}

INLINE C_TextureFilter::C_TextureFilter(const U16 bias, const U8 min, const U8 mag, const U8 conv) :
	m_bias(bias),
	m_min(min),
	m_mag(mag),
	m_conv(conv)
{
	Reset();
}

INLINE const U16 C_TextureFilter::GetBias() const
{
	return m_bias;
}

INLINE const U8 C_TextureFilter::GetMin() const
{
	return m_min;
}

INLINE const U8 C_TextureFilter::GetMag() const
{
	return m_mag;
}

INLINE const U8 C_TextureFilter::GetConv() const
{
	return m_conv;
}

INLINE void C_TextureFilter::SetBias(const U16 bias)
{
	m_bias = bias;
}

INLINE void C_TextureFilter::SetMin(const U8 min)
{
	m_min = min;
}

INLINE void C_TextureFilter::SetMag(const U8 mag)
{
	m_mag = mag;
}

INLINE void C_TextureFilter::SetConv(const U8 conv)
{
	m_conv = conv;
}

INLINE void C_TextureFilter::SetFilterMag(const E_TextureFilter filterMag)
{
	m_mag = GCMTextureFilterMag(filterMag);
}

INLINE void C_TextureFilter::SetFilterMin(const E_TextureFilter filterMin, const E_TextureFilter filterMip)
{
	m_min = GCMTextureFilterMin(filterMin, filterMip);
}

INLINE C_TextureFilter& C_TextureFilter::operator=(const C_TextureFilter& other)
{
	m_bits = other.m_bits;
	return *this;
}

INLINE bool C_TextureFilter::operator==(const C_TextureFilter& other) const
{
	return m_bits == other.m_bits;
}

INLINE bool C_TextureFilter::operator!=(const C_TextureFilter& other) const
{
	return m_bits != other.m_bits;
}

INLINE void C_TextureFilter::Reset()
{
	m_bias	= 0;
	m_min	= CELL_GCM_TEXTURE_NEAREST_LINEAR;
	m_mag	= CELL_GCM_TEXTURE_LINEAR;
	m_conv	= CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX;
}

INLINE void C_TextureFilter::Set(
	const E_TextureFilter filterMag,
	const E_TextureFilter filterMin,
	const E_TextureFilter filterMip,
	const S8 lodBias)
{
	m_bias	= GCMLodBias(lodBias);
	m_min	= GCMTextureFilterMin(filterMin, filterMip);
	m_mag	= GCMTextureFilterMag(filterMag);
	m_conv	= CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX;
}

// C_TextureOptimisation

INLINE C_TextureOptimisation::C_TextureOptimisation()
{
}

INLINE C_TextureOptimisation::C_TextureOptimisation(const U8 slope, const U8 iso, const U8 aniso) :
	m_slope(slope),
	m_iso(iso),
	m_aniso(aniso)
{
}

INLINE const U8 C_TextureOptimisation::GetSlope() const
{
	return m_slope;
}

INLINE const U8 C_TextureOptimisation::GetIso() const
{
	return m_iso;
}

INLINE const U8 C_TextureOptimisation::GetAniso() const
{
	return m_aniso;
}

INLINE void C_TextureOptimisation::SetSlope(const U8 slope)
{
	m_slope = slope;
}

INLINE void C_TextureOptimisation::SetIso(const U8 iso)
{
	m_iso = iso;
}

INLINE void C_TextureOptimisation::SetAniso(const U8 aniso)
{
	m_aniso = aniso;
}

INLINE C_TextureOptimisation& C_TextureOptimisation::operator=(const C_TextureOptimisation& other)
{
	m_bits = other.m_bits;
	return *this;
}

INLINE bool C_TextureOptimisation::operator==(const C_TextureOptimisation& other) const
{
	return m_bits == other.m_bits;
}

INLINE bool C_TextureOptimisation::operator!=(const C_TextureOptimisation& other) const
{
	return m_bits != other.m_bits;
}

INLINE void C_TextureOptimisation::Reset()
{
	m_bits = 0xffffffff;
}

// C_TextureSampler

INLINE void C_TextureSampler::Set(const U32 borderColour)
{
	m_BorderColour = borderColour;
}

}; // namespace GCM
}; // namespace Display

#endif // DISPLAY_DEVICE_GCM_GCM_DEVICE_STATES_INL
