#include <Shader/Core.h>
#include <Shader/Globals.h>
#include <Shader/Common/HDR.h>
#include <Shader/Misc/Resolve.h>

#ifdef BLUR_DX11
// [Blur2PC][HDR] Display state, shared with the native final tone mapper.
cbuffer BlurHDROutput : register(b12) { float4 g_BlurHDROutput; }
#endif

#ifdef LOD
	#ifdef SCROLLDISTORT
		#undef SCROLLDISTORT
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

#ifdef CFG_DEBUG
	float	g_fNegativeFactor   < string DebugVariable = "NegativeFactor"; string Range = "(0,4)"; string Type = "Float"; > = 2.0f;
#else
	const float	g_fNegativeFactor = 2.0f;
#endif

real4	g_vColour			< string Source = "Colour"; string Default = "1.0, 1.0, 1.0, 1.0"; >					= float4(1.0f, 1.0f, 1.0f, 1.0f);
real4	g_vColourFresnel	< string Source = "ColourFresnel"; string Default = "1.0, 1.0, 1.0, 1.0"; >				= float4(1.0f, 1.0f, 1.0f, 1.0f);
real	g_vFresnelPower		< string Source = "FresnelPower"; string Default = "1.0"; string Type = "Float"; >		= 1.0f;
float	g_vScrollTextureX	< string Source = "ScrollTextureX"; string Default = "0.0"; string Type = "Float"; >	= 0.0f;
float	g_vScrollTextureY	< string Source = "ScrollTextureY"; string Default = "0.0"; string Type = "Float"; >	= 0.0f;
float	g_vScrollSpeedX		< string Source = "ScrollSpeedX"; string Default = "0.0"; string Type = "Float"; >		= 0.0f;
float	g_vScrollSpeedY		< string Source = "ScrollSpeedY"; string Default = "0.0"; string Type = "Float"; >		= 0.0f;
float	g_fAlphaFactor		< string Source = "AlphaFactor"; string Default = "1.0"; string Range = "(0,1)"; string Type = "Float"; >		= 1.0f;
float2	g_DistanteFadeParams< string Source = "DistanteFadeParams"; string Default = "100000.0, 10000.0"; string Type = "Float2"; >		= float2(10000.0f, 10000.f);

shared float g_RacificationGlobalFade	: register(CCV_SHADER_USER_1) =    1.f;	

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

#ifdef SCROLLDISTORT
	float	gUvScrollDistortScale	< string Source = "ScrollDistortScale"; string Default = "1.0"; >;
	float	gUvScrollDistortOffsetX	< string Source = "ScrollDistortOffsetX"; string Default = "0.0"; >;
	float	gUvScrollDistortOffsetY	< string Source = "ScrollDistortOffsetY"; string Default = "0.0"; >;
	float	gUvDistort				< string Source = "Distort"; string Default = "1.0"; >;
	
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
	real4	g_2L_vColour			< string Source = "TL_Colour"; string Default = "1.0, 1.0, 1.0, 1.0"; >;
	real4	g_2L_vColourFresnel		< string Source = "TL_ColourFresnel"; string Default = "1.0, 1.0, 1.0, 1.0"; >;
	real	g_2L_vFresnelPower		< string Source = "TL_FresnelPower"; string Default = "1.0"; >;
	float	g_2L_vScrollTextureX	< string Source = "TL_ScrollTextureX"; string Default = "0.0"; >;
	float	g_2L_vScrollTextureY	< string Source = "TL_ScrollTextureY"; string Default = "0.0"; >;
	float	g_2L_vScrollSpeedX		< string Source = "TL_ScrollSpeedX"; string Default = "0.0"; >;
	float	g_2L_vScrollSpeedY		< string Source = "TL_ScrollSpeedY"; string Default = "0.0"; >;
#endif

#ifdef SCROLLDISTORT_DOUBLELAYER
	float	g_2L_UvScrollDistortScale	< string Source = "TL_ScrollDistortScale"; string Default = "1.0"; >;
	float	g_2L_UvScrollDistortOffsetX	< string Source = "TL_ScrollDistortOffsetX"; string Default = "0.0"; >;
	float	g_2L_UvScrollDistortOffsetY	< string Source = "TL_ScrollDistortOffsetY"; string Default = "0.0"; >;
	float	g_2L_UvDistort				< string Source = "TL_Distort"; string Default = "1.0"; >;
	
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
#ifdef SCROLLDISTORT
	#pragma texformat g_tUVDistortTexture LUMINANCE8_ALPHA8
#endif
#ifdef DOUBLELAYER
	#pragma texformat g_2L_tTexture COMPRESSED_RGBA_S3TC_DXT1
#endif
#ifdef SCROLLDISTORT_DOUBLELAYER
	#pragma texformat g_2L_tUVDistortTexture LUMINANCE8_ALPHA8
#endif
#pragma texformat g_tColourBuffer RGBA8
#pragma texformat g_tDepthBuffer RGBA8
#endif

struct C_VertexInput
{
	float3	m_vPosition	: POSITION;
	float4	m_vColour	: COLOR0 annotation(xyzw(colour:vertexColour) | instancemode(instance));
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
};

struct C_VertexOutput
{
	float4	m_vPosition	: POSITION;
	real4	m_vColour	: TEXCOORD1;
	#ifdef DOUBLELAYER
	real4	m_vColour2L	: TEXCOORD2;
	#endif
#ifdef DOUBLELAYER
	float4	m_vTexCoord	: TEXCOORD0;
#else
	float2	m_vTexCoord	: TEXCOORD0;
#endif

	#if !defined(MTOOL) && !defined (LOD)
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

C_VertexOutput MainVS(in C_VertexInput input)
{
	C_VertexOutput output = (C_VertexOutput) 0;
	
	float4	vObjectPosition	= float4(input.m_vPosition, 1.0f);
	float4	vWorldPosition	= mul(vObjectPosition, g_mWorld);
	float4	vProjPosition	= mul(vWorldPosition, g_mViewProj);
	
	output.m_vPosition	= vProjPosition;

	//Calculate Distance Fade

	float distance = length(vWorldPosition.xyz - g_vViewPosition.xyz);
	real distanceFade = 1.f - saturate((distance - g_DistanteFadeParams.x) / (g_DistanteFadeParams.y - g_DistanteFadeParams.x));
	real fade = g_RacificationGlobalFade * distanceFade;

#ifdef CFG_CONVERTER
	//Scroll Texture
	#ifndef SCROLLDISTORT
		output.m_vTexCoord.xy	= input.m_vTexCoord.xy + float2(g_vScrollTextureX,g_vScrollTextureY) + float2(g_vScrollSpeedX,g_vScrollSpeedY) * g_fTime;
	#else
		output.m_vTexCoord.xy	= input.m_vTexCoord.xy;
	#endif
	
	#ifdef DOUBLELAYER
		#ifndef SCROLLDISTORT_DOUBLELAYER
			output.m_vTexCoord.zw	= input.m_vTexCoord2.xy + float2(g_2L_vScrollTextureX,g_2L_vScrollTextureY) + float2(g_2L_vScrollSpeedX,g_2L_vScrollSpeedY) * g_fTime;
		#else
			output.m_vTexCoord.zw	= input.m_vTexCoord2.xy;
		#endif
	#endif
#else
	//Scroll Texture
	#ifndef SCROLLDISTORT
		output.m_vTexCoord.xy	= input.m_vTexCoord.xy + float2(g_vScrollTextureX,g_vScrollTextureY) + float2(g_vScrollSpeedX,g_vScrollSpeedY) * g_fTime;
	#else
		output.m_vTexCoord.xy	= input.m_vTexCoord.xy;
	#endif
	
	#ifdef DOUBLELAYER
		#ifndef SCROLLDISTORT_DOUBLELAYER
			output.m_vTexCoord.zw	= input.m_vTexCoord.zw + float2(g_2L_vScrollTextureX,g_2L_vScrollTextureY) + float2(g_2L_vScrollSpeedX,g_2L_vScrollSpeedY) * g_fTime;
		#else
			output.m_vTexCoord.zw	= input.m_vTexCoord.zw;
		#endif
	#endif
#endif

real fresnel = saturate(1.h - abs(dot(normalize(vWorldPosition.xyz - g_vViewPosition.xyz), normalize( mul(input.m_vNormal,(float3x3)g_mWorld)))));
	fresnel = saturate(pow(fresnel + 0.00001f, g_vFresnelPower));

	output.m_vColour = input.m_vColour * lerp(g_vColour, g_vColourFresnel, fresnel) * fade;
	#ifdef DOUBLELAYER
	output.m_vColour2L = lerp(g_2L_vColour, g_2L_vColourFresnel, fresnel) * fade;
	#endif
	
	#if !defined(CFG_CONVERTER) && !defined(LOD)
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
		real4 texColour = h4tex2D(TextureSampler, main_uv);
	#else
		real4 texColour = h4tex2D(TextureSampler, input.m_vTexCoord.xy);
	#endif

	#ifdef DOUBLELAYER
		#ifdef SCROLLDISTORT_DOUBLELAYER
			float2 distort2l = ((tex2D(UVDistortTextureSampler2L, input.m_UVDistort.zw).ga-0.5f)*2.f);
			float2	main_uv2l	= input.m_vTexCoord.zw + (distort2l.xy + float2(g_2L_vScrollTextureX,g_2L_vScrollTextureY)) * g_2L_UvDistort ;
			real4 texColour2L = h4tex2D(TextureSampler2L, main_uv2l);
		#else
			real4 texColour2L = h4tex2D(TextureSampler2L, input.m_vTexCoord.zw);
		#endif
	#endif

	#if !defined(CFG_CONVERTER) && !defined(LOD)
		float2 screenPos = input.m_ScreenPosition.xy / input.m_ScreenPosition.w;
	#endif

	output.m_vColour =  texColour * input.m_vColour;
	output.m_vColour.rgb *= output.m_vColour.a;

	#ifdef DOUBLELAYER
		float4 extraColour =  texColour2L * input.m_vColour2L;
		output.m_vColour.rgb += extraColour.rgb * extraColour.a;
		output.m_vColour.a += extraColour.a;
	#endif

	#ifdef DEPTHFADE
	#if !defined(CFG_CONVERTER) && !defined(LOD)
		//Soft edges detection
		float	fScreenDepth	= SampleDepthBuffer(DepthBufferSampler, screenPos, g_vDepthScale);
		float   fEffectDepth    = input.m_ScreenPosition.w;

		real fadeFactor = saturate((fScreenDepth - fEffectDepth)) * (1.h - saturate(input.m_ScreenPosition.z / input.m_ScreenPosition.w) );
		
		output.m_vColour *= fadeFactor;
	#endif
	#endif

#ifdef BLUR_DX11
	// [Blur2PC][HDR] Retail's SDR ceiling discards bright additive sources.
	// Keep its exact SDR clamp while allowing positive extended HDR radiance.
	output.m_vColour.rgb = g_BlurHDROutput.x > 0 ? max(output.m_vColour.rgb, 0.0) : saturate(output.m_vColour.rgb);
#else
	output.m_vColour.rgb = saturate(output.m_vColour.rgb);
#endif
	output.m_vColour.a = lerp((1.0f / 255.0f) * sign(output.m_vColour.a), output.m_vColour.a, g_fAlphaFactor);

	return output;
}

technique Material <string SortType = "racification";>
{
	pass p0
	{
		AlphaBlendEnable = TRUE;
		AlphaTestEnable = TRUE;
		BlendOp	= Add;	
		SrcBlend = One;
		DestBlend = InvSrcAlpha;
		ZWriteEnable = FALSE;
	#ifdef NOCULL
		CullMode = NONE;
	#endif
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}

technique MaterialLOD <string Define = "LOD"; string SortType = "racification";>
{
	pass p0
	{
		AlphaBlendEnable = TRUE;
		AlphaTestEnable = TRUE;
		BlendOp	= Add;	
		SrcBlend = One;
		DestBlend = InvSrcAlpha;
		ZWriteEnable = FALSE;
	#ifdef NOCULL
		CullMode = NONE;
	#endif
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}
