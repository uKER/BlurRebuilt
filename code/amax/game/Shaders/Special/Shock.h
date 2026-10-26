#ifndef AMAX_SHADERS_SPECIAL_SHOCK_H
#define AMAX_SHADERS_SPECIAL_SHOCK_H


#ifdef CFG_DEBUG
	float	g_vShockSpeed	< string DebugVariable = "ShockSpeed"; string Default = "0.33"; string Type = "Float"; >	= 0.33f;
#else
	const real	g_vShockSpeed = 0.33h;
#endif

shared real4	g_vShockIntensity; // W->Time
shared texture2D	g_tShockTexture;

sampler ShockSampler = 
sampler_state
{
	Texture = <g_tShockTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

#if defined(PLATFORM_PS3) && defined(__PIXEL__)
#pragma texformat g_tShockTexture COMPRESSED_RGBA_S3TC_DXT1
#endif

C_PixelOutput MainElectricPS(in C_VertexOutput vtxData) 
{ 
	C_PixelOutput output = MainPS(vtxData);

	real	fElectricPos	= frac(g_fTime * g_vShockSpeed);
	
	real3	vValue	= h4tex2D(ShockSampler, vtxData.m_vTexCoord.xy).rgb;
			vValue	= frac(vValue + real3(0.15h, 0.50h, 0.75h));
	real3	vDiff	= min(frac(vValue - fElectricPos), frac(-vValue + fElectricPos));
	real	fDiff	= min(vDiff.x, min(vDiff.y, vDiff.z));
	real	fShock	= pow(saturate(1.0h - 3.0h * fDiff), 10.0h);

	output.m_vColour.rgb += g_vShockIntensity.rgb * fShock;

	return output;
}

#endif // AMAX_SHADERS_SPECIAL_SHOCK_H
