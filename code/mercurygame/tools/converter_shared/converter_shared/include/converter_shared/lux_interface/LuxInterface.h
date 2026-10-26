#ifndef CONVERTER_SHARED_LUX_INTERFACE_H
#define CONVERTER_SHARED_LUX_INTERFACE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "btl/containers/vector.h"

#include <converter_shared/lux_interface/LuxEnums.h>
#include <converter_shared/lux_interface/LuxLights.h>

namespace LuxInterface
{

// C_LightingModelProperties

class C_LightingModelProperties
{
public:
	C_LightingModelProperties();
	~C_LightingModelProperties();

	bool ParseXml( const Sys::C_XmlDomElement& element );
	bool BuildXml( Sys::C_XmlDomElement& element ) const;

public:
	E_Gamma					m_Gamma;
	F32						m_InputGamma;
	F32						m_OutputGamma;
	F32						m_MeterSize;
	F32						m_Brightness;
	//F32						m_BounceScale;
	F32						m_Epsilon;
	//F32						m_AreaLightScale;
	//F32						m_MaxClusterSize;
	//F32						m_MaxClusterArea;
	F32						m_TexelDensity;
	F32						m_AlternateTexelDensityRatio;
	S32						m_DefaultLightmapSize;
	//S32						m_MaxBounces;
	E_BakeTarget			m_DefaultBakeTarget;
	S32						m_AntiAliasing;
	E_OnOff					m_AlphaScaling; // possibly use OutputType="gotham_night"
	E_OnOff					m_IgnoreDirectionalLightsFromMaya;
	E_TimeOfDay				m_TimeOfDay;
	//E_TextureFormat			m_OutputFormat;
	E_TextureType			m_OutputType;
	//S32						m_ProgressiveBouncePasses;
	//E_OnOff					m_ColouredBounceLighting;
	//E_OnOff					m_PrescaleSH;
	E_OnOff					m_TiledOutput;
	E_AntiLightLeaking		m_AntiLightLeaking;
	//E_OnOff					m_SaveExtraSamplePointData; // legacy, used for text ouput of sample points

	//E_OnOff					m_CarMode;
	//E_OnOff					m_DebugMode;
	E_ForceGothamChannels	m_ForceGothamChannels;
	//E_OnOff					m_Optimise;
	E_OnOff					m_ForceShooterChannels;
	//E_OnOff					m_BounceLighting;
	//S32						m_BouncePathsPerLight;
	E_InstanceMode			m_InstanceMode;		
	E_Bool					m_InstancesDefaultToSHLighting;
	E_Bool					m_DirectionalLambert;
	//E_Channel				m_BounceChannel;
	E_FixSeams				m_FixSeams;

	F32						m_PolyflagLightmapDensities[E_PolyflagLightmapDensity_Count];
};

// C_Set

class C_Set
{
public:
	C_Set();
	C_Set( const E_SetAttribute setAttribute, const Util::C_String& layerName );
	~C_Set();

	bool ParseXml( const Sys::C_XmlDomElement& element );
	bool BuildXml( Sys::C_XmlDomElement& element ) const;

public:	
	E_SetAttribute	m_SetAttribute;
	Util::C_String	m_LayerName;
};

// C_Pass

class C_Pass
{
public:
	C_Pass();
	~C_Pass();

	bool ParseXml( const Sys::C_XmlDomElement& element );
	bool BuildXml( Sys::C_XmlDomElement& element ) const;

public:
	Util::C_String m_Name;
	Btl::Vector<C_Set> m_SetList;
};

// C_SamplePointsProperties

class C_SamplePointsProperties
{
public:
	C_SamplePointsProperties();
	~C_SamplePointsProperties();

	bool ParseXml( const Sys::C_XmlDomElement& element );
	bool BuildXml( Sys::C_XmlDomElement& element ) const;

public:
	E_SamplePointsType			m_Type;
	Util::C_String				m_InputFilename;
	Util::C_String				m_OutputFilename;
	E_SamplePointsOutputFormat	m_Output;
};

// C_Channel

class C_Channel
{
public:
	C_Channel();
	~C_Channel();

	void ParseXmlAttributes( const Sys::C_XmlAttributeList& attributes );
	void BuildXmlAttributes( Sys::C_XmlAttributeList& attributes ) const;

public:
	Util::C_String		m_Name;
	E_Channel			m_Target;
};

// C_LightingModel

class C_LightingModel
{
public:
	C_LightingModel();
	~C_LightingModel();

	bool ReadXml( const Util::C_String& filename );
	bool WriteXml( const Util::C_String& filename ) const;

private:
	bool ParseXml( const Sys::C_XmlDom& dom );
	bool BuildXml( Sys::C_XmlDom& dom ) const;

	bool ParseXml( const Sys::C_XmlDomBaseNode& root );
	bool ParseLightingModelXml( const Sys::C_XmlDomElement & element );

public:

	E_LightingModelType			m_LightingModelType;
	C_LightingModelProperties	m_LightingModelProperties;
	C_SamplePointsProperties	m_SamplePointProperties;
	Btl::Vector<C_Pass>			m_PassList;
	Btl::Vector<C_Light*>		m_LightList;
	Btl::Vector<C_Channel>		m_ChannelList;
};

} // LuxInterface

#endif