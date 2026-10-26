#ifndef LIGHTLIST_H
#define LIGHTLIST_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/KdTree/KdTree.h>

namespace DbUtil{ class I_DebugRender;};

namespace WorldRender
{

enum E_LightType
{
	E_LightType_None = 0,
	E_LightType_Hemisphere,
	E_LightType_Area,
	E_LightType_Directional,
	E_LightType_Point,
	E_LightType_Spot,
};

// C_Light
class C_Light
{
public:
	C_Light()
	{
		m_Matrix.SetIdentity();
		m_Colour = Maths::HVec4(1.f,1.f,1.f,1.f);
		m_LightType = E_LightType_None;
	}
public:
	Maths::HMat43				m_Matrix;
	//RGB->colour and A->intensity
	Maths::HVec4				m_Colour;
	E_LightType					m_LightType;

	static Maths::HMat43		s_DebugRenderCameraMatrix;

	union
	{
		F32							m_Radius;
		F32							m_PointCutOffRadius;
		struct
		{
			F32	m_SpotCutOffRadius;
			F32 m_SpotConeAngle;
			F32 m_SpotPenumbraAngle;
			F32 m_SpotDropOff;
		};
	};
	//Cached C_AxisBox
	Maths::C_AxisBox	m_AxisBox;

	static void SetDebugRenderCameraMatrix(const Maths::HMat43 PCREF debugRenderCameraMatrix)
	{
		s_DebugRenderCameraMatrix = debugRenderCameraMatrix;
	}

	void DebugRender( DbUtil::I_DebugRender* pDebugRender, const Maths::HMat43 PCREF m, const Maths::HVec3 PCREF vCamera );

	template< typename Archive > 
	void Deserialise( Archive& ar )
	{
		U32 type;
		ar >> type;
		m_LightType = E_LightType(type);
		ar >> m_Matrix;
		ar >> m_Colour;
		ar >> m_Radius;
		switch(m_LightType)
		{
			case E_LightType_None:
			case E_LightType_Hemisphere:
			case E_LightType_Area:
			case E_LightType_Directional:
			case E_LightType_Point:
			break;

			case E_LightType_Spot:
				ar >> m_SpotConeAngle;
				ar >> m_SpotPenumbraAngle;
				ar >> m_SpotDropOff;
			break;
		}

		CalculateAxisBox();
	}
	bool Intersects(const Maths::HVec3 PCREF vPos) const
	{
		return ((vPos - m_Matrix.AW()).Length() < m_Radius) ? true:false;			
	}

	bool Intersects(const Maths::C_AxisBox PCREF v) const
	{
		return v.Intersects(m_AxisBox);
	}
	
	void CalculateAxisBox()
	{
		Maths::HVec3 centre = m_Matrix.AW();

		F32 radius = m_Radius;

		for (int i=0; i<8; i++)
			m_AxisBox.UpdateExtents(Maths::C_Vertex(Maths::HVec3((i&1)?-radius:radius,(i&2)?-radius:radius,(i&4)?-radius:radius) + centre));
	}

	operator const Maths::C_AxisBox () const
	{
		return m_AxisBox;
	}
};


//! C_RenderLevel
class C_LightList
{
public:
	C_LightList();
	~C_LightList();
	
	bool Read( Sys::I_File* pFile );
	void DebugRender( DbUtil::I_DebugRender* pDebugRender, const Maths::HMat43 PCREF m, const Maths::HVec3 PCREF vCamera );

	Maths::HVec3 GetSunDirection() const;
	const Util::KdTree<C_Light>& GetLightList() const { return m_LightList;};
private:

	
	//Lighting information
	bool m_HasSkyLight;
	bool m_HasSunLight;

	C_Light m_SkyLight;
	C_Light m_SunLight;

	Util::KdTree<C_Light> m_LightList;
};


} // namespace WorldRender

#endif // WORLD_RENDER_LEVEL_H

