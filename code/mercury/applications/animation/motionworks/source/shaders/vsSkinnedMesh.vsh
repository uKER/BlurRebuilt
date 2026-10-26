#include "vsConstants.h"
#include "vsCommon.h"

float4x4 g_ViewProjMatrix	: register(CCV_DISPLAY_VIEWPROJ_MATRIX);
float4x4 g_WorldMatrix		: register(CCV_DISPLAY_WORLD_MATRIX);

#ifdef GPU_SKINNING
float4x3 g_SkinningMatrices[52]	: register(CCV_CHARACTER_SKINNING_MATRIX_0);
#endif

float3   g_ViewPosition		: register(CCV_DISPLAY_VIEW_POSITION);

struct VS_INPUT
{
	float4	Position		: POSITION;
#ifdef GPU_SKINNING
	float4	BoneWeights		: BLENDWEIGHT;
	int4	BoneIndices		: BLENDINDICES;
#endif
	float4	Normal			: NORMAL0;
	float4	Tangent			: TANGENT0;
	float4	Binormal		: BINORMAL0;
	float2	TexCoord		: TEXCOORD0;
	
};

struct VS_OUTPUT
{
	float4 Position			: POSITION;
	float4 WorldNormal		: TEXCOORD0;
#ifdef ENABLE_NORMAL_MAP
	float3 WorldTangent		: TEXCOORD1;
	float3 WorldBinormal	: TEXCOORD2;
#endif
	float4 WorldViewDir		: TEXCOORD3;
	float2 TexCoord			: TEXCOORD4;
	float4 Color0			: COLOR0;
};

VS_OUTPUT main(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;
	
#ifdef GPU_SKINNING

	//
	// GPU skinning: transform vertices using matrix palette.
	//

	float3 poseNormal = IN.Normal.xyz;

	// Transform from base pose into skinned pose.
	float4x3 m = IN.BoneWeights.x * g_SkinningMatrices[IN.BoneIndices.x]
				+ IN.BoneWeights.y * g_SkinningMatrices[IN.BoneIndices.y]
				+ IN.BoneWeights.z * g_SkinningMatrices[IN.BoneIndices.z]
				+ IN.BoneWeights.w * g_SkinningMatrices[IN.BoneIndices.w];
	
	float4 skinnedPosition = float4(mul(float4(IN.Position.xyz, 1.f), m), 1.f);
	float4 worldPosition = mul(skinnedPosition, g_WorldMatrix);
	
	float4 skinnedNormal = float4(mul(float4(poseNormal, 0.f), m), 0.f);
	float4 worldNormal   = mul(skinnedNormal, g_WorldMatrix);

	// matrix * vector = 4 4D dot products
	// matrix * matrix = 16 4D dot products

#else

	//
	// CPU skinning: vertices and normals in model space.
	//
	
	float4x4 worldViewProjMatrix = mul(g_WorldMatrix, g_ViewProjMatrix);

	float4 modelPos = float4(IN.Position.xyz, 1.f);
	float4 modelNormal = float4( normalize(IN.Normal.xyz), 0.f );

	OUT.Position = mul(modelPos, worldViewProjMatrix);
	OUT.WorldNormal = mul(modelNormal, g_WorldMatrix);

	#ifdef ENABLE_NORMAL_MAP
	float4 modelTangent = float4( normalize(IN.Tangent.xyz), 0.f );
	float4 modelBinormal = float4( normalize(IN.Binormal.xyz), 0.f );
	OUT.WorldTangent = mul(modelTangent, g_WorldMatrix);
	OUT.WorldBinormal = mul(modelBinormal, g_WorldMatrix);
	#endif
	
#endif
	OUT.TexCoord = IN.TexCoord;
	
	return OUT;
}