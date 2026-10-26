#include "stdafx.h"

#include "Lux/Lights/Light.h"
#include "Lux/LightingModelProperties.h"
#include "Lux/GlobalIllumination/GlobalIlluminationProperties.h"
#include "lux_common/StringManipulation.h"

using namespace LUX;
using namespace XERCES_CPP_NAMESPACE;

IMPLEMENT_PERSISTENCE(Light, "Light");

Light::Light()
{ 
	Initialize();
}

Light::~Light()
{
}

Light::Light(const Aftk::C_SceneNode *pAftkLight, Aftk::Matrix44 &mat)
{
	Initialize();

	// get name
	SetName(pAftkLight->GetName());

	// get position
	HMat44 worldMatrix = LPVS::ConvertMatrix(mat);
	HVec4 vPos = worldMatrix[3];
	SetPosition( float3(vPos.x, vPos.y, vPos.z) );
}

void Light::Initialize()
{
	m_type = LUX_LIGHT_TYPE_UNKNOWN;
	m_name = "";
	m_channel = "";
	m_vPos = float3(0,0,0);
	m_vEmittance = float3(1,1,1);
	m_nSamples = -1;

	m_fIntensity = 1.0f;
	m_vColour = float3(1,1,1);

	m_pGIProp = NULL;
}

void Light::SetXMLParameter(char *pName, char *pValue)
{
	bool bUpdateMatrix = false;
	std::stringstream s;

	if (stricmp(pName, "Name") == 0)				// get name
	{		
		m_name = pValue;
	}
	else if (stricmp(pName, "Position") == 0)		// get position
	{
		Position3 position;
		StringUtility::StringToVector(pValue, position);
		m_vPos = position;
		bUpdateMatrix = true;
	}
	else if (stricmp(pName, "Colour") == 0)		// get colour
	{
		Colour4 col;
		StringUtility::StringToVector(pValue, col);
		SetColour(float3(col[0],col[1],col[2]));
	}
	else if (stricmp(pName, "Intensity") == 0)		// get intensity multiplier
	{
		float fIntensity;
		s << pValue;
		s >> fIntensity;
		SetIntensity(fIntensity);
	}
	else if (stricmp(pName, "samples") == 0)		// get number of samples
	{
		s << pValue;
		s >> m_nSamples;
	}
	else if (stricmp(pName, "channel") == 0)		// get light channel	
	{		
		s << pValue;
		s >> m_channel;
	}
}

void Light::SetName(const std::string &name)
{
	m_name = name;
}

void Light::SetPosition(const float3& position)
{
	m_vPos = position; 
}

const std::string& Light::GetName() const
{
	return m_name;
}

LuxLightTypeEnum Light::GetType() const
{
	return m_type;
}

// by default return an infinitely big bounding box
box3f Light::GetBBox(const LUX::SpatialHierarchy *pScene) const
{
	box3f out;

	return out;
}

Spectrum Light::Sample(const float3 &vPos, int iSample, float3 *wi, float *pdf, Visibility *visibility) const
{
	// an assert here may mean that a class inherited from 'Light' needs to do DECLARE_PERSISTENCE

	// (could also mean that sampling without normals hasn't been programmed yet for this type of light)
	assert(0); return float3(0,0,0);
}

Spectrum Light::Sample(const float3 &vPos, const float3 &vNormal, int iSample, float3 *wi, float *pdf, Visibility *visibility) const
{
	return Sample(vPos, iSample, wi, pdf, visibility);
}

//
void Light::write(ost::Engine &ar)
{
	ar << m_vEmittance << m_nSamples << m_iChannel;
}
void Light::read(ost::Engine &ar)
{
	ar >> m_vEmittance >> m_nSamples >> m_iChannel;
}

// apply inverse gamma correction to the light intensity
void Light::ApplyGammaSettingsToIntensity(GlobalIlluminationProperties *pGIProp)
{
	m_pGIProp = pGIProp;
	RecalculateEmittance();
}

void Light::SetColour(float3 vColour)
{
	m_vColour = vColour;
	RecalculateEmittance();
}

void Light::SetIntensity(float fIntensity)
{
	m_fIntensity = fIntensity;
	RecalculateEmittance();
}

// recalculate emittance from intensity, colour and gamma settings
void Light::RecalculateEmittance()
{
	float3 vColour = m_vColour;

	if (m_pGIProp)
	{
		float fGamma = m_pGIProp->InputGamma();

		// apply gamma correction
		if (fGamma != 1.0f)
		{
			assert(fGamma != 0);

			if (vColour.x > 0)
				vColour.x = powf(vColour.x, fGamma);
			if (vColour.y > 0)
				vColour.y = powf(vColour.y, fGamma);
			if (vColour.z > 0)
				vColour.z = powf(vColour.z, fGamma);
		}
	}
    
	m_vEmittance = vColour * m_fIntensity;
}
