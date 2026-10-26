#ifndef AMAX_SHADERS_COMMON_PARAMETERS_H
#define AMAX_SHADERS_COMMON_PARAMETERS_H

#ifndef VS_POSITION_ANNOTATION
#define VS_POSITION_ANNOTATION
#endif

#define VS_INPUT_POSITION			float3	m_vPosition		: POSITION VS_POSITION_ANNOTATION ;
#define VS_INPUT_NORMAL				float3	m_vNormal		: NORMAL;
#if defined(ENABLE_LIGHTMAP)
#	define VS_INPUT_TEXCOORD		float4	m_vTexCoord		: TEXCOORD0 annotation(xy(uv:0) | zw(uv:luxlightmap) | format(F16_4));
#elif defined(ENABLE_WIND)
#	define VS_INPUT_TEXCOORD		float4	m_vTexCoord		: TEXCOORD0 annotation(xy(uv:0) | z(windindex) | w(windweight) | format(F16_4));
#else
#	define VS_INPUT_TEXCOORD		float2	m_vTexCoord		: TEXCOORD0 annotation(xy(uv:0) | format(F16_2));
#endif
#if defined(ENABLE_TANGENTNORMAL)
#ifndef CFG_CONVERTER
#	define VS_INPUT_TANGENT			float4	m_vTangent		: TEXCOORD1 annotation(xyz(tangent) | w(sense) | format(U8_4N.bias));
#else
#	define VS_INPUT_TANGENT			float4	m_vTangent		: TANGENT0;
#endif
#else
#	define VS_INPUT_TANGENT
#endif
#if defined(ENABLE_SKINNING)
#if defined(BLUR_DX11)
// [Blur2PC] Match the native U8_4 integer input; FX9 converted it to float.
#	define VS_INPUT_BLENDINDICES	uint4	m_vBlendIndices	: TEXCOORD3 annotation(xyzw(blendindices) | format(U8_4));
#else
#	define VS_INPUT_BLENDINDICES	float4	m_vBlendIndices	: TEXCOORD3 annotation(xyzw(blendindices) | format(U8_4));
#endif
#	define VS_INPUT_BLENDWEIGHTS	float4	m_vBlendWeights	: TEXCOORD4 annotation(xyzw(blendweights) | format(U8_4N));
#else
#	define VS_INPUT_BLENDINDICES
#	define VS_INPUT_BLENDWEIGHTS
#endif
#if defined(ENABLE_VERTEXCOLOURS)
#if defined(ENABLE_VERTEXCOLOURS_OPTIMISATION)
#   if defined(ENABLE_VERTEXLIGHTING)
#		define VS_INPUT_COLOUR			float4	m_vColour		: COLOR0 annotation(xyzw(colour:luxVertexColour) | instancemode(instance) | define(NV));
#	else
#		define VS_INPUT_COLOUR			float4	m_vColour		: COLOR0 annotation(xyzw(colour:vertexColour) | instancemode(instance) | define(NV));
#	endif
#else
#   if defined(ENABLE_VERTEXLIGHTING)
#		define VS_INPUT_COLOUR			float4	m_vColour		: COLOR0 annotation(xyzw(colour:luxVertexColour) | instancemode(instance) );
#	else
#		define VS_INPUT_COLOUR			float4	m_vColour		: COLOR0 annotation(xyzw(colour:vertexColour) | instancemode(instance) );
#	endif
#endif
#else
#	define VS_INPUT_COLOUR
#endif

#define VS_OUTPUT_POSITION			float4	m_vPosition		: POSITION;
#define	VS_OUTPUT_TEXCOORD			float4	m_vTexCoord		: TEXCOORD0;
#if defined(ENABLE_NORMAL)
#	define VS_OUTPUT_NORMAL			real4	m_vNormal		: TEXCOORD1;
#else
#	define VS_OUTPUT_NORMAL
#endif
#if defined(ENABLE_VIEW)
#	define VS_OUTPUT_VIEW			real3	m_vView			: TEXCOORD2;
#else
#	define VS_OUTPUT_VIEW
#endif
#if defined(ENABLE_WORLD)
#	define VS_OUTPUT_WORLD			float4	m_vWorld		: TEXCOORD3;
#else
#	define VS_OUTPUT_WORLD
#endif
#if defined(ENABLE_TANGENTNORMAL)
#	define VS_OUTPUT_TANGENT		real3	m_vTangent		: TEXCOORD4;
#	define VS_OUTPUT_BINORMAL		real3	m_vBinormal		: TEXCOORD5;
#else
#	define VS_OUTPUT_TANGENT
#	define VS_OUTPUT_BINORMAL
#endif
#if !defined(LOD)
#	define VS_OUTPUT_SCREENHEIGHT	float4	m_vScreenPos	: TEXCOORD6;
#else
#	define VS_OUTPUT_SCREENHEIGHT
#endif
#if defined(ENABLE_TANGENTVIEW)
#	define VS_OUTPUT_TANGENTVIEW	real4	m_vTangentView	: TEXCOORD7;
#else
#	define VS_OUTPUT_TANGENTVIEW
#endif
#if defined(ENABLE_VERTEXCOLOURS)
#	define VS_OUTPUT_COLOUR			real4  m_vColour		: COLOR0;
#elif (defined(SHNODE_IN_VERTEX_CONSTANT) && !defined(ENABLE_LIGHTMAP))
#	define VS_OUTPUT_COLOUR			real4  m_vColour		: COLOR0;	// this is to output the per-vertex calculated SH
#else
#	define VS_OUTPUT_COLOUR
#endif

#define VS_INPUT_BASE \
			VS_INPUT_POSITION \
			VS_INPUT_NORMAL \
			VS_INPUT_TEXCOORD \
			VS_INPUT_TANGENT \
			VS_INPUT_BLENDINDICES \
			VS_INPUT_BLENDWEIGHTS \
			VS_INPUT_COLOUR

#define VS_OUTPUT_BASE \
			VS_OUTPUT_POSITION \
			VS_OUTPUT_TEXCOORD \
			VS_OUTPUT_NORMAL \
			VS_OUTPUT_VIEW \
			VS_OUTPUT_WORLD \
			VS_OUTPUT_TANGENT \
			VS_OUTPUT_BINORMAL \
			VS_OUTPUT_SCREENHEIGHT \
			VS_OUTPUT_COLOUR \
			VS_OUTPUT_TANGENTVIEW

#if !defined(REPLACE_VERTEXSHADER_INPUT)
struct C_VertexInput
{
	VS_INPUT_BASE
};
#endif

#if !defined(REPLACE_VERTEXSHADER_OUTPUT)
struct C_VertexOutput
{
	VS_OUTPUT_BASE
};
#endif

struct C_VertexPrepassOutput
{
	VS_OUTPUT_POSITION
	VS_OUTPUT_TEXCOORD
	VS_OUTPUT_NORMAL
	VS_OUTPUT_TANGENT
	VS_OUTPUT_BINORMAL
};

#if !defined(REPLACE_PIXELSHADER_OUTPUT)
struct C_PixelOutput
{
    real4	m_vColour	: COLOR0;
};
#endif

#endif // AMAX_SHADERS_COMMON_PARAMETERS_H
