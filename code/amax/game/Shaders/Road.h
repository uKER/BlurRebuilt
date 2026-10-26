#define REPLACE_LIGHTING_HIGHLIGHT
#define REPLACE_LIGHTING_REFLECTION
#define REPLACE_MATERIAL
#define REPLACE_SURFACE
#define REPLACE_MATERIAL_PREPASS
#define REPLACE_SURFACE_PREPASS

//#define ENABLE_PREPASS_VIEWDEPTH
#ifdef PLATFORM_PC
  #define ENABLE_PREPASS_DEPTHTEXTURE
#endif
#define ENABLE_ALBEDOMAP
#define ENABLE_FOG
#define ENABLE_NORMAL
#if !defined(NO_SPECULAR)
#define ENABLE_SPECULAR
#endif
#define ENABLE_WORLD

#define ENABLE_LIGHTMAP_OPTIMISATION
#define ENABLE_VERTEXCOLOURS_OPTIMISATION

#define MIPMAP_ALPHA_ENHANCE

#define DISABLE_LODFADE

#define OPAQUE_PASS

#define ALPHA_LUMINANCE

#if !defined(NO_LIGHTMAP)
#	define ENABLE_LIGHTMAP
#endif

#if !defined(NO_VTXCOL)
#	if !defined(ENABLE_VERTEXCOLOURS)
#		define ENABLE_VERTEXCOLOURS
#	endif
#endif

#if !defined(ENABLE_NORMAL)
#	if defined(DAY_LIGHTING) || defined(ENABLE_SPECULAR)
#		define ENABLE_NORMAL
#	endif
#endif
#if !defined(ENABLE_VIEW)
#	if defined(ENABLE_SPECULAR)
#		define ENABLE_VIEW
#	endif
#endif

#define ALBEDO_NAME_STRING "Colour"
#define MATERIAL_NAME_STRING "Material"
#define BUMP_NAME_STRING "Normal"

#if defined(ENABLE_BUMPMAP_OPTIMISATION)
#	define BUMP_DEFINE_STRING string Define = "NB";
#else
#	define BUMP_DEFINE_STRING
#endif

#if defined(ENABLE_LIGHTMAP_OPTIMISATION)
#	define LIGHTMAP_DEFINE_STRING string Define = "NL";
#else
#	define LIGHTMAP_DEFINE_STRING
#endif

#ifdef NIGHT_LIGHTING
#	define SH_NO_CHANNEL_COEFFS
#endif

#if defined(ENABLE_PREPASS)
#	if !defined(ENABLE_NORMAL)
#		define ENABLE_NORMAL
#	endif
#endif

#if defined(ENABLE_SHNODE_IN_PIXEL_SHADER)
#	if !defined(ENABLE_NORMAL)
#		define ENABLE_NORMAL
#	endif
#endif

#if defined(ENABLE_VERTEXCOLOURS) && defined(NO_DETAIL) && defined(NO_DIRT) && defined(NO_WORLDNORMAL)
#	undef ENABLE_VERTEXCOLOURS
#endif

#if defined(LOD)
#	if defined(ENABLE_NORMAL) && defined(ENABLE_LIGHTMAP) && !defined(DAY_LIGHTING)
#		undef ENABLE_NORMAL
#	endif
#	if defined(ENABLE_WORLD)
#		undef ENABLE_WORLD
#	endif
#	if defined(ENABLE_VIEW)
#		undef ENABLE_VIEW
#	endif
#	if defined(ENABLE_SPECULAR)
#		undef ENABLE_SPECULAR
#	endif
#endif

#if defined(CFG_CONVERTER)
#	if !defined(ENABLE_SHNODE_IN_PIXEL_SHADER)
#		if defined(ENABLE_SPECULAR)
#			undef ENABLE_SPECULAR
#		endif
#	endif
#	if defined(ENABLE_LIGHTMAP)
#		undef ENABLE_LIGHTMAP
#	endif
#	if defined(ENABLE_FOG)
#		undef ENABLE_FOG
#	endif
#endif

#ifndef CFG_CONVERTER
#if !defined(NO_WORLDNORMAL)
#define ENABLE_WORLDNORMAL
#define ENABLE_TANGENTNORMAL2D
#endif
#endif

struct C_VertexInput
{
	float3	m_vPosition		: POSITION;
	float3	m_vNormal		: NORMAL;
#if defined(ENABLE_LIGHTMAP)
	float4	m_vTexCoord		: TEXCOORD0 annotation(xy(uv:0) | zw(uv:luxlightmap) | format(F16_4));
#else
	float2	m_vTexCoord		: TEXCOORD0 annotation(xy(uv:0) | format(F16_2));
#endif
#if defined(ENABLE_TANGENTNORMAL2D)
#if !defined(ROAD_N)
	float4	m_vTangentBinormal2d : TEXCOORD1 annotation(xy(tangent:0.xz) | zw(binormal:0.xz) | format(F16_4));
#else // ROAD_N
	float4	m_vTangentBinormal2d : TEXCOORD1 annotation(xy(tangent:1.xz) | zw(binormal:1.xz) | format(F16_4));
#endif // ROAD_N
#endif // ENABLE_TANGENTNORMAL2D
#if !defined(NO_DETAIL) || !defined(NO_DIRT)
#if !defined(CFG_CONVERTER)
	float4	m_vDetailCoord	: TEXCOORD3 annotation(xy(uv:1) | zw(uv:2) | format(F16_4));
#else
	float2	m_vDetailCoord0	: TEXCOORD3;
	float2	m_vDetailCoord1	: TEXCOORD4;
#endif
#endif
#if defined(ENABLE_VERTEXCOLOURS)
	float4	m_vColour		: COLOR0 annotation(xyzw(colour:vertexColour) | instancemode(instance) | define(NV));
#endif
};

struct C_VertexOutput
{
	float4	m_vPosition		: POSITION;
	float4	m_vTexCoord		: TEXCOORD0;
#if defined(ENABLE_NORMAL)
	real4	m_vNormal		: TEXCOORD1;
#endif
#if defined(ENABLE_VIEW)
	real3	m_vView			: TEXCOORD2;
#endif
#if defined(ENABLE_WORLD)
	float4	m_vWorld		: TEXCOORD3;
#endif
#if !defined(LOD)
	float4	m_vScreenPos	: TEXCOORD4;
#endif
#if !defined(NO_DETAIL) || !defined(NO_DIRT)
	float4	m_vExtraCoord	: TEXCOORD5;
#endif
#if defined(ENABLE_VERTEXCOLOURS)
	real4	m_vColour		: COLOR0;
#endif
};

struct C_VertexPrepassOutput
{
	float4	m_vPosition		: POSITION;
#if defined(BLUR_DX11) && defined(NO_WORLDNORMAL) && defined(ENABLE_PREPASS_DEPTHTEXTURE)
	// [Blur2PC][Blur1PCDX11][retail-recovered] The PC road prepass carries
	// clip z/w separately from the normal (D3D9 race trace draw 7047254).
	float4 m_vDepth : TEXCOORD3;
#endif
#if !defined(NO_WORLDNORMAL)
	float4	m_vTexCoord		: TEXCOORD0;
#else
	real4	m_vNormal		: TEXCOORD1;
#endif
#if defined(ENABLE_TANGENTNORMAL2D)
	real4	m_vTangentBinormal2d	: TEXCOORD2;
#endif
};

struct C_PixelOutput
{
    real4	m_vColour	: COLOR0;
};

#if defined(ENABLE_PREPASS_DEPTHTEXTURE)
struct C_PixelOutputD
{
    real4	 m_vColour	: COLOR0;
    float4   m_fDepth   : COLOR1;
};
#endif

#include <Common/Transform.h>
#include <Common/Material.h>
#include <Common/Surface.h>
#include <Common/Lighting.h>
#include <Shader/Common/Debug.h>
#include <Special/RoadSurface.h>

// vertex shader

C_VertexOutput RoadSurfaceVS(in C_VertexInput input)
{
   	C_VertexData vtxData = (C_VertexData) 0;
	vtxData	= EvaluateBindPosition(vtxData, input);
	vtxData	= EvaluateObjectPosition(vtxData, input);
	vtxData	= EvaluateWorldPosition(vtxData, input);

	C_VertexOutput output = (C_VertexOutput) 0;
	output.m_vPosition		= mul(vtxData.m_vPosition, g_mViewProj);
#if defined(ENABLE_NORMAL)
	output.m_vNormal.xyz	= vtxData.m_vNormal.xyz;
#endif
#if defined(ENABLE_VIEW)
	output.m_vView.xyz		= vtxData.m_vPosition.xyz - g_vViewPosition.xyz;
	//output.m_vView.w		= output.m_vPosition.w;
#endif
#if defined(ENABLE_WORLD)
	float3	vWorldPos		= vtxData.m_vPosition.xyz + float3(vtxData.m_vNormal.x, 0.0f, vtxData.m_vNormal.z);
	output.m_vWorld.x		= dot(float4(vWorldPos, 1.0f), g_ReflectionViewMatrix0);
	output.m_vWorld.y		= dot(float4(vWorldPos, 1.0f), g_ReflectionViewMatrix1);
	output.m_vWorld.z		= dot(float4(vWorldPos, 1.0f), g_ReflectionViewMatrix3);
	output.m_vWorld.xy		= output.m_vWorld.xy * float2(0.5f, -0.5f) + output.m_vWorld.z * float2(0.5f, 0.5f);
#endif
#if !defined(LOD)
	output.m_vScreenPos	= float4(output.m_vPosition.xy * float2(0.5f, -0.5f) + 0.5f * output.m_vPosition.w, output.m_vPosition.w, vtxData.m_vPosition.y);
#endif
	output.m_vTexCoord.xy	= input.m_vTexCoord.xy;
#if defined(ENABLE_LIGHTMAP)
	output.m_vTexCoord.zw	= input.m_vTexCoord.zw + g_vUVOffset;
#endif
#if !defined(NO_DETAIL) || !defined(NO_DIRT)
#if !defined(CFG_CONVERTER)
	output.m_vExtraCoord	= input.m_vDetailCoord;
#else
	output.m_vExtraCoord	= float4(input.m_vDetailCoord0, input.m_vDetailCoord1);
#endif
#endif
#if defined(ENABLE_VERTEXCOLOURS) && (!defined(NO_DETAIL) || !defined(NO_DIRT) || defined(ENABLE_HACK))
	output.m_vColour		= input.m_vColour;
#endif

    return output;
}

C_VertexPrepassOutput RoadSurfacePrepassVS(in C_VertexInput input)
{
	C_VertexData vtxData = (C_VertexData) 0;
	vtxData	= EvaluateBindPosition(vtxData, input);
	vtxData	= EvaluateObjectPosition(vtxData, input);
	vtxData	= EvaluateWorldPosition(vtxData, input);

	C_VertexPrepassOutput output = (C_VertexPrepassOutput) 0;
	output.m_vPosition		= mul(vtxData.m_vPosition, g_mViewProj);
#if defined(BLUR_DX11) && defined(NO_WORLDNORMAL) && defined(ENABLE_PREPASS_DEPTHTEXTURE)
	output.m_vDepth = float4(0.0f, 0.0f, output.m_vPosition.zw);
#endif
#if defined(NO_WORLDNORMAL)
#if defined(BLUR_DX11)
	// [Blur2PC][Blur1PCDX11][retail-recovered] Every shipped PC _NW road prepass
	// VS writes a zero normal and reads no vertex normal.
	output.m_vNormal.xyz	= 0.0f;
#else
	output.m_vNormal.xyz	= vtxData.m_vNormal.xyz;
#endif
#if defined(ENABLE_PREPASS_VIEWDEPTH)
	output.m_vNormal.w		= output.m_vPosition.w / g_vNearFar.w;
#endif
#else
#if defined(ROAD_N) && !defined(NO_DETAIL)
	output.m_vTexCoord.xy	= input.m_vDetailCoord.xy;
#else
	output.m_vTexCoord.xy	= input.m_vTexCoord.xy;
#endif
#if defined(ENABLE_PREPASS_VIEWDEPTH)
	output.m_vTexCoord.z	= output.m_vPosition.w / g_vNearFar.w;
#endif
#if defined(ENABLE_VERTEXCOLOURS)
	output.m_vTexCoord.w	= input.m_vColour.g;
#else
	output.m_vTexCoord.w	= 1.0f;
#endif
	// This is for PC, but looking at it, will clearly not work for road surface without a normal map
	// and especially not now I'm using the texture coordinate w channel to output a vertex colour
#if defined(ENABLE_PREPASS_DEPTHTEXTURE)
	output.m_vTexCoord.zw	= output.m_vPosition.zw;  
#endif
#endif

#if defined(ENABLE_TANGENTNORMAL2D)
	output.m_vTangentBinormal2d = input.m_vTangentBinormal2d;
#endif

    return output;
}

#ifdef CFG_DEBUG
	float g_fNormalFactorRange < string DebugVariable = "NormalFactorRange"; string Default = "0.15"; string Type = "Float"; > = 0.15f;
#else
	const real g_fNormalFactorRange = 0.15h;
#endif

real CalculateFresnelFactor(in real3 normal)
{
	float factor = saturate((normal.y - (1.0h - g_fNormalFactorRange)) / g_fNormalFactorRange);
	return factor * factor;
}
