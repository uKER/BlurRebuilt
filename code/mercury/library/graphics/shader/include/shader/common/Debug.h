#ifndef SHADER_COMMON_DEBUG_H
#define SHADER_COMMON_DEBUG_H

float4 DebugColour(in float4 vColour, in C_Material mtlData, in C_VertexOutput vtxData)
{
#if !defined(CFG_SPEED)
#if !defined(CFG_CONVERTER)
	if (length(g_vDebugColour) > 0.0)
		vColour = g_vDebugColour;

#endif // MAYA || XSI
#endif // CFG_SPEED

	return vColour;
}

#endif // SHADER_COMMON_DEBUG_H
