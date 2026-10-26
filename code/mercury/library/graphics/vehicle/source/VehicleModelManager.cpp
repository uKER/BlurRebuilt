/////////////////////////////////////////////////////////////
// VehicleModelManager
/////////////////////////////////////////////////////////////

#include "VehiclePCH.h"

#include "Vehicle/VehicleModelManager.h"
#include "Vehicle/VehicleModel.h"
#include "Vehicle/PartCodes.h"
#include "Vehicle/Setup.h"
#include "Vehicle/RenderLayer.h"
#include "Vehicle/CodeProcessor.h"
#include "Vehicle/Damage/DamageLayer.h"

#include "scenegraph/scenegraph.h"
#include "scenegraph/visible/backfrontsorter.h"
#include "scenegraph/rendernode.h"

#include <gravity/sdk/Body.h>

#include <fx/fx.h>
#include <fx/RenderMutex.h>

#include <game_resource/ModelData.h>
#include <game_resource/ModelResource.h>
#include <world_render/RenderModel.h>

#if defined(TURN_OFF_VEHICLE_OPTIMISATIONS)
	#pragma optimize("", off)
#endif

namespace Vehicle
{

C_VehicleModelManager C_VehicleModelManager::s_Instance;

/* 
===============================================================
					class C_VehicleModelManager

NOTES: Holds all the C_VehicleModels. Handles updates, rendering.
===============================================================
*/

/*
===============
NAME: C_VehicleModelManager

NOTE: Constructor
===============
*/
C_VehicleModelManager::C_VehicleModelManager()
{
	m_DEBUG_RegisterCarModelType = true;
	m_DEBUG_RegisterWheelModelType = true;

	m_pCubeMapTexture = NULL;
}

/*
===============
NAME: ~C_VehicleModelManager

NOTE: Denstructor
===============
*/
C_VehicleModelManager::~C_VehicleModelManager()
{

}

/*
===============
NAME: Initialise

NOTE: Initialise
===============
*/
void C_VehicleModelManager::Initialise()
{
	C_CodeProcessor::Instance().BuildNameMaps();

	//for (U32 iVehicle = 0; iVehicle < m_pVehicleModels.Size(); iVehicle++)
	//{
	//	if (m_pVehicleModels[iVehicle])
	//		m_pVehicleModels[iVehicle]->Initialise();
	//}
}

Debug::Logger &g_VehicleModelManagerLog( Debug::GetLogger("mercury.vehicle.modelmanager") );
/*
===============
NAME: Create

NOTE: Creates the manager
===============
*/
bool C_VehicleModelManager::Create()
{
	FX::LockSerialiseMutex();
	//REMOVE_DIRT
	//LOGWARN(g_VehicleModelManagerLog, "'VehicleDirtLayer.dds' needs to be loaded from a pak file!");
	//bool ftw = Display::CreateTexture2DFromFile("dontship/Textures/VehicleDirtLayer.dds", &m_DirtLayerTexture);
	//m_DirtLayerTex = FX::GetSharedParameter("g_sVehicleDirtLayerTexture");

	m_VolumeTex = FX::GetSharedParameter("g_sVehicleDamageVolumeTexture");
	m_DamageInfoTex = FX::GetSharedParameter("g_sVehicleDamageInfoTexture");
	
#ifdef PLATFORM_PS3
	m_SamplerState = FX::GetSamplerState(
						Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp,
						Display::E_TextureFilter_Point, Display::E_TextureFilter_Point, Display::E_TextureFilter_Point,
						1, 0, 0);
#else
	m_SamplerState = FX::GetSamplerState(
						Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp,
						Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear,
						1, 0, 0);
#endif
	m_WrapSamplerState = FX::GetSamplerState(
						Display::E_TextureAddress_Wrap, Display::E_TextureAddress_Wrap, Display::E_TextureAddress_Wrap,
						Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear,
						1, 0, 0);	
	FX::ReleaseSerialiseMutex();

	return true;
}

/*
===============
NAME: Destroy

NOTE: Destroys the manager
===============
*/
bool C_VehicleModelManager::Destroy()
{
	/*for (int iVehicle = 0; iVehicle < m_pVehicleModels.Size(); iVehicle++)
	{
		C_VehicleModel* pVehicleModel = m_pVehicleModels[iVehicle];

		if (pVehicleModel)
		{
			pVehicleModel->Destroy();
			SAFE_DELETE_PTR(pVehicleModel);
		}
	}*/

	m_pVehicleModels.Clear();

	C_CodeProcessor::Instance().Clear();

	//Display::Device()->Destroy(&m_CubeMapTexture);
	//REMOVE_DIRT
	//Display::Device()->Destroy(&m_DirtLayerTexture);

	return true;
}

/*
===============
NAME: Update

NOTE: Update the vehicle models
===============
*/
void C_VehicleModelManager::Update()
{	
	for (int iVehicle = 0; iVehicle < m_pVehicleModels.Size(); iVehicle++)
	{
		PERF_EVENT("C_VehicleModelManager::Update Model");

		C_VehicleModel* pVehicleModel = m_pVehicleModels[iVehicle];

		if (pVehicleModel && pVehicleModel->GetModelInput()->ShouldUpdate())
			pVehicleModel->Update();
	}
}

Debug::Float g_debugLightElevation	("Vehicle.Render.Lighting.LightElevation",	0.75f, Debug::Range(-3.2f, 3.2f), Debug::Step(0.001f));
Debug::Float g_debugLightRotation	("Vehicle.Render.Lighting.LightRotation",	3.1f, Debug::Range(-3.2f, 3.2f), Debug::Step(0.001f));
Debug::Colour g_debugLightColour	("Vehicle.Render.Lighting.LightColour",		0.71f, 0.71f, 0.71f, 1.0f);
Debug::Colour g_debugGroundColour	("Vehicle.Render.Lighting.GroundColour",	0.208f, 0.2f, 0.184f, 1.0f);
Debug::Colour g_debugSkyColour		("Vehicle.Render.Lighting.SkyColour",		0.31f, 0.318f, 0.392f, 1.0f);

Debug::Float g_debugScratchLevel	("Vehicle.Render.Material.ScratchLevel",			0.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugBumpLevel		("Vehicle.Render.Material.BumpLevel",				1.0f, Debug::Range(0.0f, 10.0f), Debug::Step(0.001f));
Debug::Float g_debugDamageSpecPow	("Vehicle.Render.Material.DamageSpecularPower",		0.75f, Debug::Range(0.0f, 10.0f), Debug::Step(0.001f));
Debug::Float g_debugDamageSpecInt	("Vehicle.Render.Material.DamageSpecularIntensity",	2.0f, Debug::Range(0.0f, 10.0f), Debug::Step(0.001f));

Debug::Float g_debugMinReflection	("Vehicle.Render.Material.MinReflection",			0.0f, Debug::Range(0.0f, 10.0f), Debug::Step(0.001f));
Debug::Float g_debugMaxReflection	("Vehicle.Render.Material.MaxReflection",			1.0f, Debug::Range(0.0f, 10.0f), Debug::Step(0.001f));
Debug::Float g_debugSpecPow			("Vehicle.Render.Material.SpecularPower",			6.0f, Debug::Range(0.0f, 10.0f), Debug::Step(0.001f));
Debug::Float g_debugSpecInt			("Vehicle.Render.Material.SpecularIntensity",		1.5f, Debug::Range(0.0f, 10.0f), Debug::Step(0.001f));

Debug::Float g_debugMinAlpha		("Vehicle.Render.Material.MinAlpha",			0.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugMaxAlpha		("Vehicle.Render.Material.MaxAlpha",			1.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

Debug::Colour g_debugPaintColour	("Vehicle.Render.Material.PaintColour",	1.0f, 1.0f, 0.0f, 1.0f);

/*
===============
NAME: PreRender

NOTE: Performs pre-rendering tasks, such as sorting and culling
===============
*/

void C_VehicleModelManager::PreRender()
{
	PERF_EVENT("VehicleModelManager.PreRender");
	// FYI SetDynamicShaderParameters is now called from within the vehicle actor update

	FX::SetSamplerState(m_VolumeTex, m_SamplerState);
	FX::SetSamplerState(m_DamageInfoTex, m_SamplerState);
	//REMOVE_DIRT
	//FX::SetSamplerState(m_DirtLayerTex, m_WrapSamplerState);
}

/*
===============
NAME: AddVehicleModel

NOTE: Loads a vehicle model and adds it
===============
*/
C_VehicleModel* C_VehicleModelManager::AddVehicleModel(const C_VehicleModelSetup* p_pModelSetup)
{
	C_VehicleModel* pVehicleModel = new C_VehicleModel();
	
	if (!pVehicleModel->Create(p_pModelSetup) || !pVehicleModel->Load(p_pModelSetup))
	{
		pVehicleModel->Destroy();
		SAFE_DELETE_PTR(pVehicleModel);
		return NULL;
	}

	m_pVehicleModels.PushBack(pVehicleModel);

	return pVehicleModel;
}

/*
===============
NAME: CreateVehicleModel

NOTE: Creates a vehicle model
===============
*/
C_VehicleModel* C_VehicleModelManager::CreateVehicleModel(const C_VehicleModelSetup* p_pModelSetup)
{
	C_VehicleModel* pVehicleModel = new C_VehicleModel();
	
	if (!pVehicleModel->Create(p_pModelSetup) || !pVehicleModel->Load(p_pModelSetup))
	{
		pVehicleModel->Destroy();
		SAFE_DELETE_PTR(pVehicleModel);
		return NULL;
	}

	return pVehicleModel;
}

/*
===============
NAME: RemoveVehicleModel

NOTE: Removes vehicle model
===============
*/
void C_VehicleModelManager::RemoveVehicleModel(U32 p_Index)
{
	if (int(p_Index) >= m_pVehicleModels.Size() || !m_pVehicleModels[p_Index])
		return;

	//SAFE_DELETE_PTR(m_pVehicleModels[p_Index]);
	m_pVehicleModels[p_Index] = NULL;
}

/*
===============
NAME: GetVehicleModel

NOTE: Returns pointer to vehicle model (or NULL if invalid index)
===============
*/
C_VehicleModel* C_VehicleModelManager::GetVehicleModel(U32 p_iVehicleIndex)
{
	if (int(p_iVehicleIndex) >= m_pVehicleModels.Size())
		return NULL;

	return m_pVehicleModels[p_iVehicleIndex];
}

/*
===============
NAME: RegisterModelTypes

NOTE: Registers model types
===============
*/
void C_VehicleModelManager::RegisterModelTypes()
{
	if (m_DEBUG_RegisterCarModelType)
		GameResource::g_ModelTypeManager.RegisterModelType("car", new C_ModelTypeCar());
	if (m_DEBUG_RegisterWheelModelType)
		GameResource::g_ModelTypeManager.RegisterModelType("wheel", new C_ModelTypeWheel());
}

/*
===============
NAME: GetHeadlight

NOTE: Returns headlights
===============
*/
C_Headlight* C_VehicleModelManager::GetHeadlight(U32 p_Index)
{
	if (p_Index >= GetNumVehicles() || !m_pVehicleModels[p_Index]->GetModelInput()->m_bHeadLights.m_Current)
		return NULL;

	if (!m_pVehicleModels[p_Index]->GetHeadlight()->m_Enabled)
		return NULL;

	return m_pVehicleModels[p_Index]->GetHeadlight();
}

/* 
===============================================================
					class C_ModelTypeCar

NOTES: Car model type for dynamic material setting
===============================================================
*/

Debug::Bool g_debugExposeLiveryTexture("Vehicle.Livery.Expose", false);

/*
===============
NAME: OnLoad

NOTE: Exposes parameters
===============
*/
void C_ModelTypeCar::OnLoad(GameResource::C_ModelResource* p_pModelResource)
{
	if (!p_pModelResource)
		return;

	GameResource::C_ModelData *pModelDesc = p_pModelResource->GetModelData();
	WorldRender::C_RenderModelData *pGRModel = p_pModelResource->GetRenderModelData();

	// for each model inside the file
	for (int iModel = 0; iModel < pModelDesc->GetModelCount(); iModel++)
	{
		GameResource::C_Model *pModel = pModelDesc->GetModel(iModel);
		WorldRender::C_Model *pWRModel = &p_pModelResource->GetRenderModelData()->GetModel(iModel);

		Util::C_String modelName = pModel->GetName();
		modelName.ConvertToLower();

		if (modelName == "m_shadowcaster")
		{
			pWRModel->ExposeParameter("g_WorldMatrix", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 16);
			pWRModel->ExposeParameter("g_ShadowPlaneSpaceMatrix", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 16);
			pWRModel->ExposeParameter("g_ShadowPlane", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);
			pWRModel->ExposeParameter("g_LightPosition", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);
			pWRModel->ExposeParameter("g_ShadowAlpha", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);
		}
		else
		{
			pWRModel->ExposeParameter("g_sPaintMaterialTexture", RenderingData::MATERIAL_ELEMENT_TYPE_TEXTURE_PTR, 1);
			if (g_debugExposeLiveryTexture)
			{
				pWRModel->ExposeParameter("g_sLiveryTexture", RenderingData::MATERIAL_ELEMENT_TYPE_TEXTURE_PTR, 1);
				pWRModel->ExposeParameter("g_vLiverySettings", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);				
			}
			pWRModel->ExposeParameter("g_sVehicleDamageVolumeTexture", RenderingData::MATERIAL_ELEMENT_TYPE_TEXTURE_PTR, 1);
			pWRModel->ExposeParameter("g_sVehicleDamageInfoTexture", RenderingData::MATERIAL_ELEMENT_TYPE_TEXTURE_PTR, 1);
			//REMOVE_DIRT
			//pWRModel->ExposeParameter("g_sVehicleDirtLayerTexture", RenderingData::MATERIAL_ELEMENT_TYPE_TEXTURE_PTR, 1);
			pWRModel->ExposeParameter("g_sLightEnvironmentTexture", RenderingData::MATERIAL_ELEMENT_TYPE_TEXTURE_PTR, 1);
			
			pWRModel->ExposeParameter("g_VehicleVtxParams", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 12+12+4+4);
			//pWRModel->ExposeParameter("g_VehiclePixParams", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4+4+4);
			//REMOVE_DIRT
			pWRModel->ExposeParameter("g_VehiclePixParams", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);

			pWRModel->ExposeParameter("g_vUndercoatColour", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);
			pWRModel->ExposeParameter("g_vScuffColour", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);
			
			pWRModel->ExposeParameter("g_vControlPoints", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 72);

			// for each element inside this model
			for (int iElement=0; iElement<pModel->GetElementCount(); iElement++)
			{
				GameResource::C_ModelElement *pElement = pModel->GetElement(iElement);

				Util::C_String elementName = pElement->GetName();
				elementName.ConvertToLower();

				//Tyre
				if (elementName.Search("e_tyre") != -1)
				{
					pWRModel->ExposeParameter(pElement->GetIndex(), "g_vRenderSettings", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);
				}
				//Glass
				else if (elementName.Search("e_window") != -1 || elementName.Search("e_headlightcover") != -1)
				{
					pWRModel->ExposeParameter(pElement->GetIndex(), "g_vRenderSettings", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);
					pWRModel->ExposeParameter(pElement->GetIndex(), "g_vRenderColour", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);
					pWRModel->ExposeParameter(pElement->GetIndex(), "g_vDamageSettings", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);
				}
				//Alloy
				else if (elementName.Search("e_alloyrim") != -1 || elementName.Search("e_wheellod") != -1)
				{
					pWRModel->ExposeParameter(pElement->GetIndex(), "g_vRenderSettings", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);
					pWRModel->ExposeParameter(pElement->GetIndex(), "g_vRenderColour", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);
				}
				//Light
				else if (elementName.Search("e_headlight") != -1 || elementName.Search("e_rearlight") != -1 ||
						 elementName.Search("e_foglight") != -1 || elementName.Search("e_brakelight") != -1 ||
						 elementName.Search("e_reverselight") != -1 || elementName.Search("e_runninglight") != -1 ||
						 elementName.Search("e_signallight") != -1 || elementName.Search("e_rearandbrakelight") != -1 ||
						 elementName.Search("e_licenseplate") != -1)
				{
					pWRModel->ExposeParameter(pElement->GetIndex(), "g_vRenderColour", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);
					pWRModel->ExposeParameter(pElement->GetIndex(), "g_vDamageSettings", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);
				}
			}
		}
	}
}

/* 
===============================================================
					class C_ModelTypeWheel

NOTES: Wheel model type for dynamic material setting
===============================================================
*/

/*
===============
NAME: OnLoad

NOTE: Exposes parameters
===============
*/
void C_ModelTypeWheel::OnLoad(GameResource::C_ModelResource* p_pModelResource)
{
	if (!p_pModelResource)
		return;

	GameResource::C_ModelData *pModelDesc = p_pModelResource->GetModelData();
	WorldRender::C_RenderModelData *pGRModel = p_pModelResource->GetRenderModelData();

	// for each model inside the file
	for (int iModel = 0; iModel < pModelDesc->GetModelCount(); iModel++)
	{
		GameResource::C_Model *pModel = pModelDesc->GetModel(iModel);
		WorldRender::C_Model *pWRModel = &p_pModelResource->GetRenderModelData()->GetModel(iModel);

		pWRModel->ExposeParameter("g_sLightEnvironmentTexture", RenderingData::MATERIAL_ELEMENT_TYPE_TEXTURE_PTR, 1);

		pWRModel->ExposeParameter("g_VehicleVtxParams", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 12+12+4+4);
		//REMOVE_DIRT
		//pWRModel->ExposeParameter("g_VehiclePixParams", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4+4+4);
		pWRModel->ExposeParameter("g_VehiclePixParams", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);

		// for each element inside this model
		for (int iElement=0; iElement<pModel->GetElementCount(); iElement++)
		{
			GameResource::C_ModelElement *pElement = pModel->GetElement(iElement);
			
			pWRModel->ExposeParameter(pElement->GetIndex(), "g_vRenderSettings", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);
			pWRModel->ExposeParameter(pElement->GetIndex(), "g_vRenderColour", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);
		}
	}
}

} //namespace Vehicle

