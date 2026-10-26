#include <Core.h>
#include <Globals.h>

// road surface shader
#include <Road.h>

// three colour, one specular

texture2D	g_tDetailAlbedo		< string Source = "DetailColour"; string Default = "Black"; string Define = "ND"; >;

sampler DetailAlbedoSampler = 
sampler_state
{
    Texture = <g_tDetailAlbedo>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	SRGB_TEXTURE
};

texture2D	g_tDirtAlbedo		< string Source = "DirtColour"; string Default = "Black"; string Define = "NG"; >;

sampler DirtAlbedoSampler = 
sampler_state
{
    Texture = <g_tDirtAlbedo>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	SRGB_TEXTURE
};

texture2D	g_tSpecular	< string Source = "<Ipr0+Colour>"; string MipFilter="BOX"; string Default = "Black"; >;

sampler SpecularSampler = 
sampler_state
{
    Texture = <g_tSpecular>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	SRGB_TEXTURE
};

texture2D	g_tDetailSpecular	< string Source = "<Ipr0+DetailColour>"; string MipFilter="BOX"; string Default = "Black"; >;

sampler DetailSpecularSampler = 
sampler_state
{
    Texture = <g_tDetailSpecular>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	SRGB_TEXTURE
};

#if defined(PLATFORM_PS3) && defined(__PIXEL__)
#pragma texformat g_tDetailAlbedo COMPRESSED_RGBA_S3TC_DXT1
#pragma texformat g_tDirtAlbedo COMPRESSED_RGBA_S3TC_DXT1
#pragma texformat g_tSpecular COMPRESSED_RGBA_S3TC_DXT1
#pragma texformat g_tDetailSpecular COMPRESSED_RGBA_S3TC_DXT1
#endif

C_Material EvaluateMaterial(in C_VertexOutput vtxData)
{
	C_Material mtlData				= (C_Material) 0;

	real	fDetailBlend			= 1.0h;
	real	fDirtBlend				= 1.0h;
#if defined(ENABLE_VERTEXCOLOURS)
			fDetailBlend			*= vtxData.m_vColour.a;
			fDirtBlend				*= vtxData.m_vColour.r;
#endif

	float2	vTexCoord				= vtxData.m_vTexCoord.xy;
	real4	vAlbedo					= h4tex2D(AlbedoSampler, vTexCoord).xyzw;
#if !defined(NO_DETAIL)
	float2	vDetailCoord			= vtxData.m_vExtraCoord.xy;
	real4	vDetailAlbedo			= h4tex2D(DetailAlbedoSampler, vDetailCoord).xyzw;
			vAlbedo.rgb				= lerp(vAlbedo.rgb, vDetailAlbedo.rgb, vDetailAlbedo.a * fDetailBlend);
#endif
#if !defined(NO_DIRT)
	float2	vDirtCoord				= vtxData.m_vExtraCoord.zw;
	real4	vDirtAlbedo				= h4tex2D(DirtAlbedoSampler, vDirtCoord).xyzw;
			vAlbedo.rgb				= lerp(vAlbedo.rgb, vDirtAlbedo.rgb, vDirtAlbedo.a * fDirtBlend);
#endif
	mtlData.m_vAlbedo				= vAlbedo.rgb;
	mtlData.m_fAlpha				= vAlbedo.a;

	mtlData							= EvaluateTangentNormal(mtlData, vTexCoord);

	real	fSpecIntensity			= h4tex2D(SpecularSampler, vTexCoord).r;
#if !defined(NO_DETAIL)
	real	fDetailSpecIntensity	= h4tex2D(DetailSpecularSampler, vDetailCoord).r;
			fSpecIntensity			= lerp(fSpecIntensity, fDetailSpecIntensity, vDetailAlbedo.a * fDetailBlend);
#endif
	mtlData.m_fSpecularIntensity	= max(0.0h, fSpecIntensity + g_fRoadReflectionBrightness);
	mtlData.m_fHalfSmoothness		= g_fRoadSurfaceSmoothness;
	mtlData.m_fFresnelMidBrightness	= 0.0h;
	mtlData.m_vEmissive				= 0.0h;

	return mtlData;
}

C_Surface EvaluateSurface(in C_Material mtlData, in C_VertexOutput vtxData)
{
	C_Surface surfaceData	= (C_Surface) 0;
#if defined(ENABLE_WORLD)
	surfaceData.m_vPosition	= vtxData.m_vWorld.xyz;
#endif
#if defined(ENABLE_NORMAL)
#if defined(NO_WORLDNORMAL)
	surfaceData.m_vNormal	= normalize(vtxData.m_vNormal.xyz);
#else
	surfaceData.m_vNormal	= normalize(h4tex2D(WorldNormalSampler, vtxData.m_vTexCoord.xy).xzy - 0.5h);
#endif
#endif
#if defined(ENABLE_VIEW)
	surfaceData.m_vView		= normalize(vtxData.m_vView.xyz);
#endif
#if !defined(LOD)
	surfaceData.m_fDistance	= vtxData.m_vScreenPos.z;
#endif
#if defined(ENABLE_SPECULAR)
	surfaceData.m_fNDotV	= dot(surfaceData.m_vNormal, surfaceData.m_vView);
	surfaceData.m_vReflect	= surfaceData.m_vView - 2.0h * surfaceData.m_fNDotV * surfaceData.m_vNormal;
	
	surfaceData.m_fFresnel	= CalculateFresnelFactor(surfaceData.m_vNormal) *  mtlData.m_fSpecularIntensity;
#endif
	return surfaceData;
}

// pixel shader

C_PixelOutput RoadSurfacePS(in C_VertexOutput vtxData)
{
    C_PixelOutput output = (C_PixelOutput) 0;

	C_Material	mtlData		= EvaluateMaterial(vtxData);
	C_Surface	surfaceData	= EvaluateSurface(mtlData, vtxData);
	C_Lighting	lightData	= EvaluateLighting(mtlData, surfaceData, vtxData);

	real3	vDiffuse		= EvaluateDiffuse(mtlData, surfaceData, lightData, vtxData);
	real3	vSpecular		= EvaluateSpecular(mtlData, surfaceData, lightData, vtxData);

	output.m_vColour.xyz	= EvaluateColour(vDiffuse, vSpecular, mtlData, surfaceData);
	output.m_vColour.w		= mtlData.m_fAlpha;

#if defined(ENABLE_FOG) && !defined(LOD)
	output.m_vColour		= EvaluateDepthFog(output.m_vColour, surfaceData.m_fDistance);
#endif
	output.m_vColour		= EvaluateFinalColour(output.m_vColour);

	output.m_vColour		= DebugColour(output.m_vColour, mtlData, vtxData);

    return output;
}

#if defined(ENABLE_PREPASS_DEPTHTEXTURE)
C_PixelOutputD RoadSurfacePrepassPS(in C_VertexPrepassOutput vtxData)
{
    C_PixelOutputD output = (C_PixelOutputD) 0;
#else
C_PixelOutput RoadSurfacePrepassPS(in C_VertexPrepassOutput vtxData)
{
    C_PixelOutput output = (C_PixelOutput) 0;
#endif

#if defined(ENABLE_PREPASS_VIEWDEPTH)
	
#if !defined(NO_WORLDNORMAL)
	float	fDepth255		= vtxData.m_vTexCoord.z * 255.0f;
	real3	vWorldNormal	= EvaluateWorldNormal(vtxData, vtxData.m_vTexCoord.xy);
	real2	vViewNormal;
	vViewNormal.x			= dot(vWorldNormal, g_vViewMatrix0.xyz);
	vViewNormal.y			= dot(vWorldNormal, g_vViewMatrix1.xyz);
	output.m_vColour = real4(vViewNormal.xy * 0.5h + 0.5h, floor(fDepth255) / 255.0f, frac(fDepth255));
#else
	float	fDepth255		= vtxData.m_vNormal.w * 255.0f;
	output.m_vColour = real4(vtxData.m_vNormal.xy * 0.5h + 0.5h, floor(fDepth255) / 255.0f, frac(fDepth255));
#endif

#else

#if defined(ENABLE_PREPASS_VIEWSPACE) && !defined(NO_WORLDNORMAL)
	real3	vWorldNormal	= EvaluateWorldNormal(vtxData, vtxData.m_vTexCoord.xy);
	real3	vViewNormal;
	vViewNormal.x			= dot(vWorldNormal, g_vViewMatrix0.xyz);
	vViewNormal.y			= dot(vWorldNormal, g_vViewMatrix1.xyz);
	vViewNormal.z			= dot(vWorldNormal, g_vViewMatrix2.xyz);
	output.m_vColour		= real4(vViewNormal * 0.5h + 0.5h, 1.0h);
#elif !defined(NO_WORLDNORMAL)
	real3	vWorldNormal	= EvaluateWorldNormal(vtxData, vtxData.m_vTexCoord.xy);
	output.m_vColour	= real4(vWorldNormal * 0.5h + 0.5h, 1.0h);
#else
#if defined(BLUR_DX11)
	// [Blur2PC][Blur1PCDX11][retail-recovered] D3D9 nrm of the zero _NW normal is
	// zero (0 * rsq(0) = 0); SM5 normalize would give NaN. See Road.fx.
	float3	vNormal			= vtxData.m_vNormal.xyz;
	float3	vNormalised		= vNormal * rsqrt(dot(vNormal, vNormal));
	output.m_vColour	= real4((vNormal == 0.0f ? 0.0f : vNormalised) * 0.5h + 0.5h, 1.0h);
#else
	output.m_vColour	= real4(normalize(vtxData.m_vNormal.xyz) * 0.5h + 0.5h, 1.0h);
#endif
#endif
    
#endif


#if defined(ENABLE_PREPASS_DEPTHTEXTURE)
   #if !defined(NO_WORLDNORMAL)
    output.m_fDepth = mul( vtxData.m_vTexCoord.z,  1 / vtxData.m_vTexCoord.w );
   #else
    output.m_fDepth = vtxData.m_vNormal.z;
   #endif
#endif
    return output;
}

// technique

#define FLAG_EXCLUDE_FROM_REFLECTION 8

technique Material // < int Flags = FLAG_EXCLUDE_FROM_REFLECTION; >
{
	pass p0
	{
		VertexShader = compile vs_3_0 RoadSurfaceVS();
		PixelShader = compile ps_3_0 RoadSurfacePS();
	}
}

technique MaterialLOD < string Define = "LOD"; >
{
	pass p0
	{
		VertexShader = compile vs_3_0 RoadSurfaceVS();
		PixelShader = compile ps_3_0 RoadSurfacePS();
	}
}

#if !defined(CFG_CONVERTER)
technique MaterialLights < string Define = "ENABLE_LIGHTING_PREPASS"; >
{
	pass p0
	{
		VertexShader = compile vs_3_0 RoadSurfaceVS();
		PixelShader = compile ps_3_0 RoadSurfacePS();
	}
}

technique MaterialLSplitScreen <string Define = "SPLITSCREEN; ENABLE_LIGHTING_PREPASS";>
{
	pass p0
	{
		VertexShader = compile vs_3_0 RoadSurfaceVS();
		PixelShader = compile ps_3_0 RoadSurfacePS();
	}
}


#endif

#if !defined(CFG_CONVERTER)
technique MaterialPrepass < string Define = "ENABLE_PREPASS"; >
{
	pass p0
	{
		VertexShader = compile vs_3_0 RoadSurfacePrepassVS();
		PixelShader = compile ps_3_0 RoadSurfacePrepassPS();
	}
}
#endif

#if defined(PLATFORM_PC)
technique HACK < string Define = "ENABLE_HACK"; >
{
	pass p0
	{
		VertexShader = compile vs_3_0 RoadSurfaceVS();
		PixelShader = compile ps_3_0 RoadSurfacePS();
	}
}
#endif
