#ifndef CONVERTER_SHARED_LUX_ENUMS_H
#define CONVERTER_SHARED_LUX_ENUMS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace LuxInterface
{

enum E_Bool
{
	E_False = 0,
	E_True,
	E_Bool_Count
};

enum E_OnOff
{
	E_Off = 0,
	E_On,
	E_OnOff_Count
};

enum E_LightingModelType
{
	E_LightingModelType_Unknown = 0,
	E_LightingModelType_GlobalIllumination,  
	E_LightingModelType_Count
};

enum E_InstanceMode
{
	E_InstanceMode_Instance = 0,
	E_InstanceMode_Instantiate,
	E_InstanceMode_Delete,
	E_InstanceMode_Count
};

enum E_BakeTarget
{
	E_BakeTarget_Vertex = 0,
	E_BakeTarget_Lightmap,
	E_BakeTarget_Count
};

//enum E_TextureFormat
//{
//	E_TextureFormat_DontCare = 0,
//	E_TextureFormat_A8L8,
//	E_TextureFormat_DXT4,
//	E_TextureFormat_RGBA32,
//	E_TextureFormat_DXT1,
//	E_TextureFormat_RGB16,
//	E_TextureFormat_YUV,
//	E_TextureFormat_Count
//};

enum E_TextureType
{
	E_TextureType_Unknown = 0,
	E_TextureType_RGB,
	E_TextureType_GothamNight,
	E_TextureType_BasisV2,
	E_TextureType_Transfer,
	E_TextureType_Direction,
	E_TextureType_Count
};

enum E_LightType
{
	E_LightType_Unknown = 0,
	E_LightType_Directional,
	E_LightType_Hemisphere,
	E_LightType_Hemisphere2,
	E_LightType_BounceGather,
	E_LightType_HDR,
	//E_LightType_PointLight,	
	//E_LightType_SpotLight,
	//E_LightType_Area,
	E_LightType_Count
};

enum E_SetAttribute
{
	E_SetAttribute_Light = 0,
	E_SetAttribute_DontLight,
	E_SetAttribute_CastShadow,
	E_SetAttribute_DontCastShadow, 
	E_SetAttribute_Count
};

enum E_SamplePointsType
{
	E_SamplePointsType_None = 0,
	E_SamplePointsType_SH,  
	E_SamplePointsType_Count
};

enum E_SamplePointsOutputFormat
{
	E_SamplePointsOutputFormat_Binary = 0,
	E_SamplePointsOutputFormat_Text,
	E_SamplePointsOutputFormat_Count
};

enum E_Gamma
{
	E_Gamma_User = 0,
	E_Gamma_On,
	E_Gamma_Off,
	E_Gamma_Count
};

enum E_TimeOfDay
{
	E_TimeOfDay_All = 0,
	E_TimeOfDay_Sunny,
	E_TimeOfDay_Overcast,
	E_TimeOfDay_Night,
	E_TimeOfDay_Count
};

enum E_Channel
{
	E_Channel_None = 0,
	E_Channel_Red, 
	E_Channel_Green, 
	E_Channel_Blue, 
	E_Channel_RGB, 
	E_Channel_Alpha,	
	E_Channel_Sun, 
	E_Channel_Sky,							
	E_Channel_AnimatedOn, 
	E_Channel_AnimatedOff,
	E_Channel_Count
};

enum E_ForceGothamChannels
{
	E_ForceGothamChannels_Off = 0,
	E_ForceGothamChannels_On,
	E_ForceGothamChannels_Version2,
	E_ForceGothamChannels_Count
};

enum E_AntiLightLeaking
{
	E_AntiLightLeaking_Off = 0,
	E_AntiLightLeaking_On,
	E_AntiLightLeaking_All,
	E_AntiLightLeaking_Count
};

enum E_FixSeams
{
	E_FixSeams_Off = 0,
	E_FixSeams_On,
	E_FixSeams_All,
	E_FixSeams_Count
};

enum E_PolyflagLightmapDensity
{
	E_PolyflagLightmapDensity_High,
	E_PolyflagLightmapDensity_Normal,
	E_PolyflagLightmapDensity_Low,
	E_PolyflagLightmapDensity_VeryLow,
	E_PolyflagLightmapDensity_UltraLow,

	E_PolyflagLightmapDensity_Count
};

void GetString( const S32 value, Util::C_String& str );
void GetString( const F32 value, Util::C_String& str );
void GetString( const Util::C_String& value, Util::C_String& str );
void GetString( const Maths::HVec3& value, Util::C_String& str );
void GetString( const Maths::C_RGBA_F32& value, Util::C_String& str );

void GetString( const E_Bool value, Util::C_String& str );
void GetString( const E_OnOff value, Util::C_String& str );
void GetString( const E_LightingModelType value, Util::C_String& str );
void GetString( const E_InstanceMode value, Util::C_String& str );
void GetString( const E_BakeTarget value, Util::C_String& str );
//void GetString( const E_TextureFormat value, Util::C_String& str );
void GetString( const E_TextureType value, Util::C_String& str );
void GetString( const E_LightType value, Util::C_String& str );
void GetString( const E_SetAttribute value, Util::C_String& str );
void GetString( const E_SamplePointsType value, Util::C_String& str );
void GetString( const E_SamplePointsOutputFormat value, Util::C_String& str );
void GetString( const E_Gamma value, Util::C_String& str );
void GetString( const E_TimeOfDay value, Util::C_String& str );
void GetString( const E_Channel value, Util::C_String& str );
void GetString( const E_ForceGothamChannels value, Util::C_String& str );
void GetString( const E_AntiLightLeaking value, Util::C_String& str );
void GetString( const E_FixSeams value, Util::C_String& str );

bool GetValue( const Util::C_String& str, S32& result );
bool GetValue( const Util::C_String& str, F32& result );
bool GetValue( const Util::C_String& str, Util::C_String& result );
bool GetValue( const Util::C_String& str, Maths::HVec3& result );
bool GetValue( const Util::C_String& str, Maths::C_RGBA_F32& result );

bool GetValue( const Util::C_String& str, E_Bool& result );
bool GetValue( const Util::C_String& str, E_OnOff& result );
bool GetValue( const Util::C_String& str, E_LightingModelType& result );
bool GetValue( const Util::C_String& str, E_InstanceMode& result );
bool GetValue( const Util::C_String& str, E_BakeTarget& result );
//bool GetValue( const Util::C_String& str, E_TextureFormat& result );
bool GetValue( const Util::C_String& str, E_TextureType& result );
bool GetValue( const Util::C_String& str, E_LightType& result );
bool GetValue( const Util::C_String& str, E_SetAttribute& result );
bool GetValue( const Util::C_String& str, E_SamplePointsType& result );
bool GetValue( const Util::C_String& str, E_SamplePointsOutputFormat& result );
bool GetValue( const Util::C_String& str, E_Gamma& result );
bool GetValue( const Util::C_String& str, E_TimeOfDay& result );
bool GetValue( const Util::C_String& str, E_Channel& result );
bool GetValue( const Util::C_String& str, E_ForceGothamChannels& result );
bool GetValue( const Util::C_String& str, E_AntiLightLeaking& result );
bool GetValue( const Util::C_String& str, E_FixSeams& result );

} // LuxInterface

#endif