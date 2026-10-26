#ifndef RENDER_STATE_FOG_H
#define RENDER_STATE_FOG_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Render
{

// encapsulates fog parameters

// types of depth fog
enum E_DepthFog
{
	E_DepthFog_Disable,
	E_DepthFog_ZDepth,		// depth is the z component
	E_DepthFog_Euclidean	// depth is euclidean distance
};

// height fog either on or off
enum E_HeightFog
{
	E_HeightFog_Disable,
	E_HeightFog_Enable
};

// how the fog is computed
enum E_FogFalloff
{
	E_FogFalloff_Linear,
	E_FogFalloff_Exp,
	E_FogFalloff_ExpSq
};

class C_Fog
{
public:
	C_Fog();
	~C_Fog();

	void	SetColour(const Maths::C_RGBA_F32& colour);

	void	SetDepth(E_DepthFog eDepth, E_FogFalloff eFalloff, F32 fDistance, F32 fNearDistance);
	
	void	SetDepth(E_DepthFog eDepth);
	void	SetDepthFalloff(E_FogFalloff eDepthFalloff);
	void	SetDepthDensity(F32 fDensity);
	void	SetDepthNearDistance(F32 fNearDistance);

	void	SetHeight(E_HeightFog eHeight, E_FogFalloff eFalloff, F32 fPosition, F32 fDistance, F32 fPositionFalloff);
	
	void	SetHeight(E_HeightFog eHeight);
	void	SetHeightFalloff(E_FogFalloff eHeightFalloff);
	void	SetHeightPosition(F32 fPosition);
	void	SetHeightDensity(F32 fDensity);
	void	SetHeightPositionFalloff(F32 fPositionFalloff);

	MathsStorage::Float4		GetFogColour() const;
	MathsStorage::Float4		GetDepthFogShaderParameters() const;
	MathsStorage::Float4		GetHeightFogShaderParameters() const;

	E_DepthFog					DepthFog() const;
	E_HeightFog					HeightFog() const;

	friend class C_RenderState;

private:
	void						Update();
	// shared
	Maths::C_RGBA_F32			m_Colour;
	
	// depth fog
	E_DepthFog					m_eDepth;
	E_FogFalloff				m_eDepthFalloff;
	F32							m_fDepthDensity;
	F32							m_fDepthNearDistance;
	
	// height fog
	E_HeightFog					m_eHeight;
	E_FogFalloff				m_eHeightFalloff;
	F32							m_fHeightDensity;
	F32							m_fHeightPosition;
	F32							m_fHeightPositionFalloff;
};

// depth fog

INLINE void	C_Fog::SetDepth(E_DepthFog eDepth, E_FogFalloff eFalloff, F32 fDensity, F32 fNearDistance)
{
	SetDepth(eDepth);
	SetDepthFalloff(eFalloff);
	SetDepthDensity(fDensity);
	SetDepthNearDistance(fNearDistance);
}	

INLINE void C_Fog::SetDepth(E_DepthFog eDepth)
{
	 m_eDepth = eDepth;
}

INLINE E_DepthFog C_Fog::DepthFog() const
{
	return m_eDepth;
}

INLINE void	C_Fog::SetDepthFalloff(E_FogFalloff eFalloff)
{
	m_eDepthFalloff = eFalloff;
}

INLINE void	C_Fog::SetColour(const Maths::C_RGBA_F32& colour)
{
	m_Colour = colour;
}

INLINE void	C_Fog::SetDepthDensity(F32 fDensity)
{
	if(fDensity < 0.001f)
		fDensity = 0.001f;
	
	m_fDepthDensity = fDensity;
}

INLINE void	C_Fog::SetDepthNearDistance(F32 fNearDistance)
{
	m_fDepthNearDistance = fNearDistance;
}

// height fog

INLINE void	C_Fog::SetHeight(E_HeightFog eHeight, E_FogFalloff eFalloff, F32 fPosition, F32 fDensity, F32 fPositionFalloff)
{
	SetHeight(eHeight);
	SetHeightFalloff(eFalloff);
	SetHeightPosition(fPosition);
	SetHeightDensity(fDensity);
	SetHeightPositionFalloff(fPositionFalloff);
}

INLINE void C_Fog::SetHeight(E_HeightFog eHeight)
{
	 m_eHeight = eHeight;
}

INLINE E_HeightFog C_Fog::HeightFog() const
{
	return m_eHeight;
}

INLINE void	C_Fog::SetHeightFalloff(E_FogFalloff eFalloff)
{
	m_eHeightFalloff = eFalloff;
}

INLINE void	C_Fog::SetHeightPosition(F32 fHeightPosition)
{
	m_fHeightPosition = fHeightPosition;
}

INLINE void	C_Fog::SetHeightDensity(F32 fDensity)
{
	if(fDensity < 0.001f)
		fDensity = 0.001f;

	m_fHeightDensity = fDensity;
}



INLINE void	C_Fog::SetHeightPositionFalloff(F32 fHeightPositionFalloff)
{
	m_fHeightPositionFalloff = fHeightPositionFalloff;
}

}

#endif // RENDER_STATE_FOG_H
