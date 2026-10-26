/////////////////////////////////////////////////////////////
// Material
/////////////////////////////////////////////////////////////

#include "VehiclePCH.h"

#include "Vehicle/Material/Material.h"
#include "Vehicle/PartCodes.h"
#include "Vehicle/Db/VehicleXt.h"

#include <world_render/ModelInstance.h>

#if defined(TURN_OFF_VEHICLE_OPTIMISATIONS)
	#pragma optimize("", off)
#endif

//#pragma optimize("", off)

namespace Vehicle
{

/* 
===============================================================
					class C_MaterialBase

NOTES: Material base class 
===============================================================
*/

/*
===============
NAME: C_MaterialBase

NOTE: Constructor
===============
*/
C_MaterialBase::C_MaterialBase() 
{  
	m_pModelInstance	= NULL;
	m_Type				= E_MaterialType_Base;
}

/*
===============
NAME: C_MaterialBase

NOTE: Constructor
===============
*/
C_MaterialBase::C_MaterialBase(WorldRender::C_ModelInstance* p_pModelInstance) 
{  
	m_pModelInstance	= p_pModelInstance;
	m_Type				= E_MaterialType_Base;
}

/*
===============
NAME: C_MaterialBase

NOTE: Constructor
===============
*/
C_MaterialBase::~C_MaterialBase() 
{  
	Destroy();
}

/*
===============
NAME: Translate

NOTE: Translate material from Sorcery equivalent
===============
*/
void C_MaterialBase::Translate(XtMaterialBase* p_pMaterial)
{  
	if (!p_pMaterial)
		return;
}

/*
===============
NAME: Build

NOTE: Builds material
===============
*/
void C_MaterialBase::Build()
{  	
	PERF_EVENT("C_MaterialBase::Build");

	if (!m_pModelInstance)
		return;

	for (int iParam = 0; iParam < m_Parameters.Size(); iParam++)
	{
		Parameter* pParam = &m_Parameters[iParam];

		//Should never hit this
		if (!pParam->m_pData)
			continue;		
	}	

	Set();
}

/*
===============
NAME: Set

NOTE: Sets material
===============
*/
void C_MaterialBase::Set()
{  
	if (!m_pModelInstance)
		return;

	for (int iParam = 0; iParam < m_Parameters.Size(); iParam++)
	{
		m_pModelInstance->SetParameter(m_Parameters[iParam].m_Handle, (void*)m_Parameters[iParam].m_pData);
	}
}

/*
===============
NAME: AddParameter

NOTE: Adds a parameter
===============
*/
void C_MaterialBase::AddParameter(WorldRender::MaterialParameter p_Handle, Util::C_String p_Name, bool checkExists)
{ 
	if (p_Handle.index == -1 || p_Handle.offset == -1)
		return;

	if (checkExists)
	{
		for (int iParam = 0; iParam < m_Parameters.Size(); iParam++)
		{
			if (m_Parameters[iParam].m_Handle.index == p_Handle.index &&
				m_Parameters[iParam].m_Handle.offset == p_Handle.offset &&
				m_Parameters[iParam].m_Handle.size == p_Handle.size)
				return;
		}
	}

	m_Parameters.PushBack();
	m_Parameters.Back().m_Handle	= p_Handle;
	m_Parameters.Back().m_Name		= p_Name;
	
	m_Parameters.Back().m_pData		= new float[p_Handle.size / sizeof(float)];
	memset(m_Parameters.Back().m_pData, 0, p_Handle.size);
}

/*
===============
NAME: SetParameter

NOTE: Sets a parameter
===============
*/
void C_MaterialBase::SetParameter(WorldRender::MaterialParameter p_Handle, Util::C_String p_Name)
{ 
	AddParameter(p_Handle, p_Name, true);
}

/*
===============
NAME: Destroy

NOTE: Destroys material
===============
*/
void C_MaterialBase::Destroy()
{  
	for (int iParam = 0; iParam < m_Parameters.Size(); iParam++)
	{
		SAFE_DELETE_ARY(m_Parameters[iParam].m_pData);
	}

	m_Parameters.Clear();

	m_pModelInstance = NULL;
}

/* 
===============================================================
					class C_MaterialPaint

NOTES: Paint material 
===============================================================
*/

/*
===============
NAME: C_MaterialPaint

NOTE: Constructor
===============
*/
C_MaterialPaint::C_MaterialPaint(WorldRender::C_ModelInstance* p_pModelInstance) 
{  
	m_pModelInstance	= p_pModelInstance;

	m_Type				= E_MaterialType_Paint;
	
	m_Colour			= MathsStorage::Float3(0.0f, 0.0f, 0.0f);
}

/*
===============
NAME: Translate

NOTE: Translate material from Sorcery equivalent
===============
*/
void C_MaterialPaint::Translate(XtMaterialBase* p_pMaterial)
{  
	if (!p_pMaterial)
		return;

	XtMaterialPaint* pMaterial = (XtMaterialPaint*)p_pMaterial;

	m_Colour = MathsStorage::Float3(pMaterial->m_Colour.r, pMaterial->m_Colour.g, pMaterial->m_Colour.b);
	
	LOGDEBUG(g_Log, "C_MaterialPaint::Translate() m_Colour(%f, %f, %f)", m_Colour.x, m_Colour.y, m_Colour.z);
}

/*
===============
NAME: Build

NOTE: Builds material
===============
*/
void C_MaterialPaint::Build()
{  
	PERF_EVENT("C_MaterialPaint::Build");

	if (!m_pModelInstance)
		return;

	for (int iParam = 0; iParam < m_Parameters.Size(); iParam++)
	{
		Parameter* pParam = &m_Parameters[iParam];

		//Should never hit this
		if (!pParam->m_pData)
			continue;

		//if (pParam->m_Name == "g_vPaintColour")
		//{
		//	pParam->m_pData[0] = m_Colour.x; pParam->m_pData[1] = m_Colour.y; pParam->m_pData[2] = m_Colour.z;
		//	LOGDEBUG(g_Log, "C_MaterialPaint::Build(g_vPaintColour) m_pData(%f, %f, %f, %f)", pParam->m_pData[0],pParam->m_pData[1],pParam->m_pData[2],pParam->m_pData[3]);
		//}
	}	

	Set();
}

/*
===============
NAME: SetParameter

NOTE: Sets a parameter
===============
*/
void C_MaterialPaint::SetParameter(WorldRender::MaterialParameter p_Handle, Util::C_String p_Name)
{ 
	//if (p_Name != "g_vPaintColour")
		return;

	AddParameter(p_Handle, p_Name, true);
}

/* 
===============================================================
					class C_MaterialGlass

NOTES: Glass material 
===============================================================
*/

/*
===============
NAME: C_MaterialGlass

NOTE: Constructor
===============
*/
C_MaterialGlass::C_MaterialGlass(WorldRender::C_ModelInstance* p_pModelInstance) 
{  
	m_pModelInstance	= p_pModelInstance;

	m_Type				= E_MaterialType_Glass;

	m_Tint				= MathsStorage::Float3(0.0f, 0.0f, 0.0f);

	m_MinAlpha			= 0.0f;
	m_MaxAlpha			= 0.0f;

	m_DamageLevel		= 0.0f;
}

/*
===============
NAME: Translate

NOTE: Translate material from Sorcery equivalent
===============
*/
void C_MaterialGlass::Translate(XtMaterialBase* p_pMaterial)
{  
	if (!p_pMaterial)
		return;

	XtMaterialGlass* pMaterial = (XtMaterialGlass*)p_pMaterial;

	m_Tint		= MathsStorage::Float3(pMaterial->m_Tint.r, pMaterial->m_Tint.g, pMaterial->m_Tint.b);
	m_MinAlpha	= pMaterial->m_MinAlpha;
	m_MaxAlpha	= pMaterial->m_MaxAlpha;
	m_MinReflectivity	= pMaterial->m_MinReflectivity;
	m_MaxReflectivity	= pMaterial->m_MaxReflectivity;
	m_DamageLevel		= 0.0f;
	LOGDEBUG(g_Log, "C_MaterialGlass::Translate() m_Tint(%f, %f, %f), m_MinAlpha(%f), m_MaxAlpha(%f)", m_Tint.x, m_Tint.y, m_Tint.z, m_MinAlpha, m_MaxAlpha);
}

/*
===============
NAME: Build

NOTE: Builds material
===============
*/
void C_MaterialGlass::Build()
{  
	PERF_EVENT("C_MaterialGlass::Build");

	if (!m_pModelInstance)
		return;

	for (int iParam = 0; iParam < m_Parameters.Size(); iParam++)
	{
		Parameter* pParam = &m_Parameters[iParam];

		//Should never hit this
		if (!pParam->m_pData)
			continue;

		if (pParam->m_Name == "g_vRenderSettings")
		{
			pParam->m_pData[0] = m_MinAlpha; pParam->m_pData[1] = m_MaxAlpha; pParam->m_pData[2] = m_MinReflectivity; pParam->m_pData[3] = m_MaxReflectivity;
			LOGDEBUG(g_Log, "C_MaterialGlass::Build(g_vRenderSettings) m_pData(%f, %f, %f, %f)", pParam->m_pData[0],pParam->m_pData[1],pParam->m_pData[2],pParam->m_pData[3]);
		}
		else if (pParam->m_Name == "g_vRenderColour")
		{
			pParam->m_pData[0] = m_Tint.x; pParam->m_pData[1] = m_Tint.y; pParam->m_pData[2] = m_Tint.z;
			LOGDEBUG(g_Log, "C_MaterialGlass::Build(g_vRenderColour) m_pData(%f, %f, %f, %f)", pParam->m_pData[0],pParam->m_pData[1],pParam->m_pData[2],pParam->m_pData[3]);
		}
		else if (pParam->m_Name == "g_vDamageSettings")
		{
			pParam->m_pData[0] = m_DamageLevel;
			LOGDEBUG(g_Log, "C_MaterialGlass::Build(g_vDamageSettings) m_pData(%f, %f, %f, %f)", pParam->m_pData[0],pParam->m_pData[1],pParam->m_pData[2],pParam->m_pData[3]);
		}
	}	

	Set();
}

/*
===============
NAME: SetParameter

NOTE: Sets a parameter
===============
*/
void C_MaterialGlass::SetParameter(WorldRender::MaterialParameter p_Handle, Util::C_String p_Name)
{ 
	if (p_Name != "g_vRenderSettings" && p_Name != "g_vRenderColour" && p_Name != "g_vDamageSettings")
		return;

	AddParameter(p_Handle, p_Name, true);
}

/* 
===============================================================
					class C_MaterialTyre

NOTES: Tyre material 
===============================================================
*/

/*
===============
NAME: C_MaterialTyre

NOTE: Constructor
===============
*/
C_MaterialTyre::C_MaterialTyre(WorldRender::C_ModelInstance* p_pModelInstance) 
{  
	m_pModelInstance	= p_pModelInstance;

	m_Type				= E_MaterialType_Tyre;

	m_TreadDepth		= 1.0f;
}

/*
===============
NAME: Translate

NOTE: Translate material from Sorcery equivalent
===============
*/
void C_MaterialTyre::Translate(XtMaterialBase* p_pMaterial)
{  
	if (!p_pMaterial)
		return;

	XtMaterialTyre* pMaterial = (XtMaterialTyre*)p_pMaterial;

	m_TreadDepth	= pMaterial->m_TreadDepth;
	
	LOGDEBUG(g_Log, "C_MaterialTyre::Translate() m_TreadDepth(%f)", m_TreadDepth);
}

/*
===============
NAME: Build

NOTE: Builds material
===============
*/
void C_MaterialTyre::Build()
{  
	PERF_EVENT("C_MaterialTyre::Build");

	if (!m_pModelInstance)
		return;

	for (int iParam = 0; iParam < m_Parameters.Size(); iParam++)
	{
		Parameter* pParam = &m_Parameters[iParam];

		//Should never hit this
		if (!pParam->m_pData)
			continue;

		if (pParam->m_Name == "g_vRenderSettings")
		{
			pParam->m_pData[0] = m_TreadDepth;
			LOGDEBUG(g_Log, "C_MaterialTyre::Build(g_vRenderSettings) m_pData(%f, %f, %f, %f)", pParam->m_pData[0],pParam->m_pData[1],pParam->m_pData[2],pParam->m_pData[3]);
		}
	}	

	Set();
}

/*
===============
NAME: SetParameter

NOTE: Sets a parameter
===============
*/
void C_MaterialTyre::SetParameter(WorldRender::MaterialParameter p_Handle, Util::C_String p_Name)
{ 
	if (p_Name != "g_vRenderSettings")
		return;

	AddParameter(p_Handle, p_Name, true);
}

/* 
===============================================================
					class C_MaterialAlloy

NOTES: Alloy material 
===============================================================
*/

/*
===============
NAME: C_MaterialAlloy

NOTE: Constructor
===============
*/
C_MaterialAlloy::C_MaterialAlloy(WorldRender::C_ModelInstance* p_pModelInstance) 
{  
	m_pModelInstance	= p_pModelInstance;

	m_Type				= E_MaterialType_Alloy;
	
	m_Colour			= MathsStorage::Float4(0.0f, 0.0f, 0.0f, 1.0f);

	m_SpecularPower		= 0.0f;
	m_SpecularIntensity	= 0.0f;
}

/*
===============
NAME: Translate

NOTE: Translate material from Sorcery equivalent
===============
*/
void C_MaterialAlloy::Translate(XtMaterialBase* p_pMaterial)
{  
	if (!p_pMaterial)
		return;

	XtMaterialAlloy* pMaterial = (XtMaterialAlloy*)p_pMaterial;

	m_Colour			= MathsStorage::Float4(pMaterial->m_Colour.r, pMaterial->m_Colour.g, pMaterial->m_Colour.b, 1.0f);
	m_SpecularPower		= pMaterial->m_SpecularPower;
	m_SpecularIntensity	= pMaterial->m_SpecularIntensity;
	
	LOGDEBUG(g_Log, "C_MaterialAlloy::Translate() m_Colour(%f, %f, %f), m_SpecularPower(%f), m_SpecularIntensity(%f)", m_Colour.x, m_Colour.y, m_Colour.z, m_SpecularPower, m_SpecularIntensity);
}

/*
===============
NAME: Build

NOTE: Builds material
===============
*/
void C_MaterialAlloy::Build()
{  
	PERF_EVENT("C_MaterialAlloy::Build");

	if (!m_pModelInstance)
		return;

	for (int iParam = 0; iParam < m_Parameters.Size(); iParam++)
	{
		Parameter* pParam = &m_Parameters[iParam];

		//Should never hit this
		if (!pParam->m_pData)
			continue;

		if (pParam->m_Name == "g_vRenderSettings")
		{
			pParam->m_pData[0] = m_SpecularPower; pParam->m_pData[1] = m_SpecularIntensity;
			LOGDEBUG(g_Log, "C_MaterialAlloy::Build(g_vRenderSettings) m_pData(%f, %f, %f, %f)", pParam->m_pData[0],pParam->m_pData[1],pParam->m_pData[2],pParam->m_pData[3]);
		}
		else if (pParam->m_Name == "g_vRenderColour")
		{
			pParam->m_pData[0] = m_Colour.x; pParam->m_pData[1] = m_Colour.y; pParam->m_pData[2] = m_Colour.z; pParam->m_pData[3] = m_Colour.w;
			LOGDEBUG(g_Log, "C_MaterialAlloy::Build(g_vRenderColour) m_pData(%f, %f, %f, %f)", pParam->m_pData[0],pParam->m_pData[1],pParam->m_pData[2],pParam->m_pData[3]);
		}
	}	

	Set();
}

/*
===============
NAME: SetParameter

NOTE: Sets a parameter
===============
*/
void C_MaterialAlloy::SetParameter(WorldRender::MaterialParameter p_Handle, Util::C_String p_Name)
{ 
	if (p_Name != "g_vRenderSettings" && p_Name != "g_vRenderColour")
		return;

	AddParameter(p_Handle, p_Name, true);
}

/* 
===============================================================
					class C_MaterialLight

NOTES: Light material 
===============================================================
*/

/*
===============
NAME: C_MaterialLight

NOTE: Constructor
===============
*/
C_MaterialLight::C_MaterialLight(WorldRender::C_ModelInstance* p_pModelInstance) 
{  
	m_pModelInstance	= p_pModelInstance;

	m_Type				= E_MaterialType_Light;

	m_Tint				= MathsStorage::Float3(0.0f, 0.0f, 0.0f);

	m_Intensity			= 0.0f;
	m_DefaultIntensity	= 0.0f;
	m_Noise				= 1.0f;

	m_Damaged			= false;
	m_KnockedOut		= false;
}

/*
===============
NAME: Translate

NOTE: Translate material from Sorcery equivalent
===============
*/
void C_MaterialLight::Translate(XtMaterialBase* p_pMaterial)
{  
	if (!p_pMaterial)
		return;

	XtMaterialLight* pMaterial = (XtMaterialLight*)p_pMaterial;

	m_Tint			= MathsStorage::Float3(pMaterial->m_Tint.r, pMaterial->m_Tint.g, pMaterial->m_Tint.b);
	m_Intensity		= pMaterial->m_Intensity;
	m_DefaultIntensity	= m_Intensity;
	
	LOGDEBUG(g_Log, "C_MaterialLight::Translate() m_Tint(%f, %f, %f), m_Intensity(%f)", m_Tint.x, m_Tint.y, m_Tint.z, m_Intensity);
}

Debug::Float g_debugLightDamageDimScale("Vehicle.Damage.Lights.DimScale", 0.5f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

/*
===============
NAME: Build

NOTE: Builds material
===============
*/
void C_MaterialLight::Build()
{
//	PERF_EVENT("C_MaterialLight::Build");

	if (!m_pModelInstance)
		return;

	for (int iParam = 0; iParam < m_Parameters.Size(); iParam++)
	{
		Parameter* pParam = &m_Parameters[iParam];

		//Should never hit this
		if (!pParam->m_pData)
			continue;

		if (pParam->m_Name == "g_vRenderColour")
		{			
			pParam->m_pData[0] = m_Tint.x; pParam->m_pData[1] = m_Tint.y; pParam->m_pData[2] = m_Tint.z; pParam->m_pData[3] = Intensity();
			LOGDEBUG(g_Log, "C_MaterialLight::Build(g_vRenderColour) m_pData(%f, %f, %f, %f)", pParam->m_pData[0],pParam->m_pData[1],pParam->m_pData[2],pParam->m_pData[3]);
		}
		else if (pParam->m_Name == "g_vDamageSettings")
		{
			pParam->m_pData[0] = m_Damaged ? 1.0f : 0.0f;
			LOGDEBUG(g_Log, "C_MaterialLight::Build(g_vDamageSettings) m_pData(%f, %f, %f, %f)", pParam->m_pData[0],pParam->m_pData[1],pParam->m_pData[2],pParam->m_pData[3]);
		}
	}	

	Set();
}

/*
===============
NAME: Intensity

NOTE: Gets intensity
===============
*/
float C_MaterialLight::Intensity()
{
	float scale = m_Damaged ? g_debugLightDamageDimScale : 1.0f;
	scale = m_KnockedOut ? 0.0f : scale;
	return m_Intensity * m_DefaultIntensity * scale * m_Noise;
}

/*
===============
NAME: FlareIntensity

NOTE: Gets intensity for light flare
===============
*/
float C_MaterialLight::FlareIntensity()
{
	float scale = m_Damaged ? g_debugLightDamageDimScale : 1.0f;
	scale = m_KnockedOut ? 0.0f : scale;
	return m_Intensity * scale * m_Noise;
}

/*
===============
NAME: SetParameter

NOTE: Sets a parameter
===============
*/
void C_MaterialLight::SetParameter(WorldRender::MaterialParameter p_Handle, Util::C_String p_Name)
{ 
	if (p_Name != "g_vRenderColour" && p_Name != "g_vDamageSettings")
		return;

	AddParameter(p_Handle, p_Name, true);
}

/* 
===============================================================
					class C_MaterialManager

NOTES: Manages materials and defaults
===============================================================
*/

C_MaterialManager C_MaterialManager::s_Instance;

/*
===============
NAME: C_MaterialManager

NOTE: Constructor
===============
*/
C_MaterialManager::C_MaterialManager()
{

}

/*
===============
NAME: ~C_MaterialManager

NOTE: Destructor
===============
*/
C_MaterialManager::~C_MaterialManager()
{
	
}

/*
===============
NAME: GetXtMaterial

NOTE: Returns the Xt material for the part
===============
*/
XtMaterialBase* C_MaterialManager::GetXtMaterial(XtVehicle* p_pVehicle, 
												  XtMaterialDefaults* p_pMaterialDefaults,
												  E_PartCode p_PartCode, E_PartSubCode p_SubCode)
{
	if (!p_pVehicle || !p_pMaterialDefaults)
		return NULL;

	//Think I may only need part code here
	XtMaterialBase*		pXtMaterial = NULL;

	//Check to see if this vehicle overrides any defaults
	if (p_pVehicle->m_MaterialOverrides)
	{
		XtMaterialBase*	pDefaultXtMaterial = NULL;
		for (int iMaterial = 0; iMaterial < p_pVehicle->m_MaterialOverrides->m_Defaults.count; iMaterial++)
		{
			if (p_pVehicle->m_MaterialOverrides->m_Defaults[iMaterial]->m_Part.m_PartCode == E_PartCode_None)
				pDefaultXtMaterial = p_pVehicle->m_MaterialOverrides->m_Defaults[iMaterial]->m_Material;

			if (p_pVehicle->m_MaterialOverrides->m_Defaults[iMaterial]->m_Part.m_PartCode == p_PartCode)
			{
				pXtMaterial = p_pVehicle->m_MaterialOverrides->m_Defaults[iMaterial]->m_Material;
				break;
			}
		}

		if (!pXtMaterial)
			pXtMaterial = pDefaultXtMaterial;
	}

	//Vehicle does not override material for this part so get default
	if (!pXtMaterial)
	{
		XtMaterialBase*	pDefaultXtMaterial = NULL;
		for (int iMaterial = 0; iMaterial < p_pMaterialDefaults->m_Defaults.count; iMaterial++)
		{
			if (p_pMaterialDefaults->m_Defaults[iMaterial]->m_Part.m_PartCode == E_PartCode_None)
				pDefaultXtMaterial = p_pMaterialDefaults->m_Defaults[iMaterial]->m_Material;

			if (p_pMaterialDefaults->m_Defaults[iMaterial]->m_Part.m_PartCode == p_PartCode)
			{
				pXtMaterial = p_pMaterialDefaults->m_Defaults[iMaterial]->m_Material;
				break;
			}
		}

		if (!pXtMaterial)
			pXtMaterial = pDefaultXtMaterial;
	}

	return pXtMaterial;}

/*
===============
NAME: CreateMaterial

NOTE: Creates material for part
===============
*/
C_MaterialBase* C_MaterialManager::CreateMaterial(XtVehicle* p_pVehicle, 
												  XtMaterialDefaults* p_pMaterialDefaults,
												  WorldRender::C_ModelInstance* p_pModelInstance,
												  E_PartCode p_PartCode, E_PartSubCode p_SubCode)
{
	if (!p_pVehicle || !p_pMaterialDefaults)
		return NULL;

	//Think I may only need part code here
	C_MaterialBase*		pMaterial = NULL;
	XtMaterialBase*		pXtMaterial = GetXtMaterial(p_pVehicle, p_pMaterialDefaults, p_PartCode, p_SubCode);

	////Check to see if this vehicle overrides any defaults
	//if (p_pVehicle->m_MaterialOverrides)
	//{
	//	XtMaterialBase*	pDefaultXtMaterial = NULL;
	//	for (int iMaterial = 0; iMaterial < p_pVehicle->m_MaterialOverrides->m_Defaults.count; iMaterial++)
	//	{
	//		if (p_pVehicle->m_MaterialOverrides->m_Defaults[iMaterial]->m_Part.m_PartCode == E_PartCode_None)
	//			pDefaultXtMaterial = p_pVehicle->m_MaterialOverrides->m_Defaults[iMaterial]->m_Material;

	//		if (p_pVehicle->m_MaterialOverrides->m_Defaults[iMaterial]->m_Part.m_PartCode == p_PartCode)
	//		{
	//			pXtMaterial = p_pVehicle->m_MaterialOverrides->m_Defaults[iMaterial]->m_Material;
	//			break;
	//		}
	//	}

	//	if (!pXtMaterial)
	//		pXtMaterial = pDefaultXtMaterial;
	//}

	////Vehicle does not override material for this part so get default
	//if (!pXtMaterial)
	//{
	//	XtMaterialBase*	pDefaultXtMaterial = NULL;
	//	for (int iMaterial = 0; iMaterial < p_pMaterialDefaults->m_Defaults.count; iMaterial++)
	//	{
	//		if (p_pMaterialDefaults->m_Defaults[iMaterial]->m_Part.m_PartCode == E_PartCode_None)
	//			pDefaultXtMaterial = p_pMaterialDefaults->m_Defaults[iMaterial]->m_Material;

	//		if (p_pMaterialDefaults->m_Defaults[iMaterial]->m_Part.m_PartCode == p_PartCode)
	//		{
	//			pXtMaterial = p_pMaterialDefaults->m_Defaults[iMaterial]->m_Material;
	//			break;
	//		}
	//	}

	//	if (!pXtMaterial)
	//		pXtMaterial = pDefaultXtMaterial;
	//}

	//FAIL!
	if (!pXtMaterial)
		return NULL;

	switch (pXtMaterial->m_Type)
	{
	case E_MaterialType_Paint:
		{
			pMaterial = new C_MaterialPaint(p_pModelInstance);
			if (pMaterial)
				((C_MaterialPaint*)pMaterial)->Translate((XtMaterialPaint*)pXtMaterial);
			break;
		}
	case E_MaterialType_Glass:
		{
			pMaterial = new C_MaterialGlass(p_pModelInstance);
			if (pMaterial)
				((C_MaterialGlass*)pMaterial)->Translate((XtMaterialGlass*)pXtMaterial);
			break;
		}
	case E_MaterialType_Tyre:
		{
			pMaterial = new C_MaterialTyre(p_pModelInstance);
			if (pMaterial)
				((C_MaterialTyre*)pMaterial)->Translate((XtMaterialTyre*)pXtMaterial);
			break;
		}
	case E_MaterialType_Alloy:
		{
			pMaterial = new C_MaterialAlloy(p_pModelInstance);
			if (pMaterial)
				((C_MaterialAlloy*)pMaterial)->Translate((XtMaterialAlloy*)pXtMaterial);
			break;
		}
	case E_MaterialType_Light:
		{
			pMaterial = new C_MaterialLight(p_pModelInstance);
			if (pMaterial)
				((C_MaterialLight*)pMaterial)->Translate((XtMaterialLight*)pXtMaterial);
			break;
		}
	case E_MaterialType_Base:
	default:
		{
			pMaterial = new C_MaterialBase(p_pModelInstance);
			if (pMaterial)
				((C_MaterialBase*)pMaterial)->Translate((XtMaterialBase*)pXtMaterial);
			break;
		}
	}
	return pMaterial;
}

/*
===============
NAME: TranslateMaterial

NOTE: Translates a material from Xt to game
===============
*/
void C_MaterialManager::TranslateMaterial(XtMaterialBase* p_pXtMaterial, C_MaterialBase* p_pMaterial)
{
	if (!p_pXtMaterial || !p_pMaterial ||
		p_pXtMaterial->m_Type != p_pMaterial->m_Type)
		return;

	switch (p_pXtMaterial->m_Type)
	{
	case E_MaterialType_Paint:
		{
			((C_MaterialPaint*)p_pMaterial)->Translate((XtMaterialPaint*)p_pXtMaterial);
			break;
		}
	case E_MaterialType_Glass:
		{
			((C_MaterialGlass*)p_pMaterial)->Translate((XtMaterialGlass*)p_pXtMaterial);
			break;
		}
	case E_MaterialType_Tyre:
		{
			((C_MaterialTyre*)p_pMaterial)->Translate((XtMaterialTyre*)p_pXtMaterial);
			break;
		}
	case E_MaterialType_Alloy:
		{
			((C_MaterialAlloy*)p_pMaterial)->Translate((XtMaterialAlloy*)p_pXtMaterial);
			break;
		}
	case E_MaterialType_Light:
		{
			((C_MaterialLight*)p_pMaterial)->Translate((XtMaterialLight*)p_pXtMaterial);
			break;
		}
	case E_MaterialType_Base:
	default:
		{
			((C_MaterialBase*)p_pMaterial)->Translate((XtMaterialBase*)p_pXtMaterial);
			break;
		}
	}
}

} //namespace Vehicle
