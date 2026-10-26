#ifndef RENDER_SKY_SKY_DOME_INL
#define RENDER_SKY_SKY_DOME_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Render
{
namespace Sky
{

INLINE const F32 C_SkyDome::GetRotation() const
{
	return m_Rotation;
}

INLINE void C_SkyDome::SetRotation(const F32 rotation)
{
	m_Rotation = rotation;
}

INLINE const F32 C_SkyDome::GetIntensity() const
{
	return m_Intensity;
}

INLINE void C_SkyDome::SetIntensity(const F32 intensity)
{
	m_Intensity = intensity;
}

INLINE const Maths::C_RGBA_F32& C_SkyDome::GetFogColour() const
{
	return m_FogColour;
}

INLINE void C_SkyDome::SetFogColour(const Maths::C_RGBA_F32& fogColour)
{
	m_FogColour = fogColour;
}

} // namespace Sky
} // namespace Render

#endif // RENDER_SKY_SKY_DOME_INL
