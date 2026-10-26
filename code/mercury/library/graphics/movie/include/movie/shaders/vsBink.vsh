#ifdef PS3

uniform float4 Adjust : register(c0);
uniform float4 UVRange : register(c1);

void main( float2 iPos : POSITION, float2 iTex : TEXCOORD0, out float4 oPos:POSITION, out float4 oTex: TEXCOORD0 )
{
	oTex.xy = iTex * UVRange.xy;
	oTex.zw = iTex * UVRange.zw;
	oPos.x = iPos.x * Adjust.z;
	oPos.y = iPos.y * Adjust.w;
	oPos.x += Adjust.x;
	oPos.y += Adjust.y;
	oPos.z = 0.5f;
	oPos.w = 1.0f;
}

#else // PS3

struct VS_DATA
{
    float4 Pos	: POSITION;
    float4 T0	: TEXCOORD0;
};

VS_DATA main( VS_DATA In )
{
    return In;
}

#endif // PS3
