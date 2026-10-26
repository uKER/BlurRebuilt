#ifndef RENDER_LIGHT_LIGHT_H
#define RENDER_LIGHT_LIGHT_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Render
{
namespace Light
{

//! E_LightType

enum E_LightType
{
	E_LightType_Spot,
	E_LightType_Point,
	E_LightType_Directional,
};

//! C_Light

class C_Light
{
public:

	C_Light();
	~C_Light();

	const E_LightType GetType() const;
	const bool GetEnabled() const;
	const F32 GetRadius() const;
	const F32 GetIntensity() const;
	const Maths::HVec3 GetColour() const;
	const Maths::HVec3& GetPosition() const;
	const Maths::HVec3& GetDirection() const;
	const Maths::HMat43& GetCameraToWorld() const;

	void SetType(const E_LightType type);
	void SetEnabled(const bool enabled);
	void SetRadius(const F32 radius);
	void SetIntensity(const F32 intensity);
	void SetColour(const Maths::HVec3 PCREF colour);
	void SetPosition(const Maths::HVec3 PCREF position);
	void SetDirection(const Maths::HVec3 PCREF direction);

	void Initialise(const Maths::HVec3 PCREF position, const Maths::HVec3 PCREF direction);
	void Initialise(const Maths::HVec3 PCREF position, const F32 radius);
	void Initialise(const Maths::HVec3 PCREF direction);

	void DebugUpdate();
	void DebugRender();

private:

	E_LightType				m_Type;
	bool					m_Enabled;
	F32						m_Radius;
	F32						m_Intensity;
	Maths::HVec3			m_Colour;
	Maths::HVec3			m_Position;
	Maths::HVec3			m_Direction;
};

//! C_LightManager

class C_LightManager
{
public:

	C_LightManager();
	~C_LightManager();

	void Clear();

	const S32 GetLightCount() const;
	C_Light& GetLight(const S32 index);
	
	void SetLightCount(const S32 count, const E_LightType type);
	
	const S32 GetDisabledLightIndex();

	void DebugUpdate();
	void DebugRender();

	static C_LightManager& Instance() { return s_Instance; }

private:

	Ut::Vector<C_Light>		m_LightList;
	static C_LightManager	s_Instance;
};

} // namespace Light
} // namespace Render

#include "Render/Light/Light.inl"

#endif // RENDER_LIGHT_LIGHT_H
