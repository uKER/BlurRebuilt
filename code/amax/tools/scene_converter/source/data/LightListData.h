#ifndef APP_LIGHT_LIST_DATA_H
#define APP_LIGHT_LIST_DATA_H

#ifdef _MSC_VER
#pragma once
#endif

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
		m_Colour = Maths::C_RGBA_F32(1.f,1.f,1.f,1.f);
		m_LightType = E_LightType_None;
	}
public:
	Maths::HMat43				m_Matrix;
	//RGB->colour and A->intensity
	Maths::C_RGBA_F32			m_Colour;
	E_LightType					m_LightType;

	union
	{
		struct
		{
			F32 m_PointCutOffRadius;
		};

		struct
		{
			F32 m_SpotConeAngle;
			F32 m_SpotPenumbraAngle;
			F32 m_SpotDropOff;
			F32 m_SpotCutOffRadius;
		};
	};

	template< typename Archive > 
	void Serialise( Archive& ar )
	{
		U32 type = m_LightType;
		ar << type;
		ar << m_Matrix;
		ar << Maths::HVec4(m_Colour.R(),m_Colour.G(),m_Colour.B(),m_Colour.A());
	
		switch(m_LightType)
		{
			case E_LightType_None:
			case E_LightType_Hemisphere:
			case E_LightType_Area:
			case E_LightType_Directional:
			break;

			case E_LightType_Point:
				ar << m_PointCutOffRadius;
			break;

			case E_LightType_Spot:
				ar << m_SpotConeAngle;
				ar << m_SpotPenumbraAngle;
				ar << m_SpotDropOff;
				ar << m_SpotCutOffRadius;
			break;
		}
	}
};

typedef Btl::Vector<C_Light> C_LightList;

class C_DataLightList : public Tools::ProcessGraph::C_Data
{
public:
	
	C_DataLightList();
	~C_DataLightList();

	// export
	bool Export(Sys::I_File* pFile, const Tools::E_Platform platform ) const;

	// inherited from C_Data
	const Util::C_String GetName() const;

	const C_LightList& GetLightList()
	{
		return m_LightList;
	};

	const bool GetSkyLight(C_Light& skyLight)
	{
		if (m_HasSkyLight)
		{
			skyLight = m_SkyLight;
			return true;
		}
		else
			return false;
	}
	const bool GetSunLight(C_Light& sunLight)
	{
		if (m_HasSunLight)
		{
			sunLight = m_SunLight;
			return true;
		}
		else
			return false;
	}

	void SetSkyLight(const C_Light& skyLight)
	{
		m_SkyLight = skyLight;
		m_HasSkyLight = true;
	};
	void SetSunLight(const C_Light& sunLight)
	{
		m_SunLight = sunLight;
		m_HasSunLight = true;
	};
	void AddLight(const C_Light& light)
	{
		m_LightList.PushBack(light);
	}

private:
	C_LightList m_LightList;

	bool m_HasSkyLight;
	bool m_HasSunLight;

	C_Light m_SkyLight;
	C_Light m_SunLight;
};

#endif