sampler2D	g_AlbedoTexture		: register(s0);

struct PS_INPUT
{
	float4 Normal				: TEXCOORD0;
	float4 TexCoord				: TEXCOORD1;
};

struct PS_OUTPUT
{
	float4 Colour				: COLOR0;
};

PS_OUTPUT main(PS_INPUT input)
{
	PS_OUTPUT output = (PS_OUTPUT)0;

	float4 albedoColour = tex2D(g_AlbedoTexture, input.TexCoord.xy);

	output.Colour.xyz = albedoColour.xyz;
	output.Colour.w = albedoColour.w;

	return output;
}