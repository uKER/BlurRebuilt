#ifndef AMAX_SHADERS_COMMON_VERTEX_H
#define AMAX_SHADERS_COMMON_VERTEX_H
 

C_VertexOutput MainVS(in C_VertexInput input)
{
	C_VertexData vtxData = (C_VertexData) 0;
	vtxData	= EvaluateBindPosition(vtxData, input);
	vtxData	= EvaluateObjectPosition(vtxData, input);
	vtxData	= EvaluateWorldPosition(vtxData, input);

	C_VertexOutput output = (C_VertexOutput) 0;
	output	= EvaluatePosition(output, vtxData);
	output	= EvaluateTexCoord(output, input);
	output	= EvaluateColour(output, input, vtxData);

    return output;   
}

C_VertexPrepassOutput PrepassVS(in C_VertexInput input)
{
	C_VertexData vtxData = (C_VertexData) 0;
	vtxData	= EvaluateBindPosition(vtxData, input);
	vtxData	= EvaluateObjectPosition(vtxData, input);
	vtxData	= EvaluateWorldPosition(vtxData, input);

	C_VertexPrepassOutput output = (C_VertexPrepassOutput) 0;
	output.m_vPosition		= mul(vtxData.m_vPosition, g_mViewProj); 
#if defined(ENABLE_NORMAL)
	output.m_vNormal.xyz	= vtxData.m_vNormal.xyz;
#endif

#if defined(ENABLE_TANGENTNORMAL)
	output.m_vTangent		= vtxData.m_vTangent;
	output.m_vBinormal		= vtxData.m_vBinormal;
#endif

#if !defined(DISABLE_LODFADE)
	output.m_vNormal.a		= EvaluateLodFadeNoSaturate(output.m_vPosition.w);
#endif

	output.m_vTexCoord.xy	= input.m_vTexCoord.xy;
#if defined(ENABLE_PREPASS_VIEWDEPTH)
	output.m_vTexCoord.z	= output.m_vPosition.w / g_vNearFar.w;
#endif
#if defined(ENABLE_PREPASS_DEPTHTEXTURE)
	output.m_vTexCoord.zw = output.m_vPosition.zw;	 
#endif

    return output;
}

#endif // AMAX_SHADERS_COMMON_VERTEX_H
