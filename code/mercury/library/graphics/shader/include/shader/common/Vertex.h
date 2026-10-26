#ifndef SHADER_VERTEX_H
#define SHADER_VERTEX_H

C_VertexOutput MainVS(C_VertexInput input)
{
	C_VertexData vtxData = (C_VertexData) 0;
	vtxData	= EvaluateBindPosition(vtxData, input);
	vtxData	= EvaluateObjectPosition(vtxData, input);
	vtxData	= EvaluateWorldPosition(vtxData, input);

	C_VertexOutput output = (C_VertexOutput) 0;
	output	= EvaluatePosition(output, vtxData);
	output	= EvaluateTexCoord(output, input);
	output	= EvaluateColour(output, input);

	output.m_vNormal.a = EvaluateLodFadeNoSaturate(output.m_vPosition.w);

    return output;    
}

#endif // SHADER_VERTEX_H
