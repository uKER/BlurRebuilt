#ifndef __FX_COMMON_H__
#define __FX_COMMON_H__

/////////////////////////////////
//	Defines
/////////////////////////////////
#define NO_LIGHTMAP
#define NO_VTXCOL
#define DISABLE_LODFADE
#define ENABLE_DYNAMICCUBEMAP

#if !defined(_NO_ALBEDO) && defined(_VEH_ENABLE_ALBEDO)
#define _USE_ALBEDO
#endif
#if !defined(_NO_BUMP) && defined(_VEH_ENABLE_BUMP)
#define _USE_BUMP
#endif
#if (!defined(_NO_DENT) && defined(_VEH_ENABLE_DENT)) || defined(_VEH_ASSUME_DENT)
#define _USE_DENT
#endif
#if (!defined(_NO_SCRATCH_DAMAGE) && defined(_VEH_ENABLE_SCRATCHDAMAGE)) || defined(_VEH_ASSUME_SCRATCHDAMAGE)
#define _USE_SCRATCHDAMAGE
#endif
#if !defined(_NO_SWAP_DAMAGE) && defined(_VEH_ENABLE_SWAPDAMAGE)
#define _USE_SWAPDAMAGE
#endif
#if !defined(_NO_MORPH_TARGETS) && defined(_VEH_ENABLE_MORPHTARGETS)
#define _USE_MORPHTARGETS
#endif
#if !defined(_NO_CONTROL_POINTS) && defined(_VEH_ENABLE_CONTROL_POINTS)
#define _USE_CONTROL_POINTS
#endif

#if !defined(_MATERIAL_ALLOY)
#define _ENABLE_SHADER_DAMAGE
#endif

#if !defined(_MATERIAL_ALLOY)
#define _ENABLE_VERTEX_DEFORMATION
#define _ENABLE_MORPH_TARGETS
#endif

#if !defined(_MATERIAL_NEON) && !defined(LOD)
#define _ENABLE_RIM_LIGHT
#endif

#include <Shader/Core.h>
#include <Shader/Common/HDRRange.h> // [Blur2PC][HDR] Material range extension.
#undef SHNODE_IN_VERTEX_CONSTANT
#undef LODFADE_IN_VERTEX_CONSTANT
#include <Shader/Globals.h>

//REMOVE_DIRT
//#if !defined(LOD) && !defined(_MATERIAL_WININT)
//#define _ENABLE_DIRT
//#endif

#define REPLACE_MATERIAL
#define REPLACE_SURFACE
#define REPLACE_LIGHTING_FUNCTIONS


#include <Shader/Constants.h>
#include <Shader/Common/Defines.h>
#include <Shader/Common/Material.h>
#include <Shader/Common/Surface.h>
#include <Shader/Common/Fog.h>
#include <Shader/Common/Lighting.h>
#include <Shader/Lighting/SphericalHarmonics.h>

////////////////////////////////
//	Parameters
////////////////////////////////

// exposed vertex shader parameters
struct ExposedVertexShaderParams
{
	float3x4 mVehicleMatrix;
	float3x4 mVehicleInverseMatrix;
	float4 VehicleBoundsInfo;
	float4 RearLightColour;
};

shared ExposedVertexShaderParams g_VehicleVtxParams : register(CCV_SHADER_VEHICLE_0);

shared C_SHNode g_LocalSpaceSHNode : register(CCV_SHADER_VEHICLE_8) < string source = "VehicleSHNode"; int Stream = 1; bool Uninitialised = true; >; 

#if defined(_USE_CONTROL_POINTS)
shared float4 g_vControlPoints[18]			: register(CCV_SHADER_VEHICLE_16);
#endif

// exposed pixel shader parameters
struct ExposedPixelShaderParams
{
	float4 vAlphaScale;
	//REMOVE_DIRT
	//float4 vDirtBaseLayerSettings;
	//float4 vDirtDetailLayerSettings;
};

shared ExposedPixelShaderParams g_VehiclePixParams : register(CCP_SHADER_VEHICLE_0);

#if defined(_USE_SCRATCHDAMAGE)
shared float4 g_vUndercoatColour			: register(CCP_SHADER_VEHICLE_7);
shared float4 g_vScuffColour				: register(CCP_SHADER_VEHICLE_8);
#endif

shared float4 g_vDamageSettings				: register(CCV_SHADER_VEHICLE_14);	// also pixel shader. exposed per element

// global parameters
#if defined(_MATERIAL_WININT)
shared float4 g_vScreenSettingsVS			: register(CCV_SHADER_VEHICLE_15);// = float4(0.2f, 1.2f, 0.0f, 0.0f);//float4(yOffset(0.2f), yPinch(1.2f), 0.0f, 0.0f)
#endif

shared float4 g_vRenderSettings				: register(CCP_SHADER_VEHICLE_10);
shared float4 g_vRenderColour				: register(CCP_SHADER_VEHICLE_11) = { 0.0f, 0.0f, 0.0f, 1.0f };
#if defined(_MATERIAL_WININT)
shared float4 g_vScreenSettingsPS			: register(CCP_SHADER_VEHICLE_12);// = float4(2.0f, 30.0f, 1.0f, 0.1f);//float4(pow(2.0f), scale(30.0f), max(1.0f), minAlpha(0.1f));
#endif

struct MaterialParameters
{
	float4 vParameters;
	float4 vMaterialColour;
};

MaterialParameters g_vMtlParameters : register(CCP_SHADER_VEHICLE_13) < string Source = "0:BaseReflectivity.base_color, 1:BaseReflectivity.color1, 2:BumpTexture.factor, 3:DentTexture.factor, 4:BaseColour.base_color"; string DebugVariable = "MtlParameters";  bool Uninitialised = true; > ;

#define g_fMinReflectivity	g_vMtlParameters.vParameters.x
#define g_fMaxReflectivity	g_vMtlParameters.vParameters.y
#define g_vMaterialColour	g_vMtlParameters.vMaterialColour
#define g_fBumpDepth		g_vMtlParameters.vParameters.z
#define g_fDentDepth		g_vMtlParameters.vParameters.w

#define MAT_BUMP_DEPTH		g_vMtlParameters.vParameters.z

#if defined(_MATERIAL_LEGEND)
float4	g_MattParams : register(CCP_SHADER_VEHICLE_15 )	< string Source = "0:BaseMaterial_PaintM.specular, 3:BaseMaterial_PaintM.shiny"; string Default = "0.021, 0.021, 0.021, 0.5"; string DebugVariable = "MattParams"; bool Uninitialised = true; > =  { 0.0f, 0.0f, 0.0f, 1.0f };
#define MAT_MATT_INTENSITY			g_MattParams.rgb
#define MAT_MATT_POWER				g_MattParams.w
#endif
#if defined(_MATERIAL_MATT)
float4	g_MattParams : register(CCP_SHADER_VEHICLE_15 )	< string Source = "0:BaseMaterial_Matt.specular, 3:BaseMaterial_Matt.shiny"; string Default = "0.021, 0.021, 0.021, 0.5"; string DebugVariable = "MattParams"; bool Uninitialised = true; > =  { 0.0f, 0.0f, 0.0f, 1.0f };
#define MAT_MATT_INTENSITY			g_MattParams.rgb
#define MAT_MATT_POWER				g_MattParams.w
#endif
#if defined (_MATERIAL_ALLOY)
float4	g_MattParams : register(CCP_SHADER_VEHICLE_15 )	< string Source = "0:BaseMaterial_Alloy.specular, 3:BaseMaterial_Matt.shiny"; string Default = "0.088, 0.088, 0.088, 5.0"; string DebugVariable = "MattParams"; bool Uninitialised = true; > =  { 0.0f, 0.0f, 0.0f, 1.0f };
#define MAT_MATT_INTENSITY			g_MattParams.rgb
#define MAT_MATT_POWER				g_MattParams.w
#endif

#if defined(_MATERIAL_GLASS) || defined(_MATERIAL_WININT)
#define MAT_MIN_ALPHA			g_vRenderSettings.x
#define MAT_MAX_ALPHA			g_vRenderSettings.y
#define MAT_MIN_REFLEC			g_vRenderSettings.z
#define MAT_MAX_REFLEC			g_vRenderSettings.w
#define GLASS_DAMAGE_LEVEL		g_vDamageSettings.x;
#else
#define MAT_MIN_REFLEC			g_fMinReflectivity
#define MAT_MAX_REFLEC			g_fMaxReflectivity
#endif

#ifdef CFG_DEBUG
	#define _USE_DEBUG_VARIABLES
#endif

#ifdef _USE_DEBUG_VARIABLES

float g_DebugScratchHightFactor <string DebugVariable = "ScratchHightFactor"; string Range="(0,1)"; string Type="Float"; > = 0.5;
float g_DebugMinDamageReflection <string DebugVariable = "MinDamageReflection"; string Range="(0,4)"; string Type="Float"; > = 0.5;
float g_DebugOffsetDamageReflection <string DebugVariable = "OffsetDamageReflection"; string Range="(0,1)"; string Type="Float"; > = 0.1;
float g_DebugReflectionDamageReflectionFactor <string DebugVariable = "DebugReflectionDamageReflectionFactor"; string Range="(0,4)"; string Type="Float"; > = 1.0;
float g_DebugScratchIntensityFactor <string DebugVariable = "DebugScratchIntensityFactor"; string Range="(0,1)"; string Type="Float"; > = 0.1h;
float g_BreakGlassReflection <string DebugVariable = "BreakGlassReflection"; string Range="(0,1)"; string Type="Float"; > = 0.7f;
float g_BreakGlassAlphaPower <string DebugVariable = "BreakGlassAlphaPower"; string Range="(0,8)"; string Type="Float"; > = 5.0f;
float g_BreakGlassAlphaFactor <string DebugVariable = "BreakGlassAlphaFactor"; string Range="(0,8)"; string Type="Float"; > = 2.0f;

float g_DebugLightCubemapFactor <string DebugVariable = "LightCubemapFactor"; string Range="(0,1)"; string Type="Float"; > = 0.2;
float g_DebugMattLightCubemapFactor <string DebugVariable = "MattLightCubemapFactor"; string Range="(0,1)"; string Type="Float"; > = 0.02;
float g_fAOMin <string DebugVariable = "AOMin"; string Range="(0,1)"; string Type="Float"; > = 0.0;
float g_fAORange <string DebugVariable = "AORange"; string Range="(0,2)"; string Type="Float"; > = 2.0f;
float g_fAONormal <string DebugVariable = "AONormal"; string Range="(0,1)"; string Type="Float"; > = 1.0f;

float g_DebugCodeDamageScale <string DebugVariable = "CodeDamageScale"; string Range="(0,1)"; string Type="Float"; > = 1;
float g_DebugScratchScale <string DebugVariable = "ScratchScale"; string Range="(0,5)"; string Type="Float"; > = 2;
float g_DebugBumpScale <string DebugVariable = "BumpScale"; string Range="(0,5)"; string Type="Float"; > = 1;
float g_DebugDentScale <string DebugVariable = "DentScale"; string Range="(0,5)"; string Type="Float"; > = 1.5;
float g_DebugTextureSwapScale <string DebugVariable = "TextureSwapScale"; string Range="(0,1)"; string Type="Float"; > = 0;

float g_DebugScratchLevel1Scale <string DebugVariable = "ScratchLevel1Scale"; string Range="(0,5)"; string Type="Float"; > = 1;
float g_DebugScratchLevel2Scale <string DebugVariable = "ScratchLevel2Scale"; string Range="(0,5)"; string Type="Float"; > = 1;

float g_vScratchLevel1ColourR <string DebugVariable = "ScratchLevel1ColourR"; string Range="(0,1)"; string Type="Float"; > = 1;
float g_vScratchLevel1ColourG <string DebugVariable = "ScratchLevel1ColourG"; string Range="(0,1)"; string Type="Float"; > = 1;
float g_vScratchLevel1ColourB <string DebugVariable = "ScratchLevel1ColourB"; string Range="(0,1)"; string Type="Float"; > = 1;

float g_vScratchLevel2ColourR <string DebugVariable = "ScratchLevel2ColourR"; string Range="(0,1)"; string Type="Float"; > = 0;
float g_vScratchLevel2ColourG <string DebugVariable = "ScratchLevel2ColourG"; string Range="(0,1)"; string Type="Float"; > = 0;
float g_vScratchLevel2ColourB <string DebugVariable = "ScratchLevel2ColourB"; string Range="(0,1)"; string Type="Float"; > = 0;
float g_fPaintScratchSaturation <string DebugVariable = "ScratchLevel2Saturation"; string Range="(0,1)"; string Type="Float"; > = 0.75;


float g_DebugScratchDamageLevel <string DebugVariable = "DebugScratchDamageLevel"; string Range="(0,1)"; string Type="Float"; > = 0;
float g_DebugDentDamageLevel <string DebugVariable = "DebugDentDamageLevel"; string Range="(0,1)"; string Type="Float"; > = 0;

//float g_DebugMinDamageReflection <string DebugVariable = "MinDamageReflection"; string Range="(0,1)"; string Type="Float"; > = 0.5;

//REMOVE_DIRT
//#if defined(_MATERIAL_DEFAULT)
//float g_fMinBaseDustLevel <string DebugVariable = "MinBaseDustLevel"; string Range="(0,1)"; string Type="Float"; > = 0.5;
//#elif defined(_MATERIAL_ALLOY)
//float g_fMinBaseDustLevel <string DebugVariable = "MinBaseDustLevel"; string Range="(0,1)"; string Type="Float"; > = 0.85;
//#elif defined(_MATERIAL_PAINT)
//float g_fMinBaseDustLevel <string DebugVariable = "MinBaseDustLevel"; string Range="(0,1)"; string Type="Float"; > = 0.1;
//#elif defined(_MATERIAL_GLASS)
//float g_fMinBaseDustLevel <string DebugVariable = "MinBaseDustLevel"; string Range="(0,1)"; string Type="Float"; > = 0.75;
//#else
//float g_fMinBaseDustLevel <string DebugVariable = "MinBaseDustLevel"; string Range="(0,1)"; string Type="Float"; > = 0.5;
//#endif
//
//float g_fDirtRepeats <string DebugVariable = "DirtRepeats"; string Range="(0,50)"; string Type="Float"; > = 1;
//float g_fMinDirtReflection <string DebugVariable = "DirtMinReflection"; string Range="(0,1)"; string Type="Float"; > = 0;
float g_fUberDamageScale <string DebugVariable = "UberDamageScale"; string Range="(0,1)"; string Type="Float"; > = 1;

float g_DebugRearLight  <string DebugVariable = "DebugRearLight"; string Range="(0,10)"; string Type="Float"; > = 0.4f;

#else
static real g_DebugScratchHightFactor = 0.5h;
static real g_DebugMinDamageReflection = 0.5h;
static real g_DebugOffsetDamageReflection = 0.1h;
static real g_DebugReflectionDamageReflectionFactor = 1.0h;
static real g_DebugScratchIntensityFactor = 0.1h;
static real g_BreakGlassReflection  = 0.7h;
static real g_BreakGlassAlphaPower = 5.h; 
static real g_BreakGlassAlphaFactor = 2.h;

static real g_DebugLightCubemapFactor = 0.2h;
static real g_DebugMattLightCubemapFactor = 0.02h;
static real g_fAOMin = 0.0h;
static real g_fAORange = 2.0h;
static real g_fAONormal = 1.0h;

static real g_DebugCodeDamageScale = 1.0h;
static real g_DebugScratchScale = 2.0h;
static real g_DebugBumpScale = 1.0h;
static real g_DebugDentScale = 1.5h;
static real g_DebugTextureSwapScale = 0.0h;

static real g_DebugScratchLevel1Scale = 1.0h;
static real g_DebugScratchLevel2Scale = 1.0h;

static real g_vScratchLevel1ColourR = 1.0h;
static real g_vScratchLevel1ColourG = 1.0h;
static real g_vScratchLevel1ColourB = 1.0h;

static real g_vScratchLevel2ColourR = 0.0h;
static real g_vScratchLevel2ColourG = 0.0h;
static real g_vScratchLevel2ColourB = 0.0h;
static real g_fPaintScratchSaturation = 0.75h;

static real g_DebugScratchDamageLevel = 0.0h;
static real g_DebugDentDamageLevel = 0.0h;

//static real g_DebugMinDamageReflection = 0.5h;

//REMOVE_DIRT
//static real g_fMinBaseDustLevel = 0.1h;
//
//static real g_fDirtRepeats = 1.0h;
//static real g_fMinDirtReflection = 0.0h;
static real g_fUberDamageScale = 1.0h;

static real g_DebugRearLight = 0.4h;

#endif

float g_vCodeDamageScale < string source = "CodeDamageScale"; int Stream = 1; bool Uninitialised = true; >;
//float g_vCodeDamageScaleParam < string source = "CodeDamageScaleParam"; >;

#define SCRATCH_SCALE			g_DebugScratchScale
//#define DAMAGE_OFFSET_SCALE		g_vDebugDamageSettings.x// * g_DebugCodeDamageScale
//#define DAMAGE_OFFSET_SCALE		g_vCodeDamageScale// * g_DebugCodeDamageScale
#define DAMAGE_OFFSET_SCALE		g_vCodeDamageScale// * g_DebugCodeDamageScale

////////////////////////////////
//	Structures
////////////////////////////////
//--------------------------------------------------------------------------------------
// Vertex shader input structure
//--------------------------------------------------------------------------------------
struct VSInput
{
	float4 vPos							: POSITION annotation(format(F16_4));
	float3 vNormal						: NORMAL;

#if !defined(_MATERIAL_NEON) && !defined(_MATERIAL_LIGHT)
	#if defined(_USE_ALBEDO) || defined(_USE_DENT) || defined(_USE_SCRATCHDAMAGE) || defined(_USE_SWAPDAMAGE)
		#if defined(_USE_SWAPDAMAGE)
			float4 vTexCoord0			: TEXCOORD0 annotation(xy(uv:BaseTexture) | zw(uv:TextureDamage)	| format(F16_4));
		#else
			float4 vTexCoord0			: TEXCOORD0 annotation(xy(uv:BaseTexture) | zw(uv:ScratchTexture)	| format(F16_4));
		#endif
	#endif
	#if defined(_USE_BUMP)
	float2 vTexCoord1					: TEXCOORD1 annotation(xy(uv:BumpTexture) | format(F16_2));
	#endif	
#else
	#if defined(_MATERIAL_LIGHT)
		float4 vTexCoord0				: TEXCOORD0 annotation(xy(uv:OffTexture) | zw(uv:OnTexture)	| format(F16_4));
		float4 vTexCoord1				: TEXCOORD1 annotation(xy(uv:OffTextureDamage) | zw(uv:OnTextureDamage)	| format(F16_4));
	#else
		float4 vTexCoord0				: TEXCOORD0 annotation(xy(uv:BaseTexture) | zw(uv:TextureDamage) | format(F16_4));
	#endif
#endif

#if defined(_ENABLE_SHADER_DAMAGE)
#if !defined(LOD)
#if defined(_USE_MORPHTARGETS)
	float3 vPositionOffset				: TEXCOORD2 annotation(xyz(positionOffset) | format(SHHD_3N) | define(h));
	float3 vNormalOffset				: TEXCOORD3 annotation(xyz(normalOffset) | format(SHHD_3N) | define(h));
#endif
#endif
#if defined(_USE_CONTROL_POINTS)
	float4 vControlPointInfo			: TEXCOORD4 annotation(xyzw(controlPointInfo) | format(U8_4) | define(c));
#endif

#if !defined(_MATERIAL_ALLOY)
	float4 vSPUPos						: TANGENT annotation(stream(3) | format(F16_4));
	float4 vSPUNormal					: BINORMAL annotation(stream(3) | format(F16_4));
#endif 

#endif
	float4	m_vAmbientOcclusionSH		: COLOR0 annotation(xyzw(colour:luxVertexColour) | format(U8_4N));
};

//--------------------------------------------------------------------------------------
// Vertex shader output structure
//--------------------------------------------------------------------------------------
struct VSOutput
{
	float4 vPosition		: POSITION;	
	real4 vNormal			: TEXCOORD0;
#if defined(_USE_ALBEDO) || defined(_USE_SCRATCHDAMAGE) || defined(_USE_SWAPDAMAGE) || (defined(_USE_BUMP) && !defined(_MATERIAL_NEON) && !defined(LOD)) || (defined(_USE_DENT) && !defined(_MATERIAL_NEON) && !defined(LOD)) || defined(_MATERIAL_LIGHT)
	float4 vTextureUV0		: TEXCOORD1;
#endif
	float4 vWorldPos		: TEXCOORD2;

#if defined(_MATERIAL_LIGHT) || (!defined(_MATERIAL_NEON) && defined(_USE_BUMP))
	float4 vTextureUV1		: TEXCOORD3;
#endif

	real4 vLighting		: TEXCOORD4;
};

//--------------------------------------------------------------------------------------
// Pixel shader output structure
//--------------------------------------------------------------------------------------
struct PSOutput
{
	real4 colour : COLOR0;  // Pixel color    
};

//////////////////////////////////////////////////////////////////////////
// Surface Properties
//////////////////////////////////////////////////////////////////////////
struct SurfaceProperties
{
	real4		m_vAlbedoTex;
	real4		m_vDamageTex;
#if !defined(_MATERIAL_MATT)
	real4		m_vReflectionTex;
#endif
	//REMOVE_DIRT
	//real4		m_vDirtTex;
	real4		m_vPaintTex;

#ifdef _MATERIAL_LIGHT
	real4		m_vOffTexture;
	real4		m_vOnTexture;
	real4		m_vOffTextureDamage;
	real4		m_vOnTextureDamage;
#endif

//#ifdef _MATERIAL_PAINT
	real4		m_vDamage;
//#endif

	real4		m_vDamageColour;

	real4		m_vSurfaceColour;
	real4		m_vNormal;
	float3		m_vWorldPos;
	float		m_fDistance;
	real3		m_vView;
	real4		m_vReflect;
	real4		m_vLighting;

	real		m_fReflectivity;
	real		m_fAmbientOcclusion;
	real4		m_vVertexLighting;
	real		m_fNdotV;
	real		m_fDamageLevel;
};

////////////////////////////////
// Texture samplers
////////////////////////////////

#if defined(_USE_ALBEDO)
	#if defined(_VEH_ASSUME_ALBEDO)
texture g_MeshTexture			< string source = "BaseTexture"; bool Uninitialised = true; >;
	#else
texture g_MeshTexture			< string source = "BaseTexture"; bool Uninitialised = true; string Define="a"; >;
	#endif
#endif

#if defined(_USE_BUMP)
	#if defined(_VEH_ASSUME_BUMP)
texture g_BumpTexture			< string source = "BumpTexture"; bool Uninitialised = true; >;
	#else
texture g_BumpTexture			< string source = "BumpTexture"; bool Uninitialised = true; string Define="b"; >;
	#endif
#endif 

#if defined(_USE_SCRATCHDAMAGE)
	#if defined(_VEH_ASSUME_SCRATCHDAMAGE)
texture g_ScratchTexture		< string source = "ScratchTexture"; bool Uninitialised = true; >;
	#else
texture g_ScratchTexture		< string source = "ScratchTexture"; bool Uninitialised = true; string Define="f"; >;
	#endif
#endif

#if defined(_USE_SWAPDAMAGE)
texture g_DamageTexture			< string source = "TextureDamage"; bool Uninitialised = true; string Define="g"; >;
#endif 

#if defined(_USE_DENT)
	#if defined(_VEH_ASSUME_DENT)
texture g_DentTexture			< string source = "DentTexture"; bool Uninitialised = true; >;
	#else
texture g_DentTexture			< string source = "DentTexture"; bool Uninitialised = true; string Define="d"; >;
	#endif
#endif 

#if defined(_MATERIAL_LIGHT)
texture g_sOffTexture			< string source = "OffTexture"; bool Uninitialised = true; string Define="ly"; >;
texture g_sOnTexture			< string source = "OnTexture"; bool Uninitialised = true; string Define="ln"; >;
texture g_sOffTextureDamage		< string source = "OffTextureDamage"; bool Uninitialised = true; string Define="lnd"; >;
texture g_sOnTextureDamage		< string source = "OnTextureDamage"; bool Uninitialised = true; string Define="lyd"; >;

sampler2D OffTextureSampler : register(s7) = 
sampler_state
{
	Texture = <g_sOffTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	SRGB_TEXTURE
};


sampler2D OnTextureSampler : register(s9) = 
sampler_state
{
	Texture = <g_sOnTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	SRGB_TEXTURE
};


sampler2D OffDamageTextureSampler : register(s10) = 
sampler_state
{
	Texture = <g_sOffTextureDamage>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	SRGB_TEXTURE
};


sampler2D OnDamageTextureSampler : register(s15) = 
sampler_state
{
	Texture = <g_sOnTextureDamage>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	SRGB_TEXTURE
};

#endif

#if !defined(_MATERIAL_MATT)
shared texture g_sVehicleEnvironmentTexture;
#endif
shared texture g_sVehicleDamageInfoTexture;
shared texture g_sVehicleDamageVolumeTexture;
//REMOVE_DIRT
//shared texture g_sVehicleDirtLayerTexture;

shared texture g_sLightEnvironmentTexture;

#if defined(_MATERIAL_PAINT)
shared texture g_sPaintMaterialTexture;
#endif

#if defined(_MATERIAL_WININT)
shared texture g_tBloomBuffer;
sampler2D BloomTextureSampler : register(s6) = 
sampler_state
{
	Texture = <g_tBloomBuffer>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};
#endif

#if defined(_USE_ALBEDO)
sampler2D MeshTextureSampler : register(s7) = 
sampler_state
{
	Texture = <g_MeshTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	SRGB_TEXTURE
};
#endif

#if defined(_USE_BUMP)
sampler2D BumpTextureSampler : register(s8)  = 
sampler_state
{
	Texture = <g_BumpTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};
#endif

#if defined(_USE_SCRATCHDAMAGE)
sampler2D ScratchTextureSampler : register(s9)  = 
sampler_state
{
	Texture = <g_ScratchTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};
#endif

#if defined(_USE_SWAPDAMAGE)
sampler2D DamageTextureSampler : register(s10)  = 
sampler_state
{
	Texture = <g_DamageTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};
#endif

#if defined(_USE_DENT)
sampler2D DentTextureSampler : register(s12) = 
sampler_state
{
	Texture = <g_DentTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};
#endif

#if defined(_MATERIAL_PAINT)
sampler2D PaintTextureSampler : register(s13)  = 
sampler_state
{
	Texture = <g_sPaintMaterialTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	SRGB_TEXTURE
};
#endif


#if !defined(_MATERIAL_MATT)
samplerCUBE EnvironmentTextureSampler : register(s14)  = 
sampler_state
{
	Texture = <g_sVehicleEnvironmentTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};
#endif

samplerCUBE LightEnvironmentTextureSampler : register(s5)  = 
sampler_state
{
	Texture = <g_sLightEnvironmentTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
	SRGB_TEXTURE
};

#define _DAMAGE_USE_3D_TEX

#ifdef _DAMAGE_USE_3D_TEX
sampler3D DamageVolumeTextureSampler : register(s0)  = 
sampler_state
{
	Texture = <g_sVehicleDamageVolumeTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};
#else
sampler2D DamageVolumeTextureSampler : register(s0)  = 
sampler_state
{
	Texture = <g_sVehicleDamageVolumeTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};
#endif

sampler2D DamageInfoTextureSampler : register(s1)  = 
sampler_state
{
	Texture = <g_sVehicleDamageInfoTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

#if defined(PLATFORM_PS3) && defined(__PIXEL__)
#pragma texformat g_MeshTexture COMPRESSED_RGBA_S3TC_DXT5
#pragma texformat g_BumpTexture COMPRESSED_RGBA_S3TC_DXT1
#pragma texformat g_ScratchTexture COMPRESSED_RGBA_S3TC_DXT1
#pragma texformat g_DamageTexture COMPRESSED_RGBA_S3TC_DXT1
#pragma texformat g_DentTexture COMPRESSED_RGBA_S3TC_DXT1
#pragma texformat g_sOffTexture	COMPRESSED_RGBA_S3TC_DXT5	
#pragma texformat g_sOnTexture COMPRESSED_RGBA_S3TC_DXT5	
#pragma texformat g_sOffTextureDamage COMPRESSED_RGBA_S3TC_DXT5
#pragma texformat g_sOnTextureDamage COMPRESSED_RGBA_S3TC_DXT5
#pragma texformat g_sPaintMaterialTexture RGBA8
#pragma texformat g_sLightEnvironmentTexture RGBA8
#pragma texformat g_sVehicleEnvironmentTexture RGBA8
#pragma texformat g_sVehicleDamageVolumeTexture RGBA8
#pragma texformat g_sVehicleDamageInfoTexture RGBA8
#endif

///////////////////////////////////////////
//	Shared functions
///////////////////////////////////////////

float2 GetDamageUV(float3 position)
{
	//The tex coords
	float2 uv = float2(0.0f, 0.0f);
	//Get u coord
	uv.x = clamp((position.x + g_VehicleVtxParams.VehicleBoundsInfo.x / 2.0f) / g_VehicleVtxParams.VehicleBoundsInfo.x, 0.0f, 1.0f);
	//Get v coord
	uv.y = clamp((position.z + g_VehicleVtxParams.VehicleBoundsInfo.z / 2.0f) / g_VehicleVtxParams.VehicleBoundsInfo.z, 0.0f, 1.0f);

	return uv;
}

float4 EvaluateDamageInfoTexture(float2 uv, float yPos)
{
	float4 damageInfo = tex2Dlod(DamageInfoTextureSampler, float4(uv.x, uv.y, 0.0f, 0.0f));
	damageInfo.x = clamp(yPos / (damageInfo.x * g_VehicleVtxParams.VehicleBoundsInfo.y), 0.0f, 1.0f);
	return damageInfo;
}

float4 EvaluateVertexDamage(float3 uvw)
{
	//Get damage vector information from array	
#ifdef _DAMAGE_USE_3D_TEX
	return tex3Dlod(DamageVolumeTextureSampler, float4(uvw.x, uvw.y, uvw.z, 0.0f));
#else
	return tex2Dlod(DamageVolumeTextureSampler, float4(uvw.x, uvw.y, 0.0f, 0.0f));
#endif
}

float4 EvaluateDamageOffset(float3 position, float3 impactVector)
{	
#ifndef _DAMAGE_USE_F32
	impactVector.xyz = impactVector.xyz * 4.0f - 2.0f;
#endif
	return float4(position.x + impactVector.x * DAMAGE_OFFSET_SCALE, position.y + impactVector.y * DAMAGE_OFFSET_SCALE, position.z + impactVector.z * DAMAGE_OFFSET_SCALE, 1.0f);
}

float3 EvaluateDamageOffset(float3 position)
{
	position = mul(g_VehicleVtxParams.mVehicleInverseMatrix, position.xyz);

	//The tex coords
	float3 uv = float3(0.0f, 0.0f, 0.0f);

	//Get u coord
	uv.x = clamp((position.x + g_VehicleVtxParams.VehicleBoundsInfo.x / 2.0f) / g_VehicleVtxParams.VehicleBoundsInfo.x, 0.0f, 1.0f);

	//Get v coord
	uv.y = clamp((position.z + g_VehicleVtxParams.VehicleBoundsInfo.z / 2.0f) / g_VehicleVtxParams.VehicleBoundsInfo.z, 0.0f, 1.0f);

	//Get w coord
	uv.z = clamp((position.y - g_VehicleVtxParams.VehicleBoundsInfo.w) / g_VehicleVtxParams.VehicleBoundsInfo.y, 0.0f, 1.0f);

	//Get damage vector information from array		
	//float4 dam = float4(0.0f, 0.0f, 0.0f, 0.0f);
#ifdef _DAMAGE_USE_3D_TEX
	float4 dam = tex3Dlod(DamageVolumeTextureSampler, float4(uv.x, uv.y, uv.z, 0.0f));
#else
	float4 dam = tex2Dlod(DamageVolumeTextureSampler, float4(uv.x, uv.y, 0.0f, 0.0f));
#endif

#if defined (_ENABLE_VERTEX_DEFORMATION)
	dam.xyz -= 0.5f;

	//Calculate the direction and scale of the impact vector
	float3 impactVector = float3(dam.x, dam.y, dam.z);

	float3 newPos = float3(position.x + impactVector.x, position.y + impactVector.y, position.z + impactVector.z);

	newPos = mul(g_VehicleVtxParams.mVehicleMatrix, newPos.xyz);
#else
	float3 newPos = position;
#endif
	return newPos;
}

real EvaluateAmbientOcclusion(VSOutput input, SurfaceProperties surface)
{
#if !defined(_MATERIAL_NEON)
	return surface.m_vVertexLighting.w;
#else
	return 1.0h;
#endif
}

#if !defined(_MATERIAL_MATT)
real EvaluateReflectivity(SurfaceProperties surface)
{
	real reflectivity = 0.0h;

#if defined(_MATERIAL_PAINT)
	reflectivity = surface.m_fReflectivity;
#else
	reflectivity = lerp(MAT_MAX_REFLEC, MAT_MIN_REFLEC, surface.m_fNdotV);
#endif

#if defined(_MATERIAL_GLASS) && defined(_USE_ALBEDO)
	reflectivity *= (1.0h - 0.9h * saturate(surface.m_vAlbedoTex.w * 4.0h));
	//reflectivity = lerp(reflectivity, reflectivity * 0.1h, saturate(surface.m_vAlbedoTex.w * 4.0h));
#endif

#if defined(_MATERIAL_ALLOY) && defined(_USE_ALBEDO)
	reflectivity *= surface.m_vAlbedoTex.w;
#endif

#if defined(_USE_SCRATCHDAMAGE)
	#if defined(_MATERIAL_GLASS)
	real fBlend = g_BreakGlassReflection * saturate(pow(surface.m_vDamageTex.y * g_BreakGlassAlphaFactor, g_BreakGlassAlphaPower));
	return reflectivity * (1.0h - fBlend);
	#else
	return reflectivity * (1.0h - saturate(surface.m_fDamageLevel * max(surface.m_vDamageTex.r * g_DebugMinDamageReflection + g_DebugOffsetDamageReflection, surface.m_vDamageTex.g)  + surface.m_vDamage.w * g_DebugReflectionDamageReflectionFactor));
	#endif	
#else
	return reflectivity;
#endif
}
#endif

real3 EvaluateLighting(SurfaceProperties surface)
{
	real3 color = surface.m_vVertexLighting;
	return color.xyz * g_vBakedColour.xyz * g_fDiffuseIntensity;
}

real3 EvaluateNormal(VSOutput input, real scratchLevel)
{	
#if defined(LOD)
	return normalize(input.vNormal.xyz);
#else

	real3 vNormal = normalize(input.vNormal.xyz);

#if defined(_MATERIAL_PAINT) && defined(_USE_SCRATCH_DAMAGE)
	vNormal.y += scratchLevel * 0.5h;
#endif

#if defined(_USE_DENT) && !defined(_MATERIAL_NEON)
#if defined(_MATERIAL_GLASS)
	real3	dentTex		= h4tex2D(DentTextureSampler, input.vTextureUV0.zw).rgb;
	real	dentScale	= -4.0h * saturate(input.vNormal.w);
//	real	dentScale	= g_fUberDamageScale * clamp(g_DebugDentDamageLevel + 2.0h * input.vNormal.w, 0.0h, 2.0h) * -1.0h;
#else
	real3	dentTex		= h4tex2D(DentTextureSampler, input.vTextureUV0.zw).rgb;
	real	dentScale	= -3.0h * saturate(input.vNormal.w);
//	real	dentScale	= g_fUberDamageScale * clamp(g_DebugDentDamageLevel + g_DebugDentScale * input.vNormal.w, 0.0h, g_DebugDentScale) * -1.0h;
#endif
	real3	dent		= (dentTex - (0.5h - 0.001953125h)) * dentScale;
	real3	dentNormal	= normalize(vNormal.xyz + 0.5h * (dent.x * vNormal.xzy - dent.y * vNormal.yxz));
#if !defined(_USE_BUMP)
	return normalize(dentNormal);
#endif

#endif

#if defined(_USE_BUMP) && !defined(_MATERIAL_NEON)
	float2	bumpUV		= input.vTextureUV1.xy;
	real2	bumpTex		= h4tex2D(BumpTextureSampler, bumpUV).xy;	
	real2	bump		= (bumpTex - (0.5h - 0.001953125h)) * 2.0h * g_DebugBumpScale * MAT_BUMP_DEPTH;
#if defined(_USE_DENT) && !defined(_MATERIAL_NEON)
	return normalize(dentNormal + 0.5h * (bump.x * vNormal.xzy - bump.y * vNormal.yxz));
#else
	return normalize(vNormal.xyz + 0.5h * (bump.x * vNormal.xzy - bump.y * vNormal.yxz));
#endif

#else
	return normalize(vNormal.xyz);
#endif
#endif //LOD
}

 #ifdef _USE_DEBUG_VARIABLES
 	float g_DebugRimLightPower  <string DebugVariable = "Power"; string Range="(0,10)"; string Type="Float"; > = 15;
 	float g_DebugRimLightScale  <string DebugVariable = "Scale"; string Range="(0,10)"; string Type="Float"; > = 2;
 
 	float g_DebugDynamicRimLightPower  <string DebugVariable = "DynamicPower"; string Range="(0,40)"; string Type="Float"; > = 30;
 	float g_DebugDynamicRimLightScale  <string DebugVariable = "DynamicScale"; string Range="(0,40)"; string Type="Float"; > = 3;
 
 	float g_DebugRimFadeRange <string DebugVariable = "RimFade"; string Range="(0,1)"; string Type="Float"; > = 0.1;
 	float g_DebugRimFadeOffsetRange <string DebugVariable = "RimFadeOffset"; string Range="(0,1)"; string Type="Float"; > = 0.1;
 #else
	static real g_DebugRimLightPower  = 15.0h;
	static real g_DebugRimLightScale  = 1.0h;

	static real g_DebugDynamicRimLightPower  = 30.0h;
	static real g_DebugDynamicRimLightScale  = 3.0h;

	static real g_DebugRimFadeRange  = 0.1h;
	static real g_DebugRimFadeOffsetRange  = 0.1h;

#endif

real3 EvaluateFinalColour(SurfaceProperties surface, real alpha)
{
	real rimFade = saturate(surface.m_fNdotV / g_DebugRimFadeRange) * (1.0h - g_DebugRimFadeOffsetRange) + g_DebugRimFadeOffsetRange ;

#if defined(_MATERIAL_NEON)
	real3 neon			= surface.m_vSurfaceColour.xyz * g_vRenderColour.w;
	real3 lighting		= surface.m_vLighting.xyz * surface.m_vSurfaceColour.xyz;
	real3 finalColour	= lerp(lighting, neon, surface.m_vSurfaceColour.w) + AdjustCubemapIntensity(surface.m_vReflectionTex.xyz) * rimFade * surface.m_fReflectivity * surface.m_fAmbientOcclusion;
#elif defined(_MATERIAL_LIGHT)
	real3 finalColour	= lerp(surface.m_vSurfaceColour.xyz * surface.m_vLighting.xyz, surface.m_vSurfaceColour.xyz, surface.m_vSurfaceColour.w) + AdjustCubemapIntensity(surface.m_vReflectionTex.xyz) * rimFade * surface.m_fReflectivity * surface.m_fAmbientOcclusion;
#else
	
#if defined(_MATERIAL_GLASS) || defined(_MATERIAL_WININT)
	surface.m_vSurfaceColour.xyz *= alpha;
#endif

#if !defined(_MATERIAL_MATT)
	#ifdef _MATERIAL_GLASS
	real3 finalColour	= surface.m_vLighting.xyz * surface.m_vSurfaceColour.xyz * (1.0h - surface.m_fReflectivity) + surface.m_fReflectivity * AdjustCubemapIntensity(surface.m_vReflectionTex.xyz) * rimFade * surface.m_fAmbientOcclusion * g_VehiclePixParams.vAlphaScale.x;
	#else
	real3 finalColour	= surface.m_vLighting.xyz * surface.m_vSurfaceColour.xyz * (1.0h - surface.m_fReflectivity) + surface.m_fReflectivity * AdjustCubemapIntensity(surface.m_vReflectionTex.xyz) * rimFade * surface.m_fAmbientOcclusion;
	#endif
#else
	real3 finalColour	= surface.m_vLighting.xyz * surface.m_vSurfaceColour.xyz;
#endif

#if defined(_ENABLE_RIM_LIGHT)

	#if !defined(_MATERIAL_MATT) && !defined(_MATERIAL_ALLOY)
	finalColour.xyz += surface.m_vLighting.xyz * saturate(surface.m_vNormal.y * 0.75h + 0.25h) * pow(1.0h - surface.m_fNdotV, g_DebugRimLightPower) * g_DebugRimLightScale * rimFade;
	#endif

	#ifndef LOD

	real fresnel = pow(1.0h - surface.m_fNdotV, g_DebugDynamicRimLightPower) * g_DebugDynamicRimLightScale * rimFade * saturate(surface.m_vNormal.y * 0.75h + 0.25h) * surface.m_fAmbientOcclusion;
	real4 lightCubeSample = h4texCUBE(LightEnvironmentTextureSampler, surface.m_vReflect.xyz).rgba;
	real3 lightCubeValue = lightCubeSample.rgb * (lightCubeSample.a * 8.0h);

#if !defined(_MATERIAL_MATT) && !defined(_MATERIAL_ALLOY)
 	finalColour.xyz += lightCubeValue * (fresnel * surface.m_fReflectivity);
 	#if defined(_MATERIAL_PAINT) && defined(_USE_SCRATCHDAMAGE)
 	real3 lcm = lightCubeValue;
 	//finalColour.xyz += (lcm * surface.m_vDamage.w * 0.5h + 1.0h - surface.m_vDamage.w) * lcm * (surface.m_fAmbientOcclusion * g_DebugMattLightCubemapFactor);
	real factor = saturate((surface.m_fDamageLevel * surface.m_vDamageTex.g  - g_DebugScratchHightFactor) / (1.f - g_DebugScratchHightFactor));
	factor = factor * factor;
	finalColour.xyz += factor * lcm * lcm * surface.m_fAmbientOcclusion * g_DebugScratchIntensityFactor;
 	#endif

#if defined(_MATERIAL_LEGEND)
	//Add matt factor
	finalColour.xyz += pow((lightCubeValue), MAT_MATT_POWER) * (surface.m_fAmbientOcclusion * MAT_MATT_INTENSITY * (surface.m_vAlbedoTex.a));
#endif
// 
//  		#ifdef _MATERIAL_GLASS
//  		finalColour.xyz += saturate(lightCubeValue * lightCubeValue) * (surface.m_fAmbientOcclusion * g_DebugLightCubemapFactor) * g_VehiclePixParams.vAlphaScale.x;
//  		#else
//  		finalColour.xyz += saturate(lightCubeValue * lightCubeValue) * (surface.m_fAmbientOcclusion * g_DebugLightCubemapFactor);
//  		#endif
//  	#endif
#else
	finalColour.xyz += pow((lightCubeValue), MAT_MATT_POWER) * (surface.m_fAmbientOcclusion * MAT_MATT_INTENSITY * surface.m_fReflectivity);
#endif

	#endif
#endif
#endif

	finalColour		= EvaluateDepthFog(real4(finalColour, 0.0h), surface.m_fDistance).rgb;

	return finalColour;
}

#if defined(_MATERIAL_PAINT)
real4 EvaluateSurfaceColour(SurfaceProperties surface)
{
	real4 paintTex = surface.m_vPaintTex;

#if defined(_USE_ALBEDO)
	real4 surfaceColour			= real4(lerp(paintTex.xyz, surface.m_vAlbedoTex.xyz, surface.m_vAlbedoTex.w), 0.0h);
#else
	real4 surfaceColour			= real4(paintTex.xyz, 0.0h);
#endif

	surfaceColour.w = paintTex.w;

#if defined(_USE_SCRATCHDAMAGE)
	surfaceColour.xyz				= lerp(surfaceColour.xyz, surface.m_vDamage.xyz, surface.m_vDamage.w * g_fUberDamageScale);
#elif defined(_USE_SWAPDAMAGE)
	surfaceColour.xyz				= lerp(surfaceColour.xyz, surface.m_vDamageTex.xyz, g_vDamageSettings.x);
#endif

	return surfaceColour;
}
#elif defined(_MATERIAL_NEON)
real4 EvaluateSurfaceColour(SurfaceProperties surface)
{
#if defined(_USE_ALBEDO)
	#if defined(_USE_SWAPDAMAGE)
	return real4(lerp(surface.m_vAlbedoTex.xyz, surface.m_vDamageTex.xyz, g_vDamageSettings.x), surface.m_vAlbedoTex.w);
	#else
	return real4(surface.m_vAlbedoTex.xyzw);
	#endif
#else
	return 1.0h;
#endif
}
#elif defined(_MATERIAL_LIGHT)
real4 EvaluateSurfaceColour(SurfaceProperties surface)
{
	return lerp(surface.m_vAlbedoTex.xyzw, surface.m_vDamageTex.xyzw, g_vDamageSettings.x);
}
#else
real4 EvaluateSurfaceColour(SurfaceProperties surface)
{
#if defined(_MATERIAL_ALLOY)
	real4 materialColour = real4(g_vRenderColour.xyz, 1.0h);
#elif defined(_MATERIAL_GLASS)		
	real4 materialColour = real4(g_vMaterialColour.xyz + g_vRenderColour.xyz, 1.0h);
#else
	real4 materialColour = real4(g_vMaterialColour.xyz, 1.0h);
#endif

#if defined(_USE_ALBEDO)
	real4 surfaceColour			= real4(lerp(materialColour.xyz, surface.m_vAlbedoTex.xyz, surface.m_vAlbedoTex.w), 1.0h);
#else
	real4 surfaceColour			= real4(materialColour.xyz, 1.0h);
#endif

#if defined(_USE_SCRATCHDAMAGE)
	#if defined(_MATERIAL_GLASS)
	real fBlend					= max(max(surface.m_vDamageTex.x, surface.m_vDamageTex.y), surface.m_vDamageTex.z);
	surfaceColour				= real4(lerp(surfaceColour.xyz, surface.m_vDamageTex.xyz, fBlend), 1.0h);
	#else
	surfaceColour.xyz			= lerp(surfaceColour.xyz, surface.m_vDamageTex.yyy, surface.m_vDamageTex.y * g_fUberDamageScale);
	#endif
#elif defined(_USE_SWAPDAMAGE)
	surfaceColour				= real4(lerp(surfaceColour.xyz, surface.m_vDamageTex.xyz, g_vDamageSettings.x), 1.0h);
#endif

	return surfaceColour;
}
#endif

real EvaluateAlpha(SurfaceProperties surface)
{
	real alpha		= 1.0h;
#if !defined(LOD) //LOD
#if defined(_MATERIAL_ALLOY) && !defined(_MATERIAL_NEON)
	#if defined(_USE_ALBEDO)
		alpha = surface.m_vAlbedoTex.w * g_vRenderColour.w;
	#else
		alpha = g_vRenderColour.w;
	#endif
#elif !defined(_MATERIAL_NEON)
#if (defined(_USE_ALBEDO) && defined(_MATERIAL_ALPHA)) || defined(_MATERIAL_NEON)
	alpha = surface.m_vAlbedoTex.w;
#elif defined(_MATERIAL_GLASS)
	alpha = lerp(MAT_MAX_ALPHA, MAT_MIN_ALPHA, surface.m_fNdotV);
#if defined(_USE_ALBEDO)
	alpha = saturate(alpha + surface.m_vAlbedoTex.w);
#endif
#endif

#if defined(_USE_SCRATCHDAMAGE) && defined(_MATERIAL_GLASS)
	real fBlend = saturate(dot(1.0h, surface.m_vDamageTex.xyz) - 1.0h);
	alpha = saturate(alpha + fBlend);
#endif
#endif
#else //LOD
	#if defined(_MATERIAL_GLASS)
		#if !defined(_USE_ALBEDO)
	alpha = lerp(MAT_MAX_ALPHA, MAT_MIN_ALPHA, surface.m_fNdotV);
		#endif
	#endif
#endif //LOD

	return alpha * g_VehiclePixParams.vAlphaScale.x;
}

/////////////////////////
// Vertex Shader
/////////////////////////
VSOutput MainVS( VSInput input )
{
	VSOutput output			= (VSOutput)0;

#if !PLATFORM_PS3 || defined(_MATERIAL_ALLOY)	// no SPU damage

	float4 position			= input.vPos;
		   position.w = 1.f;
	float3 vNormal			= input.vNormal.xyz;

#if defined(_ENABLE_SHADER_DAMAGE)
#if !defined(PLATFORM_PS3)
	float3 localPos		= position.xyz;
#else
	float3 localPos		= position.xyz;
#endif

#if defined(_EXPORT_DAMAGE_HACK) && !defined(PLATFORM_PS3)
#if !defined(_MATERIAL_ALLOY)
	localPos.xyz		+= input.vSPUPos.xyz + input.vSPUNormal.xyz;
#endif 
#endif

#ifdef _USE_CONTROL_POINTS
 	float3 blendPos = lerp(g_vControlPoints[input.vControlPointInfo.x], g_vControlPoints[input.vControlPointInfo.y], (float)input.vControlPointInfo.z/254.0f);
 	localPos.xyz += blendPos.xyz;
#endif

#if !defined(LOD) //LOD

#if !defined(PLATFORM_PS3) // skip this stuff, now done on SPUs.
	float2 damageUV		= GetDamageUV(localPos.xyz);
	float4 damageInfo	= EvaluateDamageInfoTexture(damageUV, localPos.y);
	float4 damage		= EvaluateVertexDamage(float3(damageUV.xy, damageInfo.x));
	damage.xyz += 0.00196f;
	float morphTargetLevel	= damageInfo.y;//damage.w;
#else
#if !defined(_MATERIAL_ALLOY)
	float4 damage		= input.vDamageOffset;
#else
	float4 damage		= 0.0f;
#endif

	float4 damageInfo	= 0.0f;
		   damageInfo.y	=  damage.w;		// todo: sort this out for dent, etc.
	float morphTargetLevel = damageInfo.y;
#endif // #if !defined(PLATFORM_PS3)
#if defined(_USE_MORPHTARGETS) && defined(_ENABLE_MORPH_TARGETS)
	float3 posOffset		= input.vPositionOffset.xyz;
	localPos.xyz += posOffset.xyz * morphTargetLevel;

	float3 norOffset		= input.vNormalOffset.xyz;
	vNormal.xyz += norOffset.xyz * morphTargetLevel;
	vNormal = normalize(vNormal);
#endif

#if	!defined(_MATERIAL_NEON)
#if defined(_USE_DENT)
	#if defined(_MATERIAL_GLASS)
	output.vNormal.w = GLASS_DAMAGE_LEVEL //Dent level
	#else
	output.vNormal.w = damageInfo.y; //Dent level
	#endif
#endif
#endif

#if defined(_ENABLE_VERTEX_DEFORMATION)
#if !defined(PLATFORM_PS3)
	position				= EvaluateDamageOffset(localPos.xyz, damage.xyz);
#else
	position				= float4(localPos.xyz + damage.xyz, 1.0f);
#endif // #if !defined(PLATFORM_PS3)
#else
	position				= localPos;
#endif

	output.vWorldPos.w		= damage.w * SCRATCH_SCALE;
#else //LOD

	output.vWorldPos.w		= 0.0f;
#endif //LOD

#else
	output.vWorldPos.w		= 0.0f;
#endif

#else // SPU damage

	float4 position			= input.vSPUPos;
	float4 vNormal			= input.vSPUNormal;

	output.vWorldPos.w		= position.w;
	output.vNormal.w		= frac(vNormal.w);
	position.w				= 1;

#endif

	float4 localPosition	= position;

	float4 worldPos			= mul(position, g_mWorld);
	position				= mul(worldPos, g_mViewProj);
	output.vPosition		= position;
	output.vWorldPos.xyz	= worldPos.xyz;
	output.vNormal.xyz		= mul(vNormal.xyz, (float3x3)g_mWorld);
#if defined(_MATERIAL_ALLOY)
	output.vNormal.xyz		= normalize(output.vNormal.xyz);
#endif

#if defined(_MATERIAL_LIGHT) 
	output.vTextureUV0		= input.vTexCoord0;
	output.vTextureUV1		= input.vTexCoord1;
#else
	#if defined(_MATERIAL_NEON) || defined(_USE_ALBEDO) || defined(_USE_DENT) || defined(_USE_SCRATCHDAMAGE) || defined(_USE_SWAPDAMAGE)
	output.vTextureUV0		= input.vTexCoord0; 
	#endif
	#if defined(_USE_BUMP)
	output.vTextureUV1.xy	= input.vTexCoord1;
	#endif
#endif

	//Rotate AmbientOccusion
	float4 ambientOccusionSH = input.m_vAmbientOcclusionSH;
	float4 doubleAmbientOcclusionSH = ambientOccusionSH * 2.f;
	ambientOccusionSH.yzw = doubleAmbientOcclusionSH.yzw - 1.f;
	
	//dot ambientOccusion by SHNode
	output.vLighting.r = dot(g_LocalSpaceSHNode.m_vRedCoeffs, ambientOccusionSH);
	output.vLighting.g = dot(g_LocalSpaceSHNode.m_vGreenCoeffs, ambientOccusionSH);
	output.vLighting.b = dot(g_LocalSpaceSHNode.m_vBlueCoeffs, ambientOccusionSH);
	output.vLighting = BlurHDRLighting(output.vLighting); // [Blur2PC][HDR]

	output.vLighting.w = doubleAmbientOcclusionSH.x;

#if !defined(_MATERIAL_ALLOY)
#if !PLATFORM_PS3
	float rearLightFactor = saturate(saturate(-vNormal.z) - g_DebugRearLight) / (1.f - g_DebugRearLight);
	output.vLighting.rgb +=  rearLightFactor * rearLightFactor * saturate(-(localPosition.z * 0.5f)) * output.vLighting.w * g_VehicleVtxParams.RearLightColour;
#else
	output.vLighting.rgb += vNormal.w * g_VehicleVtxParams.RearLightColour;
#endif
#endif

	return output;
}

/////////////////////////
// Pixel Shader
/////////////////////////
//[removeUnusedInputs]
PSOutput MainPS( VSOutput input ) 
{ 
	PSOutput output				= (PSOutput)0;

	SurfaceProperties surface	= (SurfaceProperties)0;

	surface.m_vVertexLighting = input.vLighting;


#if defined(_USE_ALBEDO)
	surface.m_vAlbedoTex		= h4tex2D(MeshTextureSampler, input.vTextureUV0.xy);
#endif

#if defined(_MATERIAL_LIGHT)
//	float2 vLightTexCoord = lerp(input.vTextureUV0.xy, input.vTextureUV0.zw, g_vRenderColour.w);
//	surface.m_vAlbedoTex.rgb	= h4tex2D(OffTextureSampler, vLightTexCoord).rgb;
//	surface.m_vAlbedoTex.a		= g_vRenderColour.w;
	surface.m_vOffTexture		= h4tex2D(OffTextureSampler, input.vTextureUV0.xy);
	surface.m_vOnTexture		= h4tex2D(OnTextureSampler, input.vTextureUV0.zw);
	surface.m_vAlbedoTex		= lerp(real4(surface.m_vOffTexture.xyz, 0.0h), surface.m_vOnTexture, surface.m_vOnTexture.w * g_vRenderColour.w);
#endif

	surface.m_vWorldPos			= input.vWorldPos.xyz;
	surface.m_vView.xyz			= g_vViewPosition.xyz - input.vWorldPos.xyz;
	surface.m_fDistance			= length(surface.m_vView.xyz);
	surface.m_vView.xyz			= normalize(surface.m_vView.xyz);

#if defined(_USE_SCRATCHDAMAGE)

	#if defined(_MATERIAL_GLASS) || defined(_MATERIAL_WININT)
	surface.m_vDamageTex			= tex2Dbias(ScratchTextureSampler, float4(input.vTextureUV0.zw, -1.0f, -1.0f)) * g_vDamageSettings.x;
	#else
		#if defined(_MATERIAL_PAINT)
		surface.m_vDamageTex		= h4tex2D(ScratchTextureSampler, input.vTextureUV0.zw);

		surface.m_fDamageLevel		= input.vWorldPos.w;

		real2 vScuffUndercoatLevels	= surface.m_vDamageTex * saturate(2.0h * surface.m_fDamageLevel - real2(0.0h, 1.0h));
		surface.m_vDamage.xyz		= lerp(g_vScuffColour.xyz, g_vUndercoatColour.xyz, vScuffUndercoatLevels.y);
		surface.m_vDamage.w			= max(vScuffUndercoatLevels.x, vScuffUndercoatLevels.y);

		#else
		surface.m_vDamageTex		= h4tex2D(ScratchTextureSampler, input.vTextureUV0.zw) * input.vWorldPos.w;
		#endif
	#endif
#elif defined(_USE_SWAPDAMAGE)
	surface.m_vDamageTex		= h4tex2D(DamageTextureSampler, input.vTextureUV0.zw);
#endif

	surface.m_vNormal.xyz		= EvaluateNormal(input, surface.m_vDamage.w);
	surface.m_fAmbientOcclusion = EvaluateAmbientOcclusion(input, surface);

	surface.m_fNdotV			= saturate(dot(surface.m_vNormal, surface.m_vView));

#if defined(_MATERIAL_PAINT)
	surface.m_vPaintTex = tex2Dlod(PaintTextureSampler, real4(surface.m_fNdotV, 0.0h, 0.0h, 0.0h));
#endif

#if defined(_MATERIAL_LIGHT)
//	float2 vLightDamageTexCoord = lerp(input.vTextureUV1.xy, input.vTextureUV1.zw, g_vRenderColour.w);
//	surface.m_vDamageTex.rgb	= h4tex2D(OffTextureSampler, vLightDamageTexCoord).rgb;
//	surface.m_vDamageTex.a		= g_vRenderColour.w;
	surface.m_vOffTextureDamage	= h4tex2D(OffDamageTextureSampler, input.vTextureUV1.xy);
	surface.m_vOnTextureDamage	= h4tex2D(OnDamageTextureSampler, input.vTextureUV1.zw);
	surface.m_vDamageTex		= lerp(real4(surface.m_vOffTextureDamage.xyz, 0.0h), surface.m_vOnTextureDamage, surface.m_vOnTextureDamage.w * g_vRenderColour.w);
#endif

#if defined(_MATERIAL_WININT)
	real4 bloomTexture			= h4tex2D(BloomTextureSampler, input.vScreenUV);

	real4 materialColour		= 0.0h;
#if defined(_USE_ALBEDO)
	real4 surfaceColour			= real4(lerp(materialColour.xyz, surface.m_vAlbedoTex.xyz, surface.m_vAlbedoTex.w), 0.0h);
#else
	real4 surfaceColour			= real4(materialColour.xyz, 0.0h);
#endif

	surface.m_vSurfaceColour = surfaceColour;

	// [Blur2PC][HDR] Retain the authored reflection curve without its SDR ceiling.
	real3 reflection = BlurHDRReflection(pow(bloomTexture.xyz, g_vScreenSettingsPS.x) * g_vScreenSettingsPS.y, g_vScreenSettingsPS.z);

	#if defined(_USE_ALBEDO)
	output.colour.xyz			= lerp(surface.m_vSurfaceColour.xyz, reflection.xyz, 1.0h - surface.m_vAlbedoTex.w);
	#else
	output.colour.xyz			= reflection.xyz;
	#endif

	output.colour.w				= g_vScreenSettingsPS.w;
#if defined(_USE_ALBEDO)
	output.colour.w				= max(output.colour.w, surface.m_vAlbedoTex.w);
#endif

#else

	surface.m_vReflect.xyz		= normalize(-reflect(surface.m_vView.xyz, surface.m_vNormal.xyz));

#if !defined(_MATERIAL_MATT)
	surface.m_vReflectionTex	= h4texCUBE(EnvironmentTextureSampler, surface.m_vReflect.xyz);
#endif

	surface.m_vSurfaceColour	= EvaluateSurfaceColour(surface);

	surface.m_fReflectivity		= surface.m_vSurfaceColour.w;
#if !defined(_MATERIAL_MATT)
	surface.m_fReflectivity		= EvaluateReflectivity(surface);
#endif
#if defined(_MATERIAL_LEGEND)
	surface.m_fReflectivity		*= (1.f - surface.m_vAlbedoTex.a);
#endif

	surface.m_vLighting.xyz		= EvaluateLighting(surface);

	output.colour.w				= EvaluateAlpha(surface);
	output.colour.xyz			= EvaluateFinalColour(surface, output.colour.w);

	output.colour				= GetExposure(output.colour);
	
#if !defined(_MATERIAL_GLASS) && !defined(_MATERIAL_WININT) && !defined(ALPHA_LUMINANCE)
	output.colour.xyz			*= output.colour.w;
#else
	output.colour.xyz			*= g_VehiclePixParams.vAlphaScale.x;
#endif
#endif

	return output;
}


// technique


#if defined(PLATFORM_PC) 
#if defined(_MATERIAL_GLASS) || defined(_MATERIAL_ALPHA)
technique aHACK < string Define = "_EXPORT_DAMAGE_HACK";  string SortType = "backfront"; >
#elif defined(_MATEIAL_PAINT)
technique aHACK < string Define = "_EXPORT_DAMAGE_HACK"; int SortOrder = 1; >
#else
technique aHACK < string Define = "_EXPORT_DAMAGE_HACK";  >
#endif
{
	pass p0
	{
		VertexShader = compile vs_3_0 MainVS();
		PixelShader  = compile ps_3_0 MainPS();
		
#if defined(_MATERIAL_ALPHA) || defined(_MATERIAL_GLASS)
		AlphaBlendEnable	= true;
		AlphaTestEnable		= true;
		SrcBlend			= One;
		DestBlend			= InvSrcAlpha;
		BlendOp				= Add;
#if defined(_MATERIAL_GLASS)
		AlphaRef			= 0.0;
#endif

#if defined(_MATERIAL_ALPHA)
		AlphaRef			= 0.0;
#endif

#ifdef FXFILE_PARSER
		AlphaToMaskEnable	= false;
#endif

#endif
	}
}
#endif // #if defined(PLATFORM_PC)

//FULL
#if defined(_MATERIAL_GLASS) || defined(_MATERIAL_ALPHA) || defined(_MATERIAL_WININT)
technique Material < string SortType = "backfront"; >
#elif defined(_MATERIAL_PAINT)
technique Material < int SortOrder = 1; >
#else
technique Material
#endif
{
	pass p0
	{
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();

#if defined(_MATERIAL_ALPHA) || defined(_MATERIAL_GLASS) || defined(_MATERIAL_WININT)
		AlphaBlendEnable	= true;
		AlphaTestEnable		= true;
		SrcBlend			= One;
		DestBlend			= InvSrcAlpha;
		BlendOp				= Add;
#if defined(_MATERIAL_GLASS) || defined(_MATERIAL_WININT)
		AlphaRef			= 0.0;
#endif

#if defined(_MATERIAL_ALPHA)
		AlphaRef			= 0.00;
#endif

#ifdef FXFILE_PARSER
		AlphaToMaskEnable	= false;
#endif

#endif
	}
}

//FULL ALPHA LUMINANCE
#if defined(_MATERIAL_GLASS) || defined(_MATERIAL_ALPHA) || defined(_MATERIAL_WININT)
technique MaterialAlphaLuminance < string Define = "ALPHA_LUMINANCE"; string SortType = "backfront"; >
#elif defined(_MATERIAL_PAINT)
technique MaterialAlphaLuminance < string Define = "ALPHA_LUMINANCE"; int SortOrder = 1; >
#else
technique MaterialAlphaLuminance < string Define = "ALPHA_LUMINANCE"; >
#endif
{
	pass p0
	{
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();

#if defined(_MATERIAL_ALPHA) || defined(_MATERIAL_GLASS) || defined(_MATERIAL_WININT)
		AlphaBlendEnable	= true;
		AlphaTestEnable		= true;
		SrcBlend			= One;
		DestBlend			= InvSrcAlpha;
		BlendOp				= Add;
#if defined(_MATERIAL_GLASS) || defined(_MATERIAL_WININT)
		AlphaRef			= 0.0;
#endif

#if defined(_MATERIAL_ALPHA)
		AlphaRef			= 0.00;
#endif

#ifdef FXFILE_PARSER
		AlphaToMaskEnable	= false;
#endif

#endif
	}
}

//LOD
#if defined(_MATERIAL_GLASS) || defined(_MATERIAL_ALPHA)
technique MaterialLOD < string Define = "LOD"; string SortType = "backfront"; >
#elif defined(_MATERIAL_PAINT)
technique MaterialLOD < string Define = "LOD"; int SortOrder = 1; >
#else
technique MaterialLOD < string Define = "LOD"; >
#endif
{
	pass p0
	{
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();

#if defined(_MATERIAL_ALPHA) || defined(_MATERIAL_GLASS)
		AlphaBlendEnable	= true;
		AlphaTestEnable		= true;
		SrcBlend			= One;
		DestBlend			= InvSrcAlpha;
		BlendOp				= Add;
#if defined(_MATERIAL_GLASS)
		AlphaRef			= 0.0;
#endif

#if defined(_MATERIAL_ALPHA)
		AlphaRef			= 0.0;
#endif

#ifdef FXFILE_PARSER
		AlphaToMaskEnable	= false;
#endif

#endif
	}
}

//LOD ALPHA LUMINANCE
#if defined(_MATERIAL_GLASS) || defined(_MATERIAL_ALPHA)
technique MaterialAlphaLuminanceLOD < string Define = "LOD; ALPHA_LUMINANCE"; string SortType = "backfront"; >
#elif defined(_MATERIAL_PAINT)
technique MaterialAlphaLuminanceLOD < string Define = "LOD; ALPHA_LUMINANCE"; int SortOrder = 1; >
#else
technique MaterialAlphaLuminanceLOD < string Define = "LOD; ALPHA_LUMINANCE"; >
#endif
{
	pass p0
	{
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();

#if defined(_MATERIAL_ALPHA) || defined(_MATERIAL_GLASS)
		AlphaBlendEnable	= true;
		AlphaTestEnable		= true;
		SrcBlend			= One;
		DestBlend			= InvSrcAlpha;
		BlendOp				= Add;
#if defined(_MATERIAL_GLASS)
		AlphaRef			= 0.0;
#endif

#if defined(_MATERIAL_ALPHA)
		AlphaRef			= 0.0;
#endif

#ifdef FXFILE_PARSER
		AlphaToMaskEnable	= false;
#endif

#endif
	}
}



#endif

