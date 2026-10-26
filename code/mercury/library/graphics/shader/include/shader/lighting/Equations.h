#ifndef SHADER_LIGHTING_EQUATIONS_H
#define SHADER_LIGHTING_EQUATIONS_H

real Diffuse(real3 vLight, real3 vNormal)
{
	return saturate(dot(vNormal, vLight));
}

real WrappedDiffuse(real3 vLight, real3 vNormal, real fBias)
{
	return saturate(dot(vNormal, vLight) * (1.0h - fBias) + fBias);
}

real RimLight(real3 vRim, real3 vNormal, real3 vView)
{
	real3	vUp		= real3(0.0h, 1.0h, 0.0h);
	real3	vRight	= cross(vView, vUp);
	real3	vLight	= normalize(vRim.z * vView + vRim.y * vUp + vRim.x * vRight);
	return saturate(dot(vNormal, vLight));
}

real Blinn(real3 vLight, real3 vNormal, real3 vView, real fPower)
{
	real3	vHalf		= normalize(vLight - vView);
	real	fNDotH		= saturate(dot(vNormal, vHalf));
	return pow(fNDotH, fPower);
}

real Phong(real3 vLight, real3 vNormal, real3 vView, real fPower)
{
	real	fNDotV		= dot(vView, vNormal);
	real3	vReflect	= vView - 2.0h * fNDotV * vNormal;
	real	fRDotL		= saturate(dot(vReflect, vLight));
	return pow(fRDotL, fPower);
}

real Fresnel(real fNDotV)
{
	real fRoot = saturate(1.0h + fNDotV);
	return fRoot * fRoot;
}

real Fresnel(real fNDotV, real fFresnelMidBrightness)
{
	real fRoot = saturate(1.0h + (1.0h - fFresnelMidBrightness) * fNDotV);
	return fRoot * fRoot;
}

real Fresnel(real fNDotV, real fBrightness, real fPower)
{
	return fBrightness + (1.0h - fBrightness) * pow(saturate(1.0h + fNDotV), fPower);
}

real Schlick(real fNDotV)
{
	real fDot = saturate(1.0h + fNDotV);
	real fDotSquared = fDot * fDot;
	return fDotSquared * fDotSquared * fDot;
}

real Schlick(real fNDotV, real fFresnelMidBrightness)
{
	real fDot = saturate(1.0h + fNDotV);
	real fDotSquared = fDot * fDot;
	real fDot5 = fDotSquared * fDotSquared * fDot;
	return fFresnelMidBrightness + (1.0h - fFresnelMidBrightness) * fDot5;
}

real Schlick4(real fNDotV)
{
	real fDot = saturate(1.0h + fNDotV);
	real fDotSquared = fDot * fDot;
	return fDotSquared * fDotSquared;
}


real Schlick4(real fNDotV, real fFresnelMidBrightness)
{
	real fDot = saturate(1.0h + fNDotV);
	real fDotSquared = fDot * fDot;
	real fDot4 = fDotSquared * fDotSquared;
	return fFresnelMidBrightness + (1.0h - fFresnelMidBrightness) * fDot4;
}

#endif // SHADER_LIGHTING_EQUATIONS_H
