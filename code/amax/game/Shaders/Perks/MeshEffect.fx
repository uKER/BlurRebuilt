#include <Shader/Core.h>
#include <Shader/Globals.h>
#include <Shader/Common/HDR.h>
#include <Shader/Misc/Resolve.h>

struct C_VertexInput
{
	float3	m_vPosition	: POSITION;
	float4	m_vColour	: COLOR0 annotation(xyzw(colour:vertexColour));
	float3	m_vNormal	: NORMAL;
#ifdef CFG_CONVERTER
	float2	m_vTexCoord		: TEXCOORD0;
	#ifdef DOUBLELAYER
		float2	m_vTexCoord2	: TEXCOORD1;
	#endif
#else
	#ifdef DOUBLELAYER
		float4	m_vTexCoord	: TEXCOORD0 annotation(xy(uv:0) | zw(uv:1) | format(F16_4));
	#else
		float2	m_vTexCoord	: TEXCOORD0 annotation(xy(uv:0) | format(F16_2));
	#endif
#endif
	#if defined(REFRACT)
		float3	m_vTangent		: TEXCOORD2 annotation(xyz(tangent) | format(SHHD_3N));
		float3	m_vBinormal		: TEXCOORD3 annotation(xyz(binormal) | format(SHHD_3N));
	#endif

#if defined(ENABLE_SKINNING)
#if defined(BLUR_DX11)
	// [Blur2PC] Preserve numeric U8_4 bone indices with the native integer input.
	uint4	m_vBlendIndices	: TEXCOORD4 annotation(xyzw(blendindices) | format(U8_4));
#else
	float4	m_vBlendIndices	: TEXCOORD4 annotation(xyzw(blendindices) | format(U8_4));
#endif
	float4	m_vBlendWeights	: TEXCOORD5 annotation(xyzw(blendweights) | format(U8_4N));
#endif
};

struct C_VertexOutput
{
	float4	m_vPosition	: POSITION;
#ifndef LOD
	real4	m_vColour	: TEXCOORD1;
	#ifdef DOUBLELAYER
	real4	m_vColour2L	: TEXCOORD2;
	#endif
#endif
#ifdef DOUBLELAYER
	float4	m_vTexCoord	: TEXCOORD0;
#else
	#ifndef LOD
		float2	m_vTexCoord	: TEXCOORD0;
	#else
		float4	m_vTexCoord	: TEXCOORD0; // z-> Fresnel, w->Vertex Alpha
	#endif
#endif
	#if !defined(MTOOL) && !defined (LOD)
		#if defined(REFRACT)
			float4  m_TangentDir: TEXCOORD3; //x,y -> tangent in screenspace, z,w -> binormal in screenspace
			real4	m_vRefractColour : TEXCOORD6;
		#endif
	float4	m_ScreenPosition	: TEXCOORD4;
	#endif
	#ifdef SCROLLDISTORT
		#ifdef SCROLLDISTORT_DOUBLELAYER
			float4	m_UVDistort		: TEXCOORD5;
		#else
			float2	m_UVDistort		: TEXCOORD5;
		#endif
	#endif
	
};

struct C_PixelOutput
{
	real4	m_vColour	: COLOR0;
};

#define REPLACE_TRANSFORM_POSITION
#define REPLACE_VERTEXSHADER_INPUT
#define REPLACE_VERTEXSHADER_OUTPUT
#define REPLACE_PIXELSHADER_OUTPUT
#include "Common/Parameters.h"
#include "Common/Transform.h"

#ifdef LOD
	#ifdef SCROLLDISTORT
		#undef SCROLLDISTORT
	#endif
	#ifdef DOUBLELAYER
		#undef DOUBLELAYER
	#endif
	#ifdef SCROLLDISTORT_DOUBLELAYER
		#undef SCROLLDISTORT_DOUBLELAYER
	#endif
#endif

#ifdef SCROLLDISTORT_DOUBLELAYER
#define DOUBLELAYER
#endif

texture2D	g_tTexture	< string Source = "Texture"; string Default = "Black"; bool AlphaDetect = true; >;

sampler TextureSampler = 
sampler_state
{
    Texture = <g_tTexture>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	//SRGB_TEXTURE
};

shared real4	g_GlobalColour:  register(CCV_SHADER_USER_1) = float4(1.f, 1.f, 1.f, 1.f);

#ifdef CFG_DEBUG
	float	g_fNegativeFactor   < string DebugVariable = "NegativeFactor"; string Range = "(0,4)"; string Type = "Float"; > = 2.0f;
#else
	const float	g_fNegativeFactor = 2.0f;
#endif

real4	g_vColour:  register(CCV_SHADER_USER_2)			< string Source = "Colour"; string Default = "1.0, 1.0, 1.0, 1.0"; >;
real4	g_vColourFresnel:  register(CCV_SHADER_USER_3)	< string Source = "ColourFresnel"; string Default = "1.0, 1.0, 1.0, 1.0"; >;
real	g_vFresnelPower:  register(CCV_SHADER_USER_4)		< string Source = "FresnelPower"; string Default = "1.0"; >;
float	g_vScrollTextureX:  register(CCV_SHADER_USER_5)	< string Source = "ScrollTextureX"; string Default = "0.0"; >;
float	g_vScrollTextureY:  register(CCV_SHADER_USER_6)	< string Source = "ScrollTextureY"; string Default = "0.0"; >;
float	g_vSpeedScrollTextureX:  register(CCV_SHADER_USER_7)	< string Source = "SpeedScrollTextureX"; string Default = "0.0"; >	= 0.0f;
float	g_vSpeedScrollTextureY:  register(CCV_SHADER_USER_8)	< string Source = "SpeedScrollTextureY"; string Default = "0.0"; >	= 0.0f;

#ifdef DOUBLELAYER
#ifdef CULLENABLE
#ifndef DEPTHFADEDISABLE
#ifndef SCROLLDISTORT_DOUBLELAYER
float2	g_DistanteFadeParams:  register(CCV_SHADER_USER_9)	< string Source = "DistanteFadeParams"; string Default = "100000.0, 10000.0"; string Type = "Float2"; >		= float2(10000.0f, 10000.f);
#endif
#endif
#endif
#endif

#if !defined(CFG_CONVERTER) && defined(REFRACT)
shared texture2D	g_tColourBuffer;
sampler ColourBufferSampler : register(SSP_SHADER_COLOUR_BUFFER) = 
sampler_state
{
    Texture = <g_tColourBuffer>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};
#endif

#if !defined(CFG_CONVERTER)

#if defined(CFG_CONVERTER)
float4			g_vDepthScale		= 0.0f;
#else
shared float4	g_vDepthScale		: register(CCP_SHADER_DEPTH_SCALE);
#endif

shared texture2D	g_tDepthBuffer;
sampler DepthBufferSampler : register(SSP_SHADER_DEPTH_BUFFER) = 
sampler_state
{
    Texture = <g_tDepthBuffer>;
    MipFilter = POINT;
	MinFilter = POINT;
	MagFilter = POINT;
};

#endif

#ifdef REFRACT
	float	g_pRefractionIntensity:  register(CCV_SHADER_USER_10)	< string Source = "RefractionIntensity"; string Default = "0.1"; >;

	real4	g_vRefractColour:  register(CCV_SHADER_USER_11)			< string Source = "RefractColour"; string Default = "1.0, 1.0, 1.0, 1.0"; >;
	real4	g_vRefractColourFresnel:  register(CCV_SHADER_USER_12)		< string Source = "RefractColourFresnel"; string Default = "1.0, 1.0, 1.0, 1.0"; >;
#endif

#ifdef SCROLLDISTORT
	float	gUvScrollDistortScale:  register(CCV_SHADER_USER_13)	< string Source = "ScrollDistortScale"; string Default = "1.0"; >;
	float	gUvScrollDistortOffsetX	:  register(CCV_SHADER_USER_14)< string Source = "ScrollDistortOffsetX"; string Default = "0.0"; >;
	float	gUvScrollDistortOffsetY:  register(CCV_SHADER_USER_15)	< string Source = "ScrollDistortOffsetY"; string Default = "0.0"; >;
	float	gUvDistort:  register(CCV_SHADER_USER_16)				< string Source = "Distort"; string Default = "1.0"; >;
	
	texture2D	g_tUVDistortTexture	< string Source = "UVDistortTexture"; string Default = "Black"; string Format = "A8L8";>;

	sampler UVDistortTextureSampler = 
	sampler_state
	{
		Texture = <g_tUVDistortTexture>;
		MipFilter = LINEAR;
		MinFilter = LINEAR;
		MagFilter = LINEAR;
	};
#endif

#ifdef DOUBLELAYER
texture2D	g_2L_tTexture	< string Source = "TLTexture"; string Default = "Black"; bool AlphaDetect = true; >;

sampler TextureSampler2L = 
sampler_state
{
    Texture = <g_2L_tTexture>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	//SRGB_TEXTURE
};
#endif

#ifdef DOUBLELAYER
	real4	g_2L_vColour			:  register(CCV_SHADER_USER_17)< string Source = "TL_Colour"; string Default = "1.0, 1.0, 1.0, 1.0"; >;
	real4	g_2L_vColourFresnel		:  register(CCV_SHADER_USER_18)< string Source = "TL_ColourFresnel"; string Default = "1.0, 1.0, 1.0, 1.0"; >;
	real	g_2L_vFresnelPower		:  register(CCV_SHADER_USER_19)< string Source = "TL_FresnelPower"; string Default = "1.0"; >;
	float	g_2L_vScrollTextureX	:  register(CCV_SHADER_USER_20)< string Source = "TL_ScrollTextureX"; string Default = "0.0"; >;
	float	g_2L_vScrollTextureY	:  register(CCV_SHADER_USER_21)< string Source = "TL_ScrollTextureY"; string Default = "0.0"; >;
	float	g_2L_vSpeedScrollTextureX :  register(CCV_SHADER_USER_22)	< string Source = "TL_SpeedScrollTextureX"; string Default = "0.0"; >	= 0.0f;
	float	g_2L_vSpeedScrollTextureY :  register(CCV_SHADER_USER_27)	< string Source = "TL_SpeedScrollTextureY"; string Default = "0.0"; >	= 0.0f;
#endif

#ifdef SCROLLDISTORT_DOUBLELAYER
	float	g_2L_UvScrollDistortScale:  register(CCV_SHADER_USER_23)	< string Source = "TL_ScrollDistortScale"; string Default = "1.0"; >;
	float	g_2L_UvScrollDistortOffsetX:  register(CCV_SHADER_USER_24)	< string Source = "TL_ScrollDistortOffsetX"; string Default = "0.0"; >;
	float	g_2L_UvScrollDistortOffsetY:  register(CCV_SHADER_USER_25)	< string Source = "TL_ScrollDistortOffsetY"; string Default = "0.0"; >;
	float	g_2L_UvDistort:  register(CCV_SHADER_USER_26)				< string Source = "TL_Distort"; string Default = "1.0"; >;
	
	texture2D	g_2L_tUVDistortTexture	< string Source = "TL_UVDistortTexture"; string Default = "Black"; string Format = "A8L8";>;

	sampler UVDistortTextureSampler2L = 
	sampler_state
	{
		Texture = <g_2L_tUVDistortTexture>;
		MipFilter = LINEAR;
		MinFilter = LINEAR;
		MagFilter = LINEAR;
	};
#endif

#if defined(PLATFORM_PS3) && defined(__PIXEL__)
#pragma texformat g_tTexture COMPRESSED_RGBA_S3TC_DXT1
#pragma texformat g_tUVDistortTexture LUMINANCE8_ALPHA8
#ifdef DOUBLELAYER
	#pragma texformat g_2L_tTexture COMPRESSED_RGBA_S3TC_DXT1
#endif
#ifdef SCROLLDISTORT_DOUBLELAYER
	#pragma texformat g_2L_tUVDistortTexture LUMINANCE8_ALPHA8
#endif
#pragma texformat g_tColourBuffer RGBA8
#pragma texformat g_tDepthBuffer RGBA8
#endif

C_VertexOutput MainVS(in C_VertexInput input)
{
	C_VertexOutput output = (C_VertexOutput) 0;
	
	C_VertexData vtxData = (C_VertexData) 0;
	vtxData	= EvaluateBindPosition(vtxData, input);
#if defined(BLUR_DX11) && defined(ENABLE_SKINNING)
	// [Blur2PC][Blur1PCDX11][retail-recovered] Shipped skinned mesh effects skin
	// only the position; the fresnel term uses the bind normal in world space.
	float3	vBindNormal	= vtxData.m_vNormal;
#endif
	vtxData	= EvaluateObjectPosition(vtxData, input);
	vtxData	= EvaluateWorldPosition(vtxData, input);
#if defined(BLUR_DX11) && defined(ENABLE_SKINNING)
	vtxData.m_vNormal	= mul(vBindNormal, (float3x3) g_mWorld);
#endif

	float4	vWorldPosition	= vtxData.m_vPosition;
	float4	vProjPosition	= mul(vWorldPosition, g_mViewProj);
	
	output.m_vPosition	= vProjPosition;

	// Hack for a specific shader to fade with distance, for Mike Bibby
	real fade = 1.0h;
#ifdef DOUBLELAYER
#ifdef CULLENABLE
#ifndef DEPTHFADEDISABLE
#ifndef SCROLLDISTORT_DOUBLELAYER
	fade = 1.0h - saturate((vProjPosition.w - g_DistanteFadeParams.x) / (g_DistanteFadeParams.y - g_DistanteFadeParams.x));
#endif
#endif
#endif
#endif

#ifdef CFG_CONVERTER
	//Scroll Texture
	#ifndef SCROLLDISTORT
	output.m_vTexCoord.xy	= input.m_vTexCoord.xy + float2(g_vScrollTextureX,g_vScrollTextureY) + float2(g_vSpeedScrollTextureX, g_vSpeedScrollTextureY) * g_fTime;
	#else
		output.m_vTexCoord.xy	= input.m_vTexCoord.xy;
	#endif
	
	#ifdef DOUBLELAYER
		#ifndef SCROLLDISTORT_DOUBLELAYER
			output.m_vTexCoord.zw	= input.m_vTexCoord2.xy + float2(g_2L_vScrollTextureX,g_2L_vScrollTextureY) + float2(g_2L_vSpeedScrollTextureX, g_2L_vSpeedScrollTextureY) * g_fTime;
		#else
			output.m_vTexCoord.zw	= input.m_vTexCoord2.xy;
		#endif
	#endif
#else
	//Scroll Texture
	#ifndef SCROLLDISTORT
		output.m_vTexCoord.xy	= input.m_vTexCoord.xy + float2(g_vScrollTextureX,g_vScrollTextureY) + float2(g_vSpeedScrollTextureX, g_vSpeedScrollTextureY) * g_fTime;
	#else
		output.m_vTexCoord.xy	= input.m_vTexCoord.xy;
	#endif
	
	#ifdef DOUBLELAYER
		#ifndef SCROLLDISTORT_DOUBLELAYER
			output.m_vTexCoord.zw	= input.m_vTexCoord.zw + float2(g_2L_vScrollTextureX,g_2L_vScrollTextureY) + float2(g_2L_vSpeedScrollTextureX, g_2L_vSpeedScrollTextureY) * g_fTime;
		#else
			output.m_vTexCoord.zw	= input.m_vTexCoord.zw;
		#endif
	#endif
#endif

real fresnel = saturate(1.h - abs(dot(normalize(vWorldPosition.xyz - g_vViewPosition.xyz), normalize( vtxData.m_vNormal))));
	fresnel = saturate(pow(fresnel + 0.00001f, g_vFresnelPower));
#ifndef LOD
	output.m_vColour = input.m_vColour * g_GlobalColour * lerp(g_vColour, g_vColourFresnel, fresnel) * fade;
	#ifdef DOUBLELAYER
	output.m_vColour2L = input.m_vColour * g_GlobalColour * lerp(g_2L_vColour, g_2L_vColourFresnel, fresnel) * fade;
	#endif
#else
	//Add to the colour the fresnel
	output.m_vTexCoord.z = fresnel;
	output.m_vTexCoord.w = input.m_vColour.a * g_GlobalColour.a * fade;
#endif
	
	#if !defined(CFG_CONVERTER) && !defined(LOD)
		#if defined(REFRACT)
			float4	vTangentDir;
			float4 viewTangent = mul(float4(input.m_vTangent.xyz,1), (float3x4)g_mViewProj);
			float4 viewBinormal = mul(float4(input.m_vBinormal.xyz,1), (float3x4)g_mViewProj);
			vTangentDir.xy = viewTangent.xy;
			vTangentDir.zw = viewBinormal.xy;
			
			//Tangent dir
			output.m_TangentDir = vTangentDir;

			output.m_vRefractColour = lerp(g_vRefractColour, g_vRefractColourFresnel, fresnel) * input.m_vColour * g_GlobalColour;
		#endif
		//Screen Pos
		output.m_ScreenPosition = vProjPosition * float4(0.5f, - 0.5, 1.f, 1.f) + float4(0.5f * vProjPosition.w, 0.5f * vProjPosition.w, 0.f, 0.f);
	#endif
	
	#ifdef SCROLLDISTORT
		output.m_UVDistort.xy = float2(output.m_vTexCoord.x * gUvScrollDistortScale + gUvScrollDistortOffsetX, output.m_vTexCoord.y * gUvScrollDistortScale + gUvScrollDistortOffsetY);
	#endif

	#ifdef SCROLLDISTORT_DOUBLELAYER
		output.m_UVDistort.zw = float2(output.m_vTexCoord.z * g_2L_UvScrollDistortScale + g_2L_UvScrollDistortOffsetX, output.m_vTexCoord.w * g_2L_UvScrollDistortScale + g_2L_UvScrollDistortOffsetY);
	#endif
	
	return output;
}

C_PixelOutput MainPS(in C_VertexOutput input)
{
	C_PixelOutput output = (C_PixelOutput) 0; 
	
	#ifdef SCROLLDISTORT
		float2 distort = ((tex2D(UVDistortTextureSampler, input.m_UVDistort.xy).ga-0.5f)*2.f);
		float2	main_uv	= input.m_vTexCoord.xy + (distort.xy + float2(g_vScrollTextureX,g_vScrollTextureY)) * gUvDistort ;
		real3 texColour = h4tex2D(TextureSampler, main_uv).rgb;
	#else
		real3 texColour = h4tex2D(TextureSampler, input.m_vTexCoord.xy).rgb;
	#endif

	#ifdef DOUBLELAYER
		#ifdef SCROLLDISTORT_DOUBLELAYER
			float2 distort2l = ((tex2D(UVDistortTextureSampler2L, input.m_UVDistort.zw).ga-0.5f)*2.f);
			float2	main_uv2l	= input.m_vTexCoord.zw + (distort2l.xy + float2(g_2L_vScrollTextureX,g_2L_vScrollTextureY)) * g_2L_UvDistort ;
			real3 texColour2L = h4tex2D(TextureSampler2L, main_uv2l).rgb;
		#else
			real3 texColour2L = h4tex2D(TextureSampler2L, input.m_vTexCoord.zw).rgb;
		#endif
	#endif

	#if !defined(CFG_CONVERTER) && !defined(LOD)
		float2 screenPos = input.m_ScreenPosition.xy / input.m_ScreenPosition.w;
	#endif

	#if !defined(CFG_CONVERTER) && defined(REFRACT) && !defined(LOD)
		real2 distorsion = texColour.rg * 2.h - 1.h;
		real2x2 distorsionMatrix = real2x2((input.m_TangentDir.xy), (input.m_TangentDir.zw));
		
		real4 refractColour = input.m_vRefractColour;
		screenPos.xy += mul(distorsion, distorsionMatrix) * g_pRefractionIntensity * refractColour.a * texColour.b;
		
		real3 screenColour = h4tex2D(ColourBufferSampler, screenPos).rgb;

		output.m_vColour.rgb = texColour.rrr * input.m_vColour.rgb * texColour.b * input.m_vColour.a + screenColour * lerp(real3(1.h,1.h,1.h), refractColour.rgb, texColour.b);
		output.m_vColour.a = 1.h;
	#else
		#ifdef LOD
		output.m_vColour =  texColour.rrrb * lerp(g_vColour, g_vColourFresnel, input.m_vTexCoord.z) * input.m_vTexCoord.w;
		#else
		output.m_vColour =  texColour.rrrb * input.m_vColour;
		#endif
		output.m_vColour.rgb *= output.m_vColour.a;

		#ifdef DOUBLELAYER
			float4 extraColour =  texColour2L.rrrb * input.m_vColour2L;
			output.m_vColour.rgb += extraColour.rgb * extraColour.a;
			output.m_vColour.a += extraColour.a;
		#endif
	#endif

	#ifndef DEPTHFADEDISABLE
	#if !defined(CFG_CONVERTER) && !defined(LOD)
		//Soft edges detection
		float	fScreenDepth	= SampleDepthBuffer(DepthBufferSampler, screenPos, g_vDepthScale);
		float   fEffectDepth    = input.m_ScreenPosition.w;

		real fadeFactor = saturate((fScreenDepth - fEffectDepth)) * (1.h - saturate(input.m_ScreenPosition.z / input.m_ScreenPosition.w) );
		
		#if !defined(CFG_CONVERTER) && defined(REFRACT) && !defined(LOD)
		output.m_vColour.rgb = lerp(screenColour, output.m_vColour.rgb, fadeFactor);
		#else
		output.m_vColour *= fadeFactor;
		#endif
	#endif
	#endif

	//Negative
	real negative = min(output.m_vColour.r, min(output.m_vColour.g, output.m_vColour.b)) * g_fNegativeFactor;
	negative = saturate(-negative);
	output.m_vColour.rgb = max(real3(0.h, 0.h, 0.h), output.m_vColour.rgb);

#if !defined(REFRACT) || defined(LOD)
	output.m_vColour.a = max(negative, (1.0h / 255.0h) * sign(output.m_vColour.a));
#else
	output.m_vColour.a = 1.h;
#endif
	return output;
}

#if !defined(REFRACT)
technique Material < string SortType = "backfront"; >
#else
technique Material < string SortType = "postresolve"; >
#endif
{
	pass p0
	{
	#if !defined(CFG_CONVERTER) && defined(REFRACT)
		AlphaBlendEnable = FALSE;
		AlphaTestEnable = FALSE;
	#else
		AlphaBlendEnable = TRUE;
		AlphaTestEnable = TRUE;
	#ifdef NEGATIVE
		BlendOp = REVSUBTRACT;
	#else
		BlendOp	= Add;	
	#endif
		SrcBlend = One;
		DestBlend = InvSrcAlpha;
	#endif
		ZWriteEnable = FALSE;
		#ifndef REFRACT
			#ifndef CULLENABLE
				CullMode = NONE;
			#else
				CullMode = CW;
			#endif
		#else
			CullMode = CCW;
		#endif
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}

technique MaterialLOD < string SortType = "backfront"; string Define = "LOD";>
{
	pass p0
	{
	#if !defined(CFG_CONVERTER) && defined(REFRACT)
		AlphaBlendEnable = FALSE;
		AlphaTestEnable = FALSE;
	#else
		AlphaBlendEnable = TRUE;
		AlphaTestEnable = TRUE;
	#ifdef NEGATIVE
		BlendOp = REVSUBTRACT;
	#else
		BlendOp	= Add;	
	#endif
		SrcBlend = One;
		DestBlend = InvSrcAlpha;
	#endif
		ZWriteEnable = FALSE;
		#ifndef REFRACT
			#ifndef CULLENABLE
				CullMode = NONE;
			#else
				CullMode = CW;
			#endif
		#else
			CullMode = CCW;
		#endif

		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}
