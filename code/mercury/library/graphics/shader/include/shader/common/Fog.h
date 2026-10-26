#ifndef SHADER_FOG_H
#define SHADER_FOG_H

struct C_FogSettings	annotation(export)
{
	float4	m_vFogColour;
	float4	m_vDepthFog;
	float4	m_vHeightFog;
	float4	m_vBlend;
};

shared C_FogSettings	g_FogSettings	: register(CCP_FOG_SETTINGS) =
	{
		{ 0.25f,	0.25f,		0.25f,	1.0f, },
		{ 0.0001f,	25.0f,		1.0f,	0.0f, },
		{ 5.0f,		0.001f,		1.0f,	1.0f, },
		{ 0.0f,		0.0f,		0.0f,	0.0f, },
	};

#define	g_vFogColour				g_FogSettings.m_vFogColour
#define	g_fDepthFogDensity			g_FogSettings.m_vDepthFog.x
#define	g_fDepthFogNearDistance		g_FogSettings.m_vDepthFog.y
#define	g_fDepthFogScale			g_FogSettings.m_vDepthFog.z
#define	g_fHeightFogHeight			g_FogSettings.m_vHeightFog.x
#define	g_fHeightFogDensity			g_FogSettings.m_vHeightFog.y
#define	g_fHeightFogFalloff			g_FogSettings.m_vHeightFog.z
#define	g_fHeightFogScale			g_FogSettings.m_vHeightFog.w
#define	g_fBlendAddSubtract			g_FogSettings.m_vBlend.x
#define	g_fBlendAlphaPreMultiply	g_FogSettings.m_vBlend.y
#define	g_fBlendAlphaModulate		g_FogSettings.m_vBlend.z
#define	g_fBlendAlpha				g_FogSettings.m_vBlend.w

// this function gets the fog colour depending on alpha settings
real3 GetFogColour(real fAlpha)
{
#if defined(BLACK_FOG)
	return real3(0.0h, 0.0h, 0.0h);
#elif defined(WHITE_FOG)
	return real3(1.0h, 1.0h, 1.0h);
#else
	real3 vFogColour = g_vFogColour.rgb;
#ifdef ALPHA
	// take into account additive or subtractive blending
	vFogColour *= g_fBlendAddSubtract;

	// take into account multiplicative blending
	vFogColour = lerp(vFogColour, 1.0h, g_fBlendAlphaModulate);
#endif

	return vFogColour;
#endif
}

real CalculateHeightFogAmount(real fWorldHeight, real fViewHeight, real fDistance)
{
	real	f0				= (fWorldHeight - g_fHeightFogHeight);
	real	f1				= (fViewHeight - g_fHeightFogHeight);
	
	real	fMin			= min(f0, f1);
	real	fMax			= max(f0, f1);

	real	fFogRatio		= saturate(fMin / (fMin - fMax));
	real	fFogDistance	= fFogRatio * fDistance * saturate(-fMin * g_fHeightFogFalloff);
	
	return -fFogDistance * g_fHeightFogDensity;
}

real CalculateDepthFogAmount(real fDistance)
{
//	fDistance = max(fDistance - g_fDepthFogNearDistance, 0.0h);
	return -fDistance * g_fDepthFogDensity;
}

real4 EvaluateFog(real4 vColour, real fFogAmount)
{
	real3 vFogColour = GetFogColour(vColour.a);

	real fFogFactor = exp(fFogAmount);
	
	return real4(lerp(vFogColour, vColour.xyz, saturate(fFogFactor)), vColour.a);
}

real4 EvaluateHeightDepthFog(real4 vColour, real fWorldHeight, real fViewHeight, float fDistance)
{
	real fHeightFogAmount = CalculateHeightFogAmount(fWorldHeight, fViewHeight, fDistance);
	real fDepthFogAmount = CalculateDepthFogAmount(fDistance);

	return EvaluateFog(vColour, fDepthFogAmount + fHeightFogAmount);
}

real4 EvaluateHeightFog(real4 vColour, real fWorldHeight, real fViewHeight, real fDistance)
{
	return EvaluateFog(vColour, CalculateHeightFogAmount(fWorldHeight, fViewHeight, fDistance));
}

real4 EvaluateDepthFog(real4 vColour, real fDistance)
{
	return EvaluateFog(vColour, CalculateDepthFogAmount(fDistance));
}

#endif // SHADER_FOG_H
