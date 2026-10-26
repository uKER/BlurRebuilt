/////////////////////////////////////////////////////////////
// Material
/////////////////////////////////////////////////////////////

#ifndef __VEHICLE_MATERIAL_H__
#define __VEHICLE_MATERIAL_H__

#include <world_render/RenderModelData.h>

#include <Vehicle/Material/MaterialTypes.h>
#include <Vehicle/PartCodes.h>

namespace WorldRender { class C_ModelInstance; }

namespace Vehicle
{

struct XtVehicle;
struct XtMaterialDefaults;
class XtMaterialBase;

/* 
===============================================================
					class C_MaterialBase

NOTES: Material base class
===============================================================
*/
class C_MaterialBase
{
public:

	C_MaterialBase();
	C_MaterialBase(WorldRender::C_ModelInstance* p_pModelInstance);
	~C_MaterialBase();

	virtual void Translate(XtMaterialBase* p_pMaterial);

	virtual void Build();

	void Set();

	void AddParameter(WorldRender::MaterialParameter p_Handle, Util::C_String p_Name, bool checkExists = false);

	virtual void SetParameter(WorldRender::MaterialParameter p_Handle, Util::C_String p_Name);

	void Destroy();

public:

	struct Parameter
	{
		Util::C_String					m_Name;
		WorldRender::MaterialParameter	m_Handle;
		float*							m_pData;
	};

	Btl::Vector<Parameter>	m_Parameters;

	E_MaterialType		m_Type;

	WorldRender::C_ModelInstance*		m_pModelInstance;
};

/* 
===============================================================
					class C_MaterialPaint

NOTES: Paint material
===============================================================
*/
class C_MaterialPaint : public C_MaterialBase
{
public:

	C_MaterialPaint(WorldRender::C_ModelInstance* p_pModelInstance);

	void Translate(XtMaterialBase* p_pMaterial);

	void Build();

	void SetParameter(WorldRender::MaterialParameter p_Handle, Util::C_String p_Name);

public:

	MathsStorage::Float3	m_Colour;
};

/* 
===============================================================
					class C_MaterialGlass

NOTES: Glass material
===============================================================
*/
class C_MaterialGlass : public C_MaterialBase
{
public:

	C_MaterialGlass(WorldRender::C_ModelInstance* p_pModelInstance);

	void Translate(XtMaterialBase* p_pMaterial);

	void Build();

	void SetParameter(WorldRender::MaterialParameter p_Handle, Util::C_String p_Name);

public:

	MathsStorage::Float3	m_Tint;
	float					m_MinAlpha;
	float					m_MaxAlpha;
	float					m_MinReflectivity;
	float					m_MaxReflectivity;
	float					m_DamageLevel;
};

/* 
===============================================================
					class C_MaterialTyre

NOTES: Tyre material
===============================================================
*/
class C_MaterialTyre : public C_MaterialBase
{
public:

	C_MaterialTyre(WorldRender::C_ModelInstance* p_pModelInstance);

	void Translate(XtMaterialBase* p_pMaterial);

	void Build();

	void SetParameter(WorldRender::MaterialParameter p_Handle, Util::C_String p_Name);

public:

	float					m_TreadDepth;
};

/* 
===============================================================
					class C_MaterialAlloy

NOTES: Alloy material
===============================================================
*/
class C_MaterialAlloy : public C_MaterialBase
{
public:

	C_MaterialAlloy(WorldRender::C_ModelInstance* p_pModelInstance);

	void Translate(XtMaterialBase* p_pMaterial);

	void Build();

	void SetParameter(WorldRender::MaterialParameter p_Handle, Util::C_String p_Name);

public:

	MathsStorage::Float4	m_Colour;
	float					m_SpecularPower;
	float					m_SpecularIntensity;
};

/* 
===============================================================
					class C_MaterialLight

NOTES: Light material
===============================================================
*/
class C_MaterialLight : public C_MaterialBase
{
public:

	C_MaterialLight(WorldRender::C_ModelInstance* p_pModelInstance);

	void Translate(XtMaterialBase* p_pMaterial);

	void Build();

	void SetParameter(WorldRender::MaterialParameter p_Handle, Util::C_String p_Name);

	float Intensity();

	float FlareIntensity();

public:

	MathsStorage::Float3	m_Tint;
	float					m_Intensity;
	float					m_DefaultIntensity;
	float					m_Noise;
	bool					m_Damaged;
	bool					m_KnockedOut;
};

///* 
//===============================================================
//					class C_MaterialNEW
//
//NOTES: NEW material
//===============================================================
//*/
//class C_MaterialNEW
//{
//public:
//
//	C_MaterialNEW();
//
//public:
//
//	struct Parameter
//	{
//		WorldRender::MaterialParameter	m_Handle;
//		float*							m_pData;
//	};
//
//	Btl::Vector<Parameter>	m_Parameters;
//};

/* 
===============================================================
					class C_MaterialManager

NOTES: Manages materials and defaults
===============================================================
*/
class C_MaterialManager
{
public:

	static C_MaterialManager& Instance() { return s_Instance; }

	C_MaterialManager();
	~C_MaterialManager();

	//! Returns the Xt material for the part
	XtMaterialBase* GetXtMaterial(XtVehicle* p_pVehicle, 
								  XtMaterialDefaults* p_pMaterialDefaults,
								  E_PartCode p_PartCode, E_PartSubCode p_SubCode);

	//! Creates material for part
	C_MaterialBase* CreateMaterial(	XtVehicle* p_pVehicle, 
									XtMaterialDefaults* p_pMaterialDefaults,
									WorldRender::C_ModelInstance* p_pModelInstance,
									E_PartCode p_PartCode, E_PartSubCode p_SubCode);

	//! Translates a material from Xt to game
	void TranslateMaterial(XtMaterialBase* p_pXtMaterial, C_MaterialBase* p_pMaterial);

public:

	static C_MaterialManager						s_Instance;
};

} //namespace Vehicle

#endif
