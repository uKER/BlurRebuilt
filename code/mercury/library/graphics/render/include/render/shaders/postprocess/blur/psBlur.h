#define		g_vBlurFocus	g_vBlurSettings.xy
#define		g_fBlurAmount	g_vBlurSettings.z
#define		g_fBlurSamples	g_vBlurSettings.w
#define		g_fBlurWeight0	g_vBlurWeights[0].x
#define		g_fBlurWeight1	g_vBlurWeights[0].y
#define		g_fBlurWeight2	g_vBlurWeights[0].z
#define		g_fBlurWeight3	g_vBlurWeights[0].w
#define		g_fBlurWeight4	g_vBlurWeights[1].x
#define		g_fBlurWeight5	g_vBlurWeights[1].y
#define		g_fBlurWeight6	g_vBlurWeights[1].z
#define		g_fBlurWeight7	g_vBlurWeights[1].w
#define		g_fBlurWeight8	g_vBlurWeights[2].x
#define		g_fBlurWeight9	g_vBlurWeights[2].y
#define		g_fBlurWeight10	g_vBlurWeights[2].z
#define		g_fBlurWeight11	g_vBlurWeights[2].w

float3 GetRadialBlur4Samples(sampler2D colourTexture, float2 texCoord)
{
	float3 colour = 0.0f;
	float2 toFocus = texCoord * 2.0f - 1.0f - g_vBlurFocus;

	colour	+=	g_fBlurWeight0 * tex2D(colourTexture, texCoord).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight1 * tex2D(colourTexture, (toFocus + g_vBlurFocus) * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight2 * tex2D(colourTexture, (toFocus + g_vBlurFocus) * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight3 * tex2D(colourTexture, (toFocus + g_vBlurFocus) * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	return colour;
}

float3 GetRadialBlur6Samples(sampler2D colourTexture, float2 texCoord)
{
	float3 colour = 0.0f;
	float2 toFocus = texCoord * 2.0f - 1.0f - g_vBlurFocus;

	colour	+=	g_fBlurWeight0 * tex2D(colourTexture, texCoord).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight1 * tex2D(colourTexture, (toFocus + g_vBlurFocus) * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight2 * tex2D(colourTexture, (toFocus + g_vBlurFocus) * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight3 * tex2D(colourTexture, (toFocus + g_vBlurFocus) * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight4 * tex2D(colourTexture, (toFocus + g_vBlurFocus) * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight5 * tex2D(colourTexture, (toFocus + g_vBlurFocus) * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	return colour;
}

float3 GetRadialBlur8Samples(sampler2D colourTexture, float2 texCoord)
{
	float3 colour = 0.0f;
	float2 toFocus = texCoord * 2.0f - 1.0f - g_vBlurFocus;

	colour	+=	g_fBlurWeight0 * tex2D(colourTexture, texCoord).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight1 * tex2D(colourTexture, (toFocus + g_vBlurFocus) * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight2 * tex2D(colourTexture, (toFocus + g_vBlurFocus) * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight3 * tex2D(colourTexture, (toFocus + g_vBlurFocus) * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight4 * tex2D(colourTexture, (toFocus + g_vBlurFocus) * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight5 * tex2D(colourTexture, (toFocus + g_vBlurFocus) * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight6 * tex2D(colourTexture, (toFocus + g_vBlurFocus) * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight7 * tex2D(colourTexture, (toFocus + g_vBlurFocus) * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	return colour;
}

float3 GetRadialBlur10Samples(sampler2D colourTexture, float2 texCoord)
{
	float3 colour = 0.0f;
	float2 toFocus = texCoord * 2.0f - 1.0f - g_vBlurFocus;

	colour	+=	g_fBlurWeight0 * tex2D(colourTexture, texCoord).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight1 * tex2D(colourTexture, (toFocus + g_vBlurFocus) * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight2 * tex2D(colourTexture, (toFocus + g_vBlurFocus) * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight3 * tex2D(colourTexture, (toFocus + g_vBlurFocus) * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight4 * tex2D(colourTexture, (toFocus + g_vBlurFocus) * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight5 * tex2D(colourTexture, (toFocus + g_vBlurFocus) * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight6 * tex2D(colourTexture, (toFocus + g_vBlurFocus) * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight7 * tex2D(colourTexture, (toFocus + g_vBlurFocus) * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight8 * tex2D(colourTexture, (toFocus + g_vBlurFocus) * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight9 * tex2D(colourTexture, (toFocus + g_vBlurFocus) * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	return colour;
}

float3 GetRadialBlur12Samples(sampler2D colourTexture, float2 texCoord)
{
	float3 colour = 0.0f;
	float2 toFocus = texCoord * 2.0f - 1.0f - g_vBlurFocus;

	colour	+=	g_fBlurWeight0 * tex2D(colourTexture, texCoord).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight1 * tex2D(colourTexture, (toFocus + g_vBlurFocus) * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight2 * tex2D(colourTexture, (toFocus + g_vBlurFocus) * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight3 * tex2D(colourTexture, (toFocus + g_vBlurFocus) * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight4 * tex2D(colourTexture, (toFocus + g_vBlurFocus) * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight5 * tex2D(colourTexture, (toFocus + g_vBlurFocus) * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight6 * tex2D(colourTexture, (toFocus + g_vBlurFocus) * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight7 * tex2D(colourTexture, (toFocus + g_vBlurFocus) * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight8 * tex2D(colourTexture, (toFocus + g_vBlurFocus) * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight9 * tex2D(colourTexture, (toFocus + g_vBlurFocus) * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight10 * tex2D(colourTexture, (toFocus + g_vBlurFocus) * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight11 * tex2D(colourTexture, (toFocus + g_vBlurFocus) * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;
	
	return colour;
}

float3 GetRadialBlurFromZero4Samples(sampler2D colourTexture, float2 texCoord)
{
	float3 colour = 0.0f;
	float2 toFocus = texCoord * 2.0f - 1.0f;

	colour	+=	g_fBlurWeight0 * tex2D(colourTexture, texCoord).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight1 * tex2D(colourTexture, toFocus * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight2 * tex2D(colourTexture, toFocus * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight3 * tex2D(colourTexture, toFocus * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	return colour;
}

float3 GetRadialBlurFromZero6Samples(sampler2D colourTexture, float2 texCoord)
{
	float3 colour = 0.0f;
	float2 toFocus = texCoord * 2.0f - 1.0f;

	colour	+=	g_fBlurWeight0 * tex2D(colourTexture, texCoord).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight1 * tex2D(colourTexture, toFocus * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight2 * tex2D(colourTexture, toFocus * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight3 * tex2D(colourTexture, toFocus * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight4 * tex2D(colourTexture, toFocus * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight5 * tex2D(colourTexture, toFocus * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	return colour;
}

float3 GetRadialBlurFromZero8Samples(sampler2D colourTexture, float2 texCoord)
{
	float3 colour = 0.0f;
	float2 toFocus = texCoord * 2.0f - 1.0f;

	colour	+=	g_fBlurWeight0 * tex2D(colourTexture, texCoord).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight1 * tex2D(colourTexture, toFocus * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight2 * tex2D(colourTexture, toFocus * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight3 * tex2D(colourTexture, toFocus * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight4 * tex2D(colourTexture, toFocus * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight5 * tex2D(colourTexture, toFocus * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight6 * tex2D(colourTexture, toFocus * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight7 * tex2D(colourTexture, toFocus * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	return colour;
}

float3 GetRadialBlurFromZero10Samples(sampler2D colourTexture, float2 texCoord)
{
	float3 colour = 0.0f;
	float2 toFocus = texCoord * 2.0f - 1.0f;

	colour	+=	g_fBlurWeight0 * tex2D(colourTexture, texCoord).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight1 * tex2D(colourTexture, toFocus * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight2 * tex2D(colourTexture, toFocus * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight3 * tex2D(colourTexture, toFocus * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight4 * tex2D(colourTexture, toFocus * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight5 * tex2D(colourTexture, toFocus * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight6 * tex2D(colourTexture, toFocus * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight7 * tex2D(colourTexture, toFocus * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight8 * tex2D(colourTexture, toFocus * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight9 * tex2D(colourTexture, toFocus * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	return colour;
}

float3 GetRadialBlurFromZero12Samples(sampler2D colourTexture, float2 texCoord)
{
	float3 colour = 0.0f;
	float2 toFocus = texCoord * 2.0f - 1.0f;

	colour	+=	g_fBlurWeight0 * tex2D(colourTexture, texCoord).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight1 * tex2D(colourTexture, toFocus * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight2 * tex2D(colourTexture, toFocus * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight3 * tex2D(colourTexture, toFocus * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight4 * tex2D(colourTexture, toFocus * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight5 * tex2D(colourTexture, toFocus * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight6 * tex2D(colourTexture, toFocus * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight7 * tex2D(colourTexture, toFocus * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight8 * tex2D(colourTexture, toFocus * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight9 * tex2D(colourTexture, toFocus * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight10 * tex2D(colourTexture, toFocus * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;

	colour	+=	g_fBlurWeight11 * tex2D(colourTexture, toFocus * 0.5f + 0.5f).rgb;
	toFocus	*=	g_fBlurAmount;
	
	return colour;
}