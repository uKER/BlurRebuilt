
// view position
real4	g_ViewPosition	: register(CCP_DISPLAY_VIEW_POSITION);

// fog
real4	g_FogColour		: register(CCP_RENDER_FOG_COLOUR);

real4	g_DepthFog		: register(CCP_RENDER_FOG_DEPTH_SETTINGS);
#define g_DepthFogDensity		g_DepthFog.x
#define g_DepthFogNearDistance	g_DepthFog.y
#define g_DepthFogScale			g_DepthFog.z

real4	g_HeightFog		: register(CCP_RENDER_FOG_HEIGHT_SETTINGS);
#define g_HeightFogHeight	g_HeightFog.x
#define g_HeightFogDensity	g_HeightFog.y
#define g_HeightFogFalloff	g_HeightFog.z
#define g_HeightFogScale	g_HeightFog.w

real4	g_BlendSettings	: register(CCP_RENDER_BLEND_SETTINGS);
#define	g_BlendAddSubtract				g_BlendSettings.x
#define	g_BlendAlphaPreMultiply			g_BlendSettings.y
#define	g_BlendAlphaModulate			g_BlendSettings.z
#define	g_BlendAlpha					g_BlendSettings.w

// this function gets the fog colour depending on alpha settings
real3 GetFogColour(real alpha)
{
	real3 fogColour = g_FogColour.xyz;

#ifdef ALPHA
	// take into account additive or subtractive blending
	fogColour *= g_BlendAddSubtract;
#endif

	return fogColour;
}

real CalculateHeightFogAmount(real fWorldHeight, real fViewHeight, real fDistance)
{
	real	f0				= (fWorldHeight - g_HeightFogHeight);
	real	f1				= (fViewHeight - g_HeightFogHeight);

	real	fMin			= min(f0, f1);
	real	fMax			= max(f0, f1);

	real	fFogRatio		= saturate(fMin / (fMin - fMax));
	real	fFogDistance	= fFogRatio * fDistance * saturate(-fMin * g_HeightFogFalloff);

	return -fFogDistance * g_HeightFogDensity * g_HeightFogScale;
}

real CalculateDepthFogAmount(real fDistance)
{
	fDistance = max(fDistance - g_DepthFogNearDistance, 0.0f);
	return -fDistance * g_DepthFogDensity * g_DepthFogScale;
}

real4 EvaluateFog(real4 vColour, real fFogAmount)
{
	real3 vFogColour = GetFogColour(vColour.a);

	real fFogFactor = exp(fFogAmount);

	return real4(lerp(vFogColour, vColour.xyz, fFogFactor), vColour.a);
}

real4 EvaluateHeightDepthFog(real4 vColour, real fWorldHeight, float fDistance)
{
	real fHeightFogAmount = CalculateHeightFogAmount(fWorldHeight, g_ViewPosition.y, fDistance);
	real fDepthFogAmount = CalculateDepthFogAmount(fDistance);

	return EvaluateFog(vColour, fDepthFogAmount + fHeightFogAmount);
}

real4 EvaluateHeightFog(real4 vColour, real fWorldHeight, real fDistance)
{
	return EvaluateFog(vColour, CalculateHeightFogAmount(fWorldHeight, g_ViewPosition.y, fDistance));
}

real4 EvaluateDepthFog(real4 vColour, real fDistance)
{
	return EvaluateFog(vColour, CalculateDepthFogAmount(fDistance));
}



