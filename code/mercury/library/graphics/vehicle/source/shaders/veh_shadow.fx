#include <Shader/Core.h>
#include <Shader/Globals.h>


shared float4x4	g_WorldMatrix							: register(CCV_SHADER_USER_16);
shared float4x4	g_ShadowPlaneSpaceMatrix				: register(CCV_SHADER_USER_20);
shared float4		g_ShadowPlane						: register(CCV_SHADER_USER_24);
shared float4		g_LightPosition						: register(CCV_SHADER_USER_25); //w -> Z projected shadow
shared float4		g_ShadowAlpha						: register(CCV_SHADER_USER_26);

struct C_VertexInput
{
	float3	m_Position	: POSITION;
};

struct C_VertexOutput
{
	float4	m_Position	: POSITION;
	float2	m_ProjectPos : TEXCOORD0;
};

struct C_VertexOutputLOD
{
	float4	m_Position	: POSITION;
};

struct C_PixelOutput
{
	real4	m_vColour	: COLOR0;
};

C_VertexOutput MainVS(in C_VertexInput input)
{
	C_VertexOutput output = (C_VertexOutput) 0;
	
	float4 objectPosition = float4(input.m_Position, 1.f);
	float4 worldPosition = mul(objectPosition, g_WorldMatrix);

	//Calculate shadow plane intersection
	float u = (dot(float4(g_LightPosition.xyz, 1.f) , g_ShadowPlane))/(dot(g_ShadowPlane.xyz,(g_LightPosition.xyz - worldPosition.xyz).xyz));
	float4 shadowPosition = float4(g_LightPosition.xyz + (worldPosition.xyz - g_LightPosition.xyz) * u, 1.f); 
	
	//Convert to shadow texture space (2D)
	float3 shadowSpacePosition = mul(shadowPosition, g_ShadowPlaneSpaceMatrix);
	
	output.m_Position = float4(shadowSpacePosition.x, shadowSpacePosition.y, g_LightPosition.w, 1.f);
	output.m_ProjectPos.xy = output.m_Position.xy;
	
	return output;
}

C_PixelOutput MainPS(in C_VertexOutput input)
{
	C_PixelOutput output = (C_PixelOutput) 0;

	//kill using radius
	real fShadow = g_ShadowAlpha.x;
	
	float distant = sqrt(dot(input.m_ProjectPos,input.m_ProjectPos));
	 
	fShadow *= (real)saturate( (1.f - distant) * g_ShadowAlpha.y);

	output.m_vColour = real4(fShadow,fShadow,fShadow,fShadow);
	return output;
}

C_VertexOutputLOD MainVSLOD(in C_VertexInput input)
{
	C_VertexOutputLOD output = (C_VertexOutputLOD) 0;
	
	float4 position			= mul(float4(input.m_Position,1.f), g_mWorld);
	output.m_Position		= mul(position, g_mViewProj); 
	
	return output;
}

C_PixelOutput MainPSLOD(in C_VertexOutputLOD input)
{
	C_PixelOutput output = (C_PixelOutput) 0;
	output.m_vColour = real4(0.h, 0.h, 0.h, 1.h);

	return output;
}

technique Default
{
	pass p0
	{
		AlphaBlendEnable = FALSE;
 		AlphaTestEnable = FALSE;
		CullMode = CW;
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}

// Hack:
// The LOD version will be use to render a reduced version of the car in the reflection, it is the shadowcaster with LOD technique
technique DefaultLOD < string Define = "LOD"; >
{
pass p0
{
	AlphaBlendEnable = FALSE;
	AlphaTestEnable = FALSE;
	CullMode = CW;
	VertexShader = compile vs_3_0 MainVSLOD();
	PixelShader = compile ps_3_0 MainPSLOD();
}
}
