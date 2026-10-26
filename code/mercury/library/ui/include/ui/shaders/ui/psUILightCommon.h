float4		g_LightPos			: register(CCP_LIGHTPOS);
float3		g_LightDir			: register(CCP_LIGHTDIR);
float4		g_LightColourFrom	: register(CCP_LIGHTCOLFROM);
float4		g_LightColourTo		: register(CCP_LIGHTCOLTO);

float		g_LightFallOff		: register(CCP_LIGHTFALLOFF);

float4 LightColour( float4 worldPosition )
{
	float3 dir = worldPosition.xyz - g_LightPos.xyz;
	
	float t = saturate( dot( g_LightDir, dir ) / g_LightFallOff );
	
	float4 col = lerp( g_LightColourFrom, g_LightColourTo, t );
	
	return col;
}

float4 CombineFog( float4 colour, float4 fog)
{
//	return float4(colour.xyz * fog.a + fog.xyz, colour.a);
	return float4(colour.xyz * fog.a + fog.xyz, colour.a * fog.a);
}
