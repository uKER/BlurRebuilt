#pragma once

namespace LUX
{

struct VirtualPointLight
{
	VirtualPointLight() {};
	VirtualPointLight(Float3 ivPos, Float3 ivNormal, Float3 iluminance) : 
							vPos(ivPos), luminance(iluminance), vNormal(ivNormal) {};

	Box3		GetBBox(float fScale);

	Float3		vPos;
	Float3		vNormal;
	Float3		luminance;
	float		m_fMaxRadius;

#ifdef _INTEGRATOR_H_
	friend ost::Engine& operator << (ost::Engine& ar, VirtualPointLight &v) { ar << v.luminance << v.vNormal << v.vPos << v.m_fMaxRadius; return ar; }
	friend ost::Engine& operator >> (ost::Engine& ar, VirtualPointLight &v) { ar >> v.luminance >> v.vNormal >> v.vPos >> v.m_fMaxRadius; return ar; }
#endif
};

}
