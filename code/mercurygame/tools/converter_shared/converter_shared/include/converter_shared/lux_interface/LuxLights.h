#ifndef CONVERTER_SHARED_LUX_LIGHTS_H
#define CONVERTER_SHARED_LUX_LIGHTS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <converter_shared/lux_interface/LuxEnums.h>

namespace LuxInterface
{

// C_Light

class C_Light
{
public:
	C_Light();
	virtual ~C_Light();

	virtual E_LightType GetLightType() const = 0;
	
	virtual void ParseXmlAttributes( const Sys::C_XmlAttributeList& attributes ) = 0;
	virtual void BuildXmlAttributes( Sys::C_XmlAttributeList& attributes ) const = 0;

public:
	Util::C_String		m_Name;
	Maths::HVec3		m_Position;
	Maths::C_RGBA_F32	m_Colour;
	F32					m_Intensity;
	S32					m_Samples;
	E_Channel			m_Channel;
};

// C_LightDirectional

class C_LightDirectional : public C_Light
{
public:
	C_LightDirectional();
	~C_LightDirectional();

	E_LightType GetLightType() const { return E_LightType_Directional; }

	void ParseXmlAttributes( const Sys::C_XmlAttributeList& attributes );
	void BuildXmlAttributes( Sys::C_XmlAttributeList& attributes ) const;
public:
	Maths::HVec3		m_Direction;
	F32					m_Penumbra;
	E_OnOff				m_Lambert;
};

// C_LightHemisphere

class C_LightHemisphere : public C_Light
{
public:
	C_LightHemisphere();
	~C_LightHemisphere();

	E_LightType GetLightType() const { return E_LightType_Hemisphere; }

	void ParseXmlAttributes( const Sys::C_XmlAttributeList& attributes );
	void BuildXmlAttributes( Sys::C_XmlAttributeList& attributes ) const;
public:

	E_Bool	m_Hemisphere;
};

// C_LightHemisphere2

class C_LightHemisphere2 : public C_Light
{
public:
	C_LightHemisphere2();
	~C_LightHemisphere2();

	E_LightType GetLightType() const { return E_LightType_Hemisphere2; }

	void ParseXmlAttributes( const Sys::C_XmlAttributeList& attributes );
	void BuildXmlAttributes( Sys::C_XmlAttributeList& attributes ) const;
public:
	Maths::HVec3	m_SunDirection;
	F32				m_SunIntensity;
};

// C_LightBounceGather

class C_LightBounceGather : public C_Light
{
public:
	C_LightBounceGather();
	~C_LightBounceGather();

	E_LightType GetLightType() const { return E_LightType_BounceGather; }

	void ParseXmlAttributes( const Sys::C_XmlAttributeList& attributes );
	void BuildXmlAttributes( Sys::C_XmlAttributeList& attributes ) const;
public:
	F32				m_BounceScale;
	S32				m_BouncePasses;
	E_OnOff			m_CalcLightTransfer;
	E_Bool			m_Hemisphere;
};

// C_LightHDR

class C_LightHDR : public C_Light
{
public:
	C_LightHDR();
	~C_LightHDR();

	E_LightType GetLightType() const { return E_LightType_HDR; }

	void ParseXmlAttributes( const Sys::C_XmlAttributeList& attributes );
	void BuildXmlAttributes( Sys::C_XmlAttributeList& attributes ) const;
public:
	Util::C_String	m_HdrMap;
	F32				m_Rotation;
};

} // LuxInterface

#endif