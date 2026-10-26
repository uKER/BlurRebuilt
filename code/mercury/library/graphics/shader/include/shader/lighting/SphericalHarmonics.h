#ifndef SHADER_LIGHTING_SPHERICAL_HARMONICS_H
#define SHADER_LIGHTING_SPHERICAL_HARMONICS_H

void RGBSH(in C_SHNode shNode, in real3 vWorldNormal, inout real3 vColour)
{
	vColour.r *= dot(real4(vWorldNormal, 1.0h), shNode.m_vRedCoeffs);
	vColour.g *= dot(real4(vWorldNormal, 1.0h), shNode.m_vGreenCoeffs);
	vColour.b *= dot(real4(vWorldNormal, 1.0h), shNode.m_vBlueCoeffs);
}

void SunSkySH(in C_SHNode shNode, in real3 vWorldNormal, inout real fSunOcclusion, inout real fSkyOcclusion)
{
	fSkyOcclusion *= dot(real4(vWorldNormal, 1.0h), shNode.m_vGreenCoeffs);
	fSunOcclusion *= dot(real4(vWorldNormal, 1.0h), shNode.m_vBlueCoeffs);
}

void BasisSH(in C_SHNode shNode, in real3 vWorldNormal, inout real3 vColour, inout real fSunOcclusion, inout real fSkyOcclusion)
{
	vColour.r *= dot(real4(vWorldNormal, 1.0h), shNode.m_vRedCoeffs);
	vColour.g *= dot(real4(vWorldNormal, 1.0h), shNode.m_vGreenCoeffs);
	vColour.b *= dot(real4(vWorldNormal, 1.0h), shNode.m_vBlueCoeffs);
#ifndef SH_NO_CHANNEL_COEFFS
	fSunOcclusion *= shNode.m_vChannelCoeffs.x;
	fSkyOcclusion *= shNode.m_vChannelCoeffs.y;
#endif
}

void DirectionalSH(in C_SHNode shNode, inout real3 vDirection)
{
	real3 vSHDirection = (shNode.m_vRedCoeffs.xyz + shNode.m_vGreenCoeffs.xyz + shNode.m_vBlueCoeffs.xyz + float3( 0.0f, 0.01f, 0.0f )) / 3.0f; // NOTE: small upward bias to avoid NAN
	vDirection = normalize(vDirection + vSHDirection);
}

#endif // SHADER_LIGHTING_SPHERICAL_HARMONICS_H
