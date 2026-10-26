#ifndef AMAX_SHADERS_COMMON_TRANSFORM_H
#define AMAX_SHADERS_COMMON_TRANSFORM_H

#if defined(ENABLE_SKINNING)
float3x4	g_mPalette[MAX_NUMBER_SKINNING_TRANSFORMS] : register(c96) < string source = "<MatrixPalette>"; bool Uninitialised = true; int Stream = 1; >;
#endif

#if defined(CFG_CONVERTER)

#define GET_INPUT_NORMAL(X) (X.m_vNormal.xyz)
#define GET_INPUT_TANGENT(X) (X.m_vTangent.xyz * -1.0f)
#define GET_INPUT_BINORMAL(X) (X.m_vBinormal.xyz)

#else // CFG_CONVERTER

#if defined(PLATFORM_PC)
#define GET_INPUT_NORMAL(X) (X.m_vNormal.xyz * 2.0f - 1.0f)
#define GET_INPUT_TANGENT(X) (X.m_vTangent.xyz * 2.0f - 1.0f)
#define GET_INPUT_BINORMAL(X) (X.m_vBinormal.xyz * 2.0f - 1.0f)
#else
#define GET_INPUT_NORMAL(X) (X.m_vNormal.xyz)
#define GET_INPUT_TANGENT(X) (X.m_vTangent.xyz * 2.0f - 1.0f)
#define GET_INPUT_BINORMAL(X) (X.m_vBinormal.xyz)
#endif

#endif // CFG_CONVERTER

#ifdef ENABLE_LIGHTMAP
#if !defined(CFG_CONVERTER)
shared float2			g_vUVOffset		: register(CCV_LIGHTING_UV_OFFSET)	< string Source = "UVoffset"; string InstanceMode = "InstanceOnly"; >;
#else
static float2			g_vUVOffset		= { 0.0f, 0.0f };
#endif
#endif

struct C_VertexData
{
	float4	m_vPosition;
	float3	m_vNormal;
#if defined(ENABLE_TANGENTNORMAL)
	float3	m_vTangent;
	float3	m_vBinormal;
#endif
};

#if !defined(REPLACE_TRANSFORM_BINDPOSITION)
C_VertexData EvaluateBindPosition(in C_VertexData vtxData, in C_VertexInput input)
{
	vtxData.m_vPosition	= float4(input.m_vPosition.xyz, 1.0f);
	vtxData.m_vNormal	= GET_INPUT_NORMAL(input);
#if defined(ENABLE_TANGENTNORMAL)
	vtxData.m_vTangent	= GET_INPUT_TANGENT(input);
#endif
	return vtxData;
}
#endif

#if !defined(REPLACE_TRANSFORM_OBJECTPOSITION)
C_VertexData EvaluateObjectPosition(in C_VertexData vtxData, in C_VertexInput input)
{
#if defined(ENABLE_SKINNING)
	float4	vObjectPosition	= float4(0.0f, 0.0f, 0.0f, 1.0f);
	float3	vObjectNormal	= 0.0f;
#if defined(ENABLE_TANGENTNORMAL)
	float3	vObjectTangent	= 0.0f;
#endif

	float3x4 mBoneTransform = (float3x4)0;

#ifndef LOD
	float fWeightSum = 1.f;

	for (int i = 1; i < 4; ++i)
	{
		mBoneTransform += ((input.m_vBlendWeights[i] > 0.f) ? (g_mPalette[input.m_vBlendIndices[i]] * input.m_vBlendWeights[i]) : (float3x4)0);
		fWeightSum -= input.m_vBlendWeights[i];
	}

	mBoneTransform += (g_mPalette[input.m_vBlendIndices[0]] * fWeightSum);
#else
	mBoneTransform = g_mPalette[input.m_vBlendIndices.x];
#endif

	float3x3 mBoneNormalTransform = (float3x3)mBoneTransform;

	vObjectPosition.xyz = mul(mBoneTransform, vtxData.m_vPosition).xyz;
	vObjectNormal.xyz	= mul(mBoneNormalTransform, vtxData.m_vNormal).xyz;

#if defined(ENABLE_TANGENTNORMAL)
	vObjectTangent.xyz	= mul(mBoneNormalTransform, vtxData.m_vTangent).xyz;
#endif

	vtxData.m_vPosition	= vObjectPosition;
	vtxData.m_vNormal	= vObjectNormal;
#if defined(ENABLE_TANGENTNORMAL)
	vtxData.m_vTangent	= vObjectTangent;
#endif
#elif defined(ENABLE_WIND) && !defined(LOD)
	vtxData.m_vPosition	= lerp(vtxData.m_vPosition, mul(vtxData.m_vPosition, g_mWind[input.m_vTexCoord.z]), input.m_vTexCoord.w);
#endif
	return vtxData;
}
#endif

#if !defined(REPLACE_TRANSFORM_WORLDPOSITION)
C_VertexData EvaluateWorldPosition(in C_VertexData vtxData, in C_VertexInput input)
{
	vtxData.m_vPosition	= mul(vtxData.m_vPosition, g_mWorld);
	vtxData.m_vNormal	= mul(vtxData.m_vNormal, (float3x3) g_mWorld);
#if defined(ENABLE_TANGENTNORMAL)
	vtxData.m_vTangent	= mul(vtxData.m_vTangent, (float3x3) g_mWorld);
	vtxData.m_vBinormal	= cross(vtxData.m_vNormal, vtxData.m_vTangent) * (2.0f * input.m_vTangent.w - 1.0f);
#endif
#if defined(ENABLE_PREPASS) && defined(ENABLE_PREPASS_VIEWSPACE)
	vtxData.m_vNormal	= mul(vtxData.m_vNormal, (float3x3) g_mView);
#if defined(ENABLE_TANGENTNORMAL)
	vtxData.m_vTangent	= mul(vtxData.m_vTangent, (float3x3) g_mView);
	vtxData.m_vBinormal	= cross(vtxData.m_vNormal, vtxData.m_vTangent) * (2.0f * input.m_vTangent.w - 1.0f);
#endif
#endif
	return vtxData;
}
#endif

#if !defined(REPLACE_TRANSFORM_POSITION)
C_VertexOutput EvaluatePosition(in C_VertexOutput output, in C_VertexData vtxData)
{
	output.m_vPosition		= mul(vtxData.m_vPosition, g_mViewProj); 
#if defined(ENABLE_NORMAL)
	output.m_vNormal.xyz	= vtxData.m_vNormal.xyz;
#endif
#if defined(ENABLE_TANGENTNORMAL)
	output.m_vTangent		= vtxData.m_vTangent;
	output.m_vBinormal		= vtxData.m_vBinormal;
#endif
#if defined(ENABLE_VIEW)
	output.m_vView.xyz		= vtxData.m_vPosition.xyz - g_vViewPosition.xyz;
#endif
#if defined(ENABLE_WORLD)
	output.m_vWorld.xyz		= vtxData.m_vPosition.xyz;
#endif
#if !defined(LOD)
	output.m_vScreenPos	= float4(output.m_vPosition.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f) * output.m_vPosition.w, output.m_vPosition.w, 0.0f);
#endif
#if defined(ENABLE_TANGENTVIEW)
	float3 vWorldView = vtxData.m_vPosition.xyz - g_vViewPosition.xyz;
	output.m_vTangentView.xyz = vWorldView.x * float3(vtxData.m_vTangent.x, vtxData.m_vBinormal.x, vtxData.m_vNormal.x)
							  + vWorldView.y * float3(vtxData.m_vTangent.y, vtxData.m_vBinormal.y, vtxData.m_vNormal.y)
							  + vWorldView.z * float3(vtxData.m_vTangent.z, vtxData.m_vBinormal.z, vtxData.m_vNormal.z);
#endif
	return output;
}
#endif

C_VertexOutput EvaluateTexCoord(in C_VertexOutput output, in C_VertexInput input)
{
	output.m_vTexCoord.xy	= input.m_vTexCoord.xy;
#if defined(ENABLE_LIGHTMAP)
	output.m_vTexCoord.zw	= input.m_vTexCoord.zw + g_vUVOffset;
#endif
	return output;
}

#ifndef SHNODE_IN_VERTEX_CONSTANT
C_VertexOutput EvaluateColour(in C_VertexOutput output, in C_VertexInput input, in C_VertexData vtxData)
{
#if defined(ENABLE_VERTEXCOLOURS)
	output.m_vColour	= input.m_vColour.rgba;
#endif
	return output;
}
#endif

#endif // AMAX_SHADERS_COMMON_TRANSFORM_H
