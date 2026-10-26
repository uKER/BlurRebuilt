#ifndef DISPLAY_GCMDEVICESAMPLER_H
#define DISPLAY_GCMDEVICESAMPLER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Texture/GCM/GCMTexture.h"

namespace NewDisplay
{
namespace GCM
{

class C_GCMSampler
{
public:

	C_GCMSampler();
	~C_GCMSampler();

	void SetTexture(const C_Texture* pTexture);
	void SetAddressU(const E_TextureAddress addressU);
	void SetAddressV(const E_TextureAddress addressV);
	void SetAddressW(const E_TextureAddress addressW);
	void SetMagFilter(const E_TextureFilter filterMag);
	void SetMinFilter(const E_TextureFilter filterMin);
	void SetMinLod(const U16 minLod);
	void SetMaxLod(const U16 maxLod);
	void SetBorderColour(const U32 borderColour);
	void Set(const U32 sampler);
	void SetDefaults(const U32 sampler);

public:

	const C_Texture*	m_pTexture;
	E_TextureAddress	m_AddressU;
	E_TextureAddress	m_AddressV;
	E_TextureAddress	m_AddressW;
	E_TextureFilter		m_FilterMag;
	E_TextureFilter		m_FilterMin;
	U16					m_MinLod;
	U16					m_MaxLod;
	U32					m_BorderColour;

private:

	bool				m_SetTexture;
	bool				m_SetControl;
	bool				m_SetAddress;
	bool				m_SetFilter;
	bool				m_SetBorderColour;
};

#include "GCMDeviceSampler.inl"

}; // namespace GCM
}; // namespace NewDisplay

#endif // DISPLAY_GCMDEVICESAMPLER_H
