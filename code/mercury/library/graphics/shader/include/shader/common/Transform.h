#ifndef SHADER_TRANSFORM_H
#define SHADER_TRANSFORM_H

#if defined(ENABLE_SKINNING)
float3x4	g_mPalette[MAX_NUMBER_SKINNING_TRANSFORMS] : register(c96) < string source = "<MatrixPalette>"; bool Uninitialised = true; int Stream = 1; >;
#endif

#if defined(ENABLE_LIGHTMAP)
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

C_VertexData EvaluateBindPosition(C_VertexData vtxData, C_VertexInput input)
{
	vtxData.m_vPosition	= float4(input.m_vPosition.xyz, 1.0f);
#if defined(PLATFORM_PC)
	vtxData.m_vNormal		= input.m_vNormal * 2.0f - 1.0f;
	#if defined(ENABLE_TANGENTNORMAL)
		vtxData.m_vTangent	= input.m_vTangent * 2.0f - 1.0f;
		vtxData.m_vBinormal	= input.m_vBinormal * 2.0f - 1.0f;
	#endif
#else
	vtxData.m_vNormal		= input.m_vNormal;
	#if defined(ENABLE_TANGENTNORMAL)
		vtxData.m_vTangent	= input.m_vTangent;
		vtxData.m_vBinormal	= input.m_vBinormal;
	#endif
#endif
	return vtxData;
}

C_VertexData EvaluateObjectPosition(C_VertexData vtxData, C_VertexInput input)
{
#if defined(ENABLE_SKINNING)
	float4	vObjectPosition	= float4(0.0f, 0.0f, 0.0f, 1.0f);
	float3	vObjectNormal	= 0.0f;
#if defined(ENABLE_TANGENTNORMAL)
	float3	vObjectTangent	= 0.0f;
	float3	vObjectBinormal	= 0.0f;
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

	vObjectPosition.xyz = mul(mBoneTransform, float4(vtxData.m_vPosition.xyz, 1.f)).xyz;
	vObjectNormal.xyz	= mul(mBoneNormalTransform, vtxData.m_vNormal).xyz;

#if defined(ENABLE_TANGENTNORMAL)
	vObjectTangent.xyz	= mul(mBoneNormalTransform, vtxData.m_vTangent).xyz;
	vObjectBinormal.xyz	= mul(mBoneNormalTransform, vtxData.m_vBinormal).xyz;
#endif

	vtxData.m_vPosition	= vObjectPosition;
	vtxData.m_vNormal	= vObjectNormal;
#if defined(ENABLE_TANGENTNORMAL)
	vtxData.m_vTangent	= vObjectTangent;
	vtxData.m_vBinormal	= vObjectBinormal;
#endif
#endif
	return vtxData;
}

C_VertexData EvaluateWorldPosition(C_VertexData vtxData, C_VertexInput input)
{
	vtxData.m_vPosition	= mul(vtxData.m_vPosition, g_mWorld);
	vtxData.m_vNormal	= mul(vtxData.m_vNormal, (float3x3) g_mWorld);
#if defined(ENABLE_TANGENTNORMAL)
	vtxData.m_vTangent	= mul(vtxData.m_vTangent, (float3x3) g_mWorld);
	vtxData.m_vBinormal	= mul(vtxData.m_vBinormal, (float3x3) g_mWorld);
#endif
	return vtxData;
}

C_VertexOutput EvaluatePosition(C_VertexOutput output, C_VertexData vtxData)
{
	output.m_vPosition	= mul(vtxData.m_vPosition, g_mViewProj); 
	output.m_vNormal.xyz= vtxData.m_vNormal;
#if defined(ENABLE_TANGENTNORMAL)
	output.m_vTangent	= vtxData.m_vTangent;
	output.m_vBinormal	= vtxData.m_vBinormal;
#endif
#if defined(ENABLE_PROJPOS)
	output.m_vProjPos	= output.m_vPosition;
#endif
#if defined(ENABLE_WORLDPOS)
	output.m_vWorldPos	= vtxData.m_vPosition;
#endif
#if defined(ENABLE_VIEWVECTOR)
	output.m_vView.xyz	= vtxData.m_vPosition.xyz - g_vViewPosition.xyz;
#endif
#if defined(ENABLE_TANGENTVIEW)
	float3 vWorldView = vtxData.m_vPosition.xyz - g_vViewPosition.xyz;
	output.m_vTangentView.xyz = vWorldView.x * float3(vtxData.m_vTangent.x, vtxData.m_vBinormal.x, vtxData.m_vNormal.x)
		+ vWorldView.y * float3(vtxData.m_vTangent.y, vtxData.m_vBinormal.y, vtxData.m_vNormal.y)
		+ vWorldView.z * float3(vtxData.m_vTangent.z, vtxData.m_vBinormal.z, vtxData.m_vNormal.z);
#endif
	return output;
}

C_VertexOutput EvaluateTexCoord(C_VertexOutput output, C_VertexInput input)
{
#if defined(ENABLE_LIGHTMAP)
	output.m_vTexCoord	= input.m_vTexCoord + float4(0.0f, 0.0f, g_vUVOffset);
#else
	output.m_vTexCoord	= input.m_vTexCoord;
#endif
	return output;
}

C_VertexOutput EvaluateColour(C_VertexOutput output, C_VertexInput input)
{
#if defined(ENABLE_VERTEXCOLOURS)
	output.m_vColour	= input.m_vColour;
#endif
	return output;
}

#endif // SHADER_TRANSFORM_H
