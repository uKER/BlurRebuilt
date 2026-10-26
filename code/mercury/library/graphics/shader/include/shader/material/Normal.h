#ifndef SHADER_MATERIAL_NORMAL_H
#define SHADER_MATERIAL_NORMAL_H

real3 EvaluateTangentNormal(sampler NormalSampler, float2 vTexCoord)
{
#if 1
	return normalize(h4tex2D(NormalSampler, vTexCoord).rgb - 0.5h);
#else
	real3 vSample = h4tex2D(NormalSampler, vTexCoord).rgb;

	real3 vTangentNormal;
	vTangentNormal.xy = vSample.xy * 2.0f - 1.0f;
	vTangentNormal.z = sqrt(1.0f - dot(vTangentNormal.xy, vTangentNormal.xy));

	return vTangentNormal;
#endif
}

real3 EvaluateWorldNormal(real3 vTangentNormal, real3 vNormal, real3 vTangent, real3 vBinormal)
{
	return normalize(vTangentNormal.z * vNormal + vTangentNormal.x * vTangent + vTangentNormal.y * vBinormal);
}

#endif // SHADER_MATERIAL_NORMAL_H
