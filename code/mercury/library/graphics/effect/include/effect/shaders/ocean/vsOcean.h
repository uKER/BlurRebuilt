#if defined(VS_2_0)
#define tex2Dlod(A, B) float4(0.0f, 0.0f, 0.0f, 0.0f)
#endif

#if defined(ANIMATION_HEIGHT_MAP)
extern float4		g_vHeightData	: register(CCV_EFFECT_OCEAN_HEIGHTMAP_SETTINGS);
extern sampler2D	g_tHeight		: register(s0);
#endif

#if defined(ANIMATION_HEIGHT_DX_DY_MAP)
extern sampler2D	g_tDX			: register(s1);
extern sampler2D	g_tDY			: register(s2);
#endif

#if defined(ANIMATION_HEIGHT_GRADIENT_MAP)
extern sampler2D	g_tGradient		: register(s1);
#endif

#if defined(ANIMATION_HEIGHT_NORMAL_MAP)
extern sampler2D	g_tNormal		: register(s1);
#endif

float4 GetHeightCoordinates(float3 vWorldPosition, float4 vData)
{
	return float4(vWorldPosition.xz * vData.x, 0.0f, 0.0f);
}

void GetDisplacementAndNormal(in float3 vWorldPosition, out float3 vDisplacement, out float3 vNormal, out float3 vTangent, out float3 vBinormal)
{
	#if defined(ANIMATION_HEIGHT_MAP)
	float4 vTexCoord0 = GetHeightCoordinates(vWorldPosition, g_vHeightData);

	float fHeight = g_vHeightData.y * tex2Dlod(g_tHeight, vTexCoord0).x;

	#if defined(ANIMATION_HEIGHT_DX_DY_MAP)
	float2 vDX;
	vDX.x = tex2Dlod(g_tDX, vTexCoord0).x;
	vDY.y = tex2Dlod(g_tDY, vTexCoord0).x;
	vNormal = normalize(float3(-vDX.x, 1.0f, -vDX.y));
	vTangent = normalize(float3(0.0f, vDX.y, 1.0f));
	vBinormal = normalize(float3(1.0f, vDX.x, 0.0f));
	#elif defined(ANIMATION_HEIGHT_GRADIENT_MAP)
	float2 vDX = tex2Dlod(g_tGradient, vTexCoord0).xy;
	vNormal = normalize(float3(-vDX.x, 1.0f, -vDX.y));
	vTangent = normalize(float3(0.0f, vDX.y, 1.0f));
	vBinormal = normalize(float3(1.0f, vDX.x, 0.0f));
	#elif defined(ANIMATION_HEIGHT_NORMAL_MAP)
	vNormal = normalize(tex2Dlod(g_tNormal, vTexCoord0).xyz * 2.0f - 1.0f);
	vTangent = cross(float3(0.0f, 0.0f, 1.0f), vNormal);
	vBinormal = cross(vNormal, float3(1.0f, 0.0f, 0.0f));
	float2 vDX = -vNormal.xz;
	#else
	vNormal = float3(0.0f, 1.0f, 0.0f);
	vTangent = float3(1.0f, 0.0f, 0.0f);
	vBinormal = float3(0.0f, 0.0f, 1.0f);
	#endif

	vDisplacement = float3(-vDX.x * g_vHeightData.z, fHeight, -vDX.y * g_vHeightData.z);

	#else

	vDisplacement = 0.0f;
	vNormal = float3(0.0f, 1.0f, 0.0f);
	vTangent = float3(1.0f, 0.0f, 0.0f);
	vBinormal = float3(0.0f, 0.0f, 1.0f);

	#endif // ANIMATION_HEIGHT_MAP
}
