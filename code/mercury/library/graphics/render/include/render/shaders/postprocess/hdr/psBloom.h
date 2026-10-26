sampler2D	bloomTexture0		: register(s0);
sampler2D	bloomTexture1		: register(s1);
sampler2D	bloomTexture2		: register(s2);
sampler2D	bloomTexture3		: register(s3);

float4		g_vBloomSettings	: register(CCP_RENDER_POSTPROCESS_PARAMETERS_2);

#define		g_fBloomScale0		g_vBloomSettings.x
#define		g_fBloomScale1		g_vBloomSettings.y
#define		g_fBloomScale2		g_vBloomSettings.z
#define		g_fBloomScale3		g_vBloomSettings.w

#if defined(PLATFORM_PS3)
#pragma texformat bloomTexture0 RGBA8
#pragma texformat bloomTexture1 RGBA8
#pragma texformat bloomTexture2 RGBA8
#pragma texformat bloomTexture3 RGBA8
#endif
