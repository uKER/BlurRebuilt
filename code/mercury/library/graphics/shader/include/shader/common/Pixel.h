#ifndef SHADER_PIXEL_H
#define SHADER_PIXEL_H

C_PixelOutput MainPS(in C_VertexOutput vtxData) 
{ 
    C_PixelOutput output = (C_PixelOutput) 0;

	C_Material	mtlData		= EvaluateMaterial(vtxData);
	C_Surface	surfaceData	= EvaluateSurface(mtlData, vtxData);
	C_Lighting	lightData	= EvaluateLighting(mtlData, surfaceData, vtxData);

	real3	vDiffuse		= EvaluateDiffuse(mtlData, surfaceData, lightData, vtxData);
	real3	vSpecular		= EvaluateSpecular(mtlData, surfaceData, lightData, vtxData);

	output.m_vColour.xyz	= EvaluateColour(vDiffuse, vSpecular, mtlData, surfaceData, lightData);
	output.m_vColour.w		= EvaluateAlpha(mtlData);

	output.m_vColour		= EvaluateFog(output.m_vColour, surfaceData);
	output.m_vColour		= EvaluateFinalColour(output.m_vColour);
#if !defined(ALPHA_TO_COVERAGE)
	output.m_vColour.xyz	*= output.m_vColour.w;
#endif

	output.m_vColour.a		*= vtxData.m_vNormal.a;

#ifndef CFG_SPEED
#if !defined(CFG_CONVERTER)

#endif // MAYA || XSI
#endif // CFG_SPEED

	//output.m_vColour.rgb = saturate( lightData.m_vDirection.xyz * 0.5f + 0.5f );

    return output;
}

#endif // SHADER_PIXEL_H
