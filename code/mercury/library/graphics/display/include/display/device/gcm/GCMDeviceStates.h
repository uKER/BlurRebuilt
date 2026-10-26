#ifndef DISPLAY_DEVICE_GCM_GCM_DEVICE_STATES_H
#define DISPLAY_DEVICE_GCM_GCM_DEVICE_STATES_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/GCM/GCMDeviceTypes.h"

namespace Display
{
namespace GCM
{

class C_GCMDevice;

//! C_TextureAddress

class C_TextureAddress
{
friend class C_GCMDevice;

public:

	C_TextureAddress();
	C_TextureAddress(const U8 wraps, const U8 wrapt, const U8 wrapr, const U8 unsignedRemap, const U8 zfunc, const U8 gamma);

	const U8 GetWrapS() const;
	const U8 GetWrapT() const;
	const U8 GetWrapR() const;
	const U8 GetUnsignedRemap() const;
	const U8 GetZFunc() const;
	const U8 GetGamma() const;

	void SetWrapS(const U8 wraps);
	void SetWrapT(const U8 wrapt);
	void SetWrapR(const U8 wrapr);
	void SetUnsignedRemap(const U8 unsignedRemap);
	void SetZFunc(const U8 zfunc);
	void SetGamma(const U8 gamma);

	void SetAddressU(const E_TextureAddress addressU);
	void SetAddressV(const E_TextureAddress addressV);
	void SetAddressW(const E_TextureAddress addressW);

	C_TextureAddress& operator=(const C_TextureAddress& other);
	bool operator==(const C_TextureAddress& other) const;
	bool operator!=(const C_TextureAddress& other) const;

	void Reset();
	
	void Set(
		const E_TextureAddress addressU,
		const E_TextureAddress addressV,
		const E_TextureAddress addressW,
		const E_Bool gamma);

private:

	union
	{
		struct
		{
			U64 	m_wraps				: 8;
			U64 	m_wrapt				: 8;
			U64 	m_wrapr				: 8;
			U64 	m_unsignedRemap		: 8;
			U64 	m_zfunc				: 8;
			U64 	m_gamma				: 24;
		};
		U64			m_bits;
	};
};

//! C_TextureControl

class C_TextureControl
{
friend class C_GCMDevice;

public:

	C_TextureControl();
	C_TextureControl(const U32 enable, const U16 minLod, const U16 maxLod, const U8 maxAniso);

	const U8 GetEnable() const;
	const U16 GetMinLod() const;
	const U16 GetMaxLod() const;
	const U8 GetMaxAniso() const;
	
	void SetEnable(const U8 enable);
	void SetMinLod(const U16 minLod);
	void SetMaxLod(const U16 maxLod);
	void SetMaxAniso(const U8 maxAniso);
	
	C_TextureControl& operator=(const C_TextureControl& other);
	bool operator==(const C_TextureControl& other) const;
	bool operator!=(const C_TextureControl& other) const;

	void Reset();

	void Set(
		const U32 enable,
		const U16 minLod,
		const U16 maxLod,
		const U8 maxAniso);

private:

	union
	{
		struct
		{
			U32		m_enable			: 8;
			U32		m_minLod			: 16;
			U32		m_maxLod			: 16;
			U32		m_maxAniso			: 8;
		};
		U32			m_bits;
	};
};

//! C_TextureFilter

class C_TextureFilter
{
friend class C_GCMDevice;

public:

	C_TextureFilter();
	C_TextureFilter(const U16 bias, const U8 min, const U8 mag, const U8 conv);

	const U16 GetBias() const;
	const U8 GetMin() const;
	const U8 GetMag() const;
	const U8 GetConv() const;

	void SetBias(const U16 bias);
	void SetMin(const U8 min);
	void SetMag(const U8 mag);
	void SetConv(const U8 conv);

	void SetFilterMag(const E_TextureFilter filterMag);
	void SetFilterMin(const E_TextureFilter filterMin, const E_TextureFilter filterMip);
	
	C_TextureFilter& operator=(const C_TextureFilter& other);
	bool operator==(const C_TextureFilter& other) const;
	bool operator!=(const C_TextureFilter& other) const;

	void Reset();

	void Set(
		const E_TextureFilter filterMag,
		const E_TextureFilter filterMin,
		const E_TextureFilter filterMip,
		const S8 lodBias);

private:

	union
	{
		struct
		{
			U32		m_bias				: 16;
			U32 	m_min				: 8;
			U32 	m_mag				: 8;
			U32 	m_conv				: 8;
		};
		U32			m_bits;
	};
};

//! C_TextureOptimisation

class C_TextureOptimisation
{
friend class C_GCMDevice;

public:

	C_TextureOptimisation();
	C_TextureOptimisation(const U8 slope, const U8 iso, const U8 aniso);

	void Reset();

	const U8 GetSlope() const;
	const U8 GetIso() const;
	const U8 GetAniso() const;

	void SetSlope(const U8 slope);
	void SetIso(const U8 iso);
	void SetAniso(const U8 aniso);
	
	C_TextureOptimisation& operator=(const C_TextureOptimisation& other);
	bool operator==(const C_TextureOptimisation& other) const;
	bool operator!=(const C_TextureOptimisation& other) const;

private:

	union
	{
		struct
		{
			U32		m_slope				: 8;
			U32		m_iso				: 8;
			U32		m_aniso				: 16;
		};
		U32			m_bits;
	};
};

//! C_TextureSampler

class C_TextureSampler
{
friend class C_GCMDevice;

public:

	C_TextureSampler();

	void Set(const U32 borderColour);

private:

	U32						m_BorderColour;
	C_TextureAddress		m_Address;
	C_TextureControl		m_Control;
	C_TextureFilter			m_Filter;
	C_TextureOptimisation	m_Optimisation;
};

}; // namespace GCM
}; // namespace Display

#ifdef CFG_DEBUG
#define DONT_INLINE_GCM_DEVICE_STATES_INL
#endif

#ifndef DONT_INLINE_GCM_DEVICE_STATES_INL
#include "Display/Device/GCM/GCMDeviceStates.inl"
#endif

#endif // DISPLAY_DEVICE_GCM_GCM_DEVICE_STATES_H
