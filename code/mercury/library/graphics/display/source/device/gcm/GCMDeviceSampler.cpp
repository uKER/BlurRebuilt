#include "stdafx.h"
#include "Display/Device/GCM/GCMDeviceSampler.h"

namespace NewDisplay
{
namespace GCM
{

C_GCMSampler::C_GCMSampler() :
	m_pTexture(NULL),
	m_AddressU(E_TextureAddress_Wrap),
	m_AddressV(E_TextureAddress_Wrap),
	m_AddressW(E_TextureAddress_Wrap),
	m_FilterMag(E_TextureFilter_Linear),
	m_FilterMin(E_TextureFilter_Linear),
	m_MinLod(1 * 0xff),
	m_MaxLod(13 * 0xff),
	m_BorderColour(0x00000000),
	m_SetTexture(false),
	m_SetControl(false),
	m_SetAddress(false),
	m_SetFilter(false),
	m_SetBorderColour(false)
{
}

C_GCMSampler::~C_GCMSampler()
{
}

}; // namespace GCM
}; // namespace NewDisplay
