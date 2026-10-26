#ifndef RENDER_LIGHT_LIGHT_INL
#define RENDER_LIGHT_LIGHT_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Render
{
namespace Light
{

// C_Light

INLINE const E_LightType C_Light::GetType() const
{
	return m_Type;
}

INLINE const bool C_Light::GetEnabled() const
{
	return m_Enabled;
}

INLINE const F32 C_Light::GetRadius() const
{
	return m_Radius;
}

INLINE const F32 C_Light::GetIntensity() const
{
	return m_Intensity;
}

INLINE const Maths::HVec3 C_Light::GetColour() const
{
	return m_Colour;
}

INLINE const Maths::HVec3& C_Light::GetPosition() const
{
	return m_Position;
}

INLINE const Maths::HVec3& C_Light::GetDirection() const
{
	return m_Direction;
}

INLINE void C_Light::SetType(const E_LightType type)
{
	m_Type = type;
}

INLINE void C_Light::SetEnabled(const bool enabled)
{
	m_Enabled = enabled;
}

INLINE void C_Light::SetRadius(const F32 radius)
{
	m_Radius = radius;
}

INLINE void C_Light::SetIntensity(const F32 intensity)
{
	m_Intensity = intensity;
}

INLINE void C_Light::SetColour(const Maths::HVec3 PCREF colour)
{
	m_Colour = colour;
}

INLINE void C_Light::SetPosition(const Maths::HVec3 PCREF position)
{
	m_Position = position;
}

INLINE void C_Light::SetDirection(const Maths::HVec3 PCREF direction)
{
	m_Direction = direction;
}

// C_LightManager

INLINE const S32 C_LightManager::GetLightCount() const
{
	return m_LightList.Size();
}

INLINE C_Light& C_LightManager::GetLight(const S32 index)
{
	assert(index >= 0 && index < m_LightList.Size());
	return m_LightList[index];
}

} // namespace Light
} // namespace Render

#endif // RENDER_LIGHT_LIGHT_INL
