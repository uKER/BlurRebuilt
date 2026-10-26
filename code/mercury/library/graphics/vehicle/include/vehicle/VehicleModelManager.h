/////////////////////////////////////////////////////////////
// VehicleModelManager
/////////////////////////////////////////////////////////////

#ifndef __VEHICLE_MODEL_MANAGER_H__
#define __VEHICLE_MODEL_MANAGER_H__

#include <btl/containers/vector.h>
#include <btl/containers/map.h>

#include <game_resource/ModelTypeManager.h>

#include <Vehicle/PartCodes.h>

#include <fx/fx.h>

namespace SceneGraph { class C_SceneGraph; }
namespace Game { class C_GameWorld; }
namespace Gravity { class C_Body; }
namespace GameResource { class C_ModelResource; }

namespace Vehicle
{

class C_VehicleModel;
class C_VehicleModelSetup;
class C_Headlight;

/* 
===============================================================
					class C_VehicleModelManager

NOTES: Holds all the C_VehicleModels. Handles updates, rendering.
===============================================================
*/
class C_VehicleModelManager
{
public:

	static C_VehicleModelManager& Instance() { return s_Instance; }

	C_VehicleModelManager();

	~C_VehicleModelManager();
	
	//! Registers model types
	void RegisterModelTypes();

	//! Initialise
	void Initialise();

	//! Creates the manager
	bool Create();

	//! Destroys the manager
	bool Destroy();

	//! Update the vehicle models
	void Update();

	//! Performs pre-rendering tasks, such as sorting and culling
	void PreRender();

	//! Loads a vehicle model and adds it
	C_VehicleModel* AddVehicleModel(const C_VehicleModelSetup* p_pModelSetup);
	
	//! Creates a vehicle model
	C_VehicleModel* CreateVehicleModel(const C_VehicleModelSetup* p_pModelSetup);

	//! Removes vehicle model
	void RemoveVehicleModel(U32 p_Index);

	//! Returns pointer to vehicle model (or NULL if invalid index)
	C_VehicleModel* GetVehicleModel(U32 p_iVehicleIndex);

	U32 GetNumVehicles() { return m_pVehicleModels.Size(); }

	// ~~~~~~~~~~~~~ DEBUG ~~~~~~~~~~~~~

	C_Headlight* GetHeadlight(U32 p_Index);

	bool m_DEBUG_RegisterCarModelType;
	bool m_DEBUG_RegisterWheelModelType;

	//REMOVE_DIRT
	//Display::C_Texture2D m_DirtLayerTexture;
private:

	static C_VehicleModelManager	s_Instance;								//!< Instance

	Btl::Vector<C_VehicleModel*>	m_pVehicleModels;						//!< All the vehicle models in the system

public:

	void SetCubeMapTexture(const void* pAddress) { m_pCubeMapTexture = pAddress;}
	const void*						m_pCubeMapTexture;

	FX::SamplerStateHandle	m_SamplerState, m_WrapSamplerState;
	//REMOVE_DIRT
	FX::ParameterHandle		m_VolumeTex, m_DamageInfoTex;//, m_DirtLayerTex;
};

/* 
===============================================================
					class C_ModelTypeCar

NOTES: Car model type for dynamic material setting
===============================================================
*/
class C_ModelTypeCar : public GameResource::I_ModelType
{
public:
	C_ModelTypeCar(){}
	void OnLoad(GameResource::C_ModelResource* p_pModelResource);
public:
};

/* 
===============================================================
					class C_ModelTypeWheel

NOTES: Wheel model type for dynamic material setting
===============================================================
*/
class C_ModelTypeWheel : public GameResource::I_ModelType
{
public:
	C_ModelTypeWheel(){}
	void OnLoad(GameResource::C_ModelResource* p_pModelResource);
public:
};

} //namespace Vehicle

#endif
