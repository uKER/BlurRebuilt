/////////////////////////////////////////////////////////////
// Lights
/////////////////////////////////////////////////////////////

#ifndef __VEHICLE_LIGHTS_H__
#define __VEHICLE_LIGHTS_H__

#include <Dynamic/Light/Generic/XtInterface.h>
#include <Dynamic/Light/Manager.h>
#include <Dynamic/Node.h>
#include <SceneGraph/Light/Light.h>

namespace AuxFlares
{
	class C_AuxFlareInstance;
}

namespace Vehicle
{

class C_Node;

/* 
===============================================================
					class C_Headlight

NOTES: 
===============================================================
*/
class C_Headlight
{
public:

	C_Headlight();

	float GetIntensity() { return m_Intensity * m_IntensityModifier * m_IntensityFade; }

public:

	Maths::HMat43	m_WorldMatrix;
	Maths::HVec3	m_Position;
	Maths::C_RGBA_F32	m_Colour;
	float			m_ZOffset;
	float			m_Width;
	float			m_Length;
	float			m_Height;
	float			m_Intensity;
	float			m_IntensityModifier;
	float			m_IntensityFade;
	float			m_Dip;
	bool			m_Enabled;
};

/* 
===============================================================
class C_LightFlares

NOTES: 
===============================================================
*/

class C_Part;

struct C_Flare
{
	DynamicEffect::Light::LightDefinition*	m_pLightDefinition;
	DynamicEffect::Light::LightHandle		m_Handle;
	XtHandle<DynamicEffect::Light::XtInterface::Light> m_XtHandle;
	DynamicEffect::C_Node					m_Node;
	C_Part*									m_pNode;
	Vehicle::C_Node*						m_pDamageNode;
	float									m_RenderOffset;
	bool									m_Inverted;
};

class C_LightFlares
{
public:
	Btl::Vector<C_Flare>	m_LightFlareVector;
};

/* 
===============================================================
class C_DynamicLights

NOTES: 
===============================================================
*/
struct C_DynamicLight
{
	MathsStorage::Float3	m_Colour;
	F32						m_Radius;
	C_Part*		m_pNode;
	Maths::HVec3			m_Offset;
	U32						m_Flags;

	SceneGraph::C_PointLightHandle m_LightHandle;

	C_DynamicLight() {}
};

class C_DynamicLights
{
public:
	Btl::Vector<C_DynamicLight>	m_DynamicLightVector;
};

/* 
===============================================================
class C_AuxFlares

NOTES: 
===============================================================
*/
enum E_AuxFlareColour
{
	E_AuxFlareColour_White,
	E_AuxFlareColour_Red,
};

struct C_AuxFlare
{
	//WorldRender::C_ModelInstance*	m_pModelInstance;
	AuxFlares::C_AuxFlareInstance*	m_pAuxFlareInstance;
	C_Part*							m_pNode;
	E_AuxFlareColour				m_FlareColour;
	F32								m_Intensity;
	Maths::HMat43					m_Matrix;

	C_AuxFlare() {}
};

class C_AuxFlares
{
public:
	Btl::Vector<C_AuxFlare>	m_AuxFlareVector;
};

} //namespace Vehicle

#endif
