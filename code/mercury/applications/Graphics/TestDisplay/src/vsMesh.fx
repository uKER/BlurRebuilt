float4x4	g_WorldMatrix		: register(c0);
float4x4	g_ViewProjMatrix	: register(c4);

struct VS_INPUT
{
	float3 Position				: POSITION;
	float3 Normal				: NORMAL;
	float2 TexCoord				: TEXCOORD;
};

struct VS_OUTPUT
{
	float4 Position				: POSITION;
	float4 Normal				: TEXCOORD0;
	float4 TexCoord				: TEXCOORD1;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	
	float4 objectPosition = float4(input.Position, 1.0f);
	float4 worldPosition = mul(objectPosition, g_WorldMatrix);
	output.Position = mul(worldPosition, g_ViewProjMatrix);

	output.Normal.xyz = normalize(mul(input.Normal, g_WorldMatrix));
	
	output.TexCoord.xy = input.TexCoord;
	
	return output;
}