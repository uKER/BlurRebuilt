/////////////////////////////////////////////////////////////
// VehicleModel
/////////////////////////////////////////////////////////////

#ifndef __VEHICLE_MODEL_H__
#define __VEHICLE_MODEL_H__

#include "Vehicle/Input.h"
#include "Vehicle/Setup.h"
#include "Vehicle/Component.h"
#include "Vehicle/Customisation/PaintEditor.h"
#include "Vehicle/Effects/Wheels.h"
#include "Vehicle/Effects/Lights.h"
#include "Vehicle/Effects/DirtLayer.h"

#ifdef PLATFORM_PS3
#include "Vehicle/damage/spu/SPUDamageContext.h"	// Only want the damage context on PS3.
#endif // #ifdef PLATFORM_PS3

#include <Resource/Resource.h>
#include <game_resource/ModelResource.h>
#include <game_resource/ModelData.h>

namespace WorldRender
{
	class C_LightSampler;
	class C_ModelInstance;
	class C_Model;
	struct MaterialParameter;
}

namespace SceneGraph
{
	class C_VisibleDetect;
}

namespace Vehicle
{

class C_RenderLayer;
class C_GameLayer;
class C_DamageLayer;
class C_MessageHandler;
class C_VehicleModelSetup;
struct XtVehicle;
struct XtMaterialDefaults;
enum E_PartCode;
enum E_PartSubCode;

struct VehicleVtxParams
{
	MathsStorage::Float43	mVehicleMatrix;
	MathsStorage::Float43	mVehicleInverseMatrix;
	MathsStorage::Float4	VehicleBoundsInfo;
	MathsStorage::Float4	RearLightColour;
	MathsStorage::Float44	VehicleSHNode;
};

struct VehiclePixParams
{
	MathsStorage::Float4	vAlphaScale;
	//REMOVE_DIRT
	//MathsStorage::Float4	vDirtBaseLayerSettings;
	//MathsStorage::Float4	vDirtDetailLayerSettings;
};

enum E_ModelInstanceType
{
	E_ModelInstanceType_None,

	E_ModelInstanceType_Exterior,
	E_ModelInstanceType_Interior,
	E_ModelInstanceType_Wheels,
	E_ModelInstanceType_Shadow,

	E_ModelInstanceType_Max,
};

enum E_CachedParamName
{
	//REMOVE_DIRT
	//E_CachedParamName_DirtBaseLayerSetting,
	//E_CachedParamName_DirtDetailLayerSetting,
	E_CachedParamName_VehicleDamageVolumeTexture,
	E_CachedParamName_VehicleDamageInfoTexture,
	E_CachedParamName_PaintMaterialTexture,
	//REMOVE_DIRT
	//E_CachedParamName_VehicleDirtLayerTexture,
	E_CachedParamName_VehicleVtxParams,
	E_CachedParamName_VehiclePixParams,
	E_CachedParamName_DebugDamageSettings,
	E_CachedParamName_DamageColour,
	E_CachedParamName_UndercoatColour,
	E_CachedParamName_ScuffColour,
	E_CachedParamName_SHNode,
	E_CachedParamName_LightEnvironmentTexture,
	E_CachedParamName_LiveryTexture,
	E_CachedParamName_AlphaScale,
	E_CachedParamName_LiverySettings,
	E_CachedParamName_VehicleSHNode,
	E_CachedParamName_BackLeftSHNode,
	E_CachedParamName_FrontLeftSHNode,
	E_CachedParamName_BackRightSHNode,
	E_CachedParamName_FrontRightSHNode,
	E_CachedParamName_Max
};

typedef Btl::Vector<WorldRender::MaterialParameter, Btl::VectorParameters_Array<E_CachedParamName_Max> > CachedModelParameters;


struct C_VehicleVertexBuffer
{
	C_VehicleVertexBuffer();

	bool									m_Used;
	Display::C_VertexBuffer					m_Data;
};

class C_VehicleVertexBufferManager
{
public:

	C_VehicleVertexBufferManager();

	C_VehicleVertexBuffer* CreateVertexBuffer(const U32 size);
	void DestroyVertexBuffer(C_VehicleVertexBuffer* pVertexBuffer);
	void DestroyVertexBuffers();

	static C_VehicleVertexBufferManager& GetInstance()
	{
		return s_Instance;
	}

private:

	C_VehicleVertexBuffer* FindVertexBuffer(const U32 size);
	C_VehicleVertexBuffer* FindVertexBuffer(C_VehicleVertexBuffer* pVertexBuffer);

private:

	#if !CFG_GOLD
	S32										m_Size;
	#endif
	Btl::Set<C_VehicleVertexBuffer*>		m_List;
	static C_VehicleVertexBufferManager		s_Instance;
};

/* 
===============================================================
					class C_VehicleModel

NOTES: 
===============================================================
*/
class C_VehicleModel
{
public:

	C_VehicleModel();

	~C_VehicleModel();

	//! Create models and add them to scene graph and vehicle component list
	void CreateModels();

	//! Creates model
	bool Create(const C_VehicleModelSetup* p_pModelSetup);

	//! Initialises model
	void Initialise(XtVehicle* p_pVehicle);

	//! Destroys model
	bool Destroy();

	//! Loads the model
	bool Load(const C_VehicleModelSetup* p_ModelSetup);

	//! Updates model
	void Update(float p_FrameTime = 0.0f);

	//! Updates debug stuff about the model
	void Update_Debug();

	//! Update light sampler
	void UpdateLightSampler();

	//! Update deferred
	void UpdateDeferred();

	//! Updates all dynamic materials for the parts
	void UpdateDynamicMaterials(XtVehicle* p_pVehicle, XtMaterialDefaults* p_pMaterialDefaults);

	//! Updates lights
	void UpdateLights();

	//! Update HeadLight Matrix
	void UpdateHeadLightMatrix();

	//! Render placeholder
	void Render();

	//! Return pointer to the render layer
	C_RenderLayer* GetRenderLayer() { return m_pRenderLayer; }

	//! Return pointer to the game layer
	C_GameLayer* GetGameLayer() { return m_pGameLayer; }

	//! Return pointer to the damage layer
	C_DamageLayer* GetDamageLayer() { return m_pDamageLayer; }

	//! Return pointer to the message handler
	C_MessageHandler* GetMessageHandler() { return m_pMessageHandler; }

	//! Returns pointer to the model input structure
	C_ModelInput* GetModelInput() { return &m_ModelInput; }

	//! Sets model resource pointer
	void SetModelResource(GameResource::C_ModelResource* p_pModelResource) 
	{ 
		m_pModelResource = p_pModelResource; 
		return;
	}
	
	void SetVehicleModelResource(GameResource::C_ModelResource* p_pModelResource) { m_pVehicleModelResource = p_pModelResource; }
	void SetWheelModelResource(GameResource::C_ModelResource* p_pModelResource) { m_pWheelModelResource = p_pModelResource; }

	//! Gets model resource pointer
	GameResource::C_ModelResource* GetModelResource() { return m_pModelResource; }

	//! Gets reference to model handle
	GameResource::C_ModelHandle& GetModelHandle() { return m_ModelHandle; }

	WorldRender::C_LightSampler* GetLightSampler() { return m_pLightSampler; }

	//! Sets component information
	C_Part*  SetComponent(	const GameResource::C_Model* p_pModel, 
									const GameResource::C_ModelElement* p_pModelElement,
									const GameResource::C_ModelResource* p_pModelResource,
									WorldRender::C_ModelInstance* p_pModelInstance,
									Gravity::C_Body* p_pBody,
									Gravity::C_Scene* p_pPhysicsScene,
									Maths::HMat43* p_pMatrix = NULL,
									E_PartCode p_PartCode = E_PartCode_None,
									E_PartSubCode p_LocationCode = E_PartSubCode_None);

	//! Gets component
	C_Part*  GetComponent(E_PartCode p_PartCode, E_PartSubCode p_PartSubCode);

	C_PartList& GetComponentList() { return m_Parts; }

	//! Sets up dynamic material stuff
	void SetupDynamicMaterials(XtVehicle* p_pVehicle, 
							   XtMaterialDefaults* p_pMaterialDefaults,
							   GameResource::C_ModelResource* p_pModelResource);
	
	//! Sets up dynamic material stuff for the wheels
	void SetupDynamicMaterialsWheel(XtVehicle* p_pVehicle, 
									XtMaterialDefaults* p_pMaterialDefaults);
	//! Sets up a dynamic material
	void SetupDynamicMaterial(	XtVehicle* p_pVehicle, 
								XtMaterialDefaults* p_pMaterialDefaults,
								WorldRender::C_Model* p_pWRModel, 
								GameResource::C_ModelElement* p_pElement);

	//! Sets material handle and default values
	void SetMaterial(XtVehicle* p_pVehicle, 
					 XtMaterialDefaults* p_pMaterialDefaults,
					 Util::C_String p_ElementName, 
					 const WorldRender::MaterialParameter& p_Handle,
					 Util::C_String p_HandleName);

	//! Sets mateiral handle and default values
	void SetMaterialExplicit(	XtVehicle* p_pVehicle, 
								XtMaterialDefaults* p_pMaterialDefaults,
								E_PartCode p_PartCode,
								E_PartSubCode p_SubCode);

	//! Set all material parameters that don't change per-frame
	void SetStaticShaderParameters();

	//! Set all material parameters that can change per-frame
	void SetDynamicShaderParameters(bool force = false);

	//! Set sh material parameters that can change per-frame
	void SetDynamicSHLightShaderParameters(const F32 additiveSHLightData[16], const Maths::HMat43& matrix, const MathsStorage::Float4& rearLightColour);

	//! Set visible
	void SetVisDetect(const SceneGraph::C_VisibleDetect* p_pVisDetect) { m_pVisDetect = p_pVisDetect; }

	//! Adds wheel blur
	void AddWheelBlur(C_Part* p_pUnblurredSpokes, C_Part* p_pBlurredSpokes);

	void SetPartVisibility(E_PartCode p_PartCode, bool p_Visible);

	//! Model instance add/get stuff
	void AddModelInstance(WorldRender::C_ModelInstance* p_pModelInstance, Util::C_String p_Name, S32 wheelIndex = -1);
	WorldRender::C_ModelInstance* GetModelInstance(S32 p_Index) { return p_Index >= m_pModelInstances.Size() ? NULL : m_pModelInstances[p_Index].m_pModelInstance; }
	E_ModelInstanceType GetModelInstanceType(S32 p_Index) { return p_Index >= m_pModelInstances.Size() ? E_ModelInstanceType_None : m_pModelInstances[p_Index].m_eType; }
	void GetModelInstanceName(S32 p_Index, Util::C_String& p_Name) { if (p_Index < m_pModelInstances.Size()) p_Name = m_pModelInstances[p_Index].m_Name; }
	U32 GetNumModelInstances() { return (U32)m_pModelInstances.Size(); }

#ifdef PLATFORM_PS3
	//! Return a vector of damage offset vertex buffers.
	const Btl::Vector<C_VehicleVertexBuffer*>& GetDamageOffsetVertexBuffers() const
	{
		return m_damageOffsets;
	}
	
#endif //PLATFORM_PS3
	
	//! Gets paint job
	C_PaintJob*	GetPaintJob() { return &m_PaintJob; }

	C_Headlight* GetHeadlight() { return &m_Headlight; }
	C_DynamicLight* GetHeroLight() { return &m_HeroLight; }

	C_LightFlares* GetLightFlares() { return &m_LightFlares; }
	C_DynamicLights* GetDynamicLights() { return &m_DynamicLights; }
	C_AuxFlares* GetAuxFlares() { return &m_AuxFlares; }

	//REMOVE_DIRT
	//C_DirtLayer* GetDirtLayer() { return &m_DirtLayer; }

	void	SetLightEnvironmentTexture(Display::C_TextureCube* pLightEnvironmentTexture);

	void	SetLiveryTexture(Display::C_Texture2D* pLiveryTexture);

	void	SetAlpha(const F32 alpha) { m_Alpha = alpha; }
	float	GetAlpha() { return m_Alpha; }

	const Maths::C_RGBA_F32& GetLighting() const { return m_Lighting; } 

	void Freeze(bool p_bFreeze);

	void Reset();
	void ResetInitialStates();

	bool						m_Hidden;
	bool						m_FocusVehicle;

	void						EnableVisibilityFlags(U32 visibilityFlags);
	void						DisableVisibilityFlags(U32 visibilityFlags);

	Btl::Vector<C_WheelBlur>	m_WheelBlur;	

	F32							GetHeadLightIntensity() const { return m_Parts.m_HeadLightIntensity;}
	F32							GetRearLightIntensity() const { return m_Parts.m_RearLightIntensity;}
	F32							GetBreakLightIntensity() const { return m_Parts.m_BreakLightIntensity;}
	F32							GetReverseLightIntensity() const { return m_Parts.m_ReverseLightIntensity;}

	void						SetFrameTime(const F32 frameTime) { m_FrameTime = frameTime;} 
private:
	void UpdateLight(int iPartCode, int *piLightVisibile, bool bVisible, bool bBaseVisible = false);

	WorldRender::MaterialParameter GetParameter(int iModel, int iParam) { return m_CachedParameters[iModel][iParam]; }
	void CacheParameters(int i, WorldRender::C_ModelInstance* pInstance);

	Maths::HMat43				m_Matrix;

	C_RenderLayer*				m_pRenderLayer;						//!< Everything rendering related. Doesn't need to know about anything else
	C_GameLayer*				m_pGameLayer;						//!< Handles the game-side logic and updating
	C_DamageLayer*				m_pDamageLayer;						//!< Handles damage processing

	C_MessageHandler*			m_pMessageHandler;					//!< Handles messaging between layers

	C_ModelInput				m_ModelInput;						//!< Collates data from the vehicle physics
	C_VehicleModelSetup			m_ModelSetup;

	GameResource::C_ModelHandle					m_ModelHandle;
	GameResource::C_ModelResource*				m_pModelResource;
	GameResource::C_ModelResource*				m_pVehicleModelResource;
	GameResource::C_ModelResource*				m_pWheelModelResource;
	
	struct RenderModelInstance
	{
		RenderModelInstance(WorldRender::C_ModelInstance* p_pModelInstance, Util::C_String p_Name, U32 visibilityFlagMask)
		{
			m_pModelInstance = p_pModelInstance;
			m_Name = p_Name;
			m_VisibilityFlagMask = visibilityFlagMask;
		}
		WorldRender::C_ModelInstance*	m_pModelInstance;
		Util::C_String					m_Name;
		E_ModelInstanceType				m_eType;
		U32								m_VisibilityFlagMask;
		S32								m_WheelIndex;
	};
	
#ifdef PLATFORM_PS3
	Btl::Vector<C_VehicleVertexBuffer*>		
										m_damageOffsets;		//!< Vertex double buffers for damage offsets.
#endif // PLATFORM_PS3

	Ut::Vector<RenderModelInstance>		m_pModelInstances;
	const SceneGraph::C_VisibleDetect*	m_pVisDetect;

	bool						m_Initialised;
	
	C_PartList					m_Parts;		//!< Render node to component mapping
	int							m_iBrakeLightsVisible, m_iHeadLightsVisible, m_iRearLightsVisible, m_iReverseLightsVisible, m_iFogLightsVisible, m_iRearAndBrakeLightsVisible;

	C_PaintJob					m_PaintJob;

	C_Headlight					m_Headlight;
	C_DynamicLight				m_HeroLight;

	WorldRender::C_LightSampler* m_pLightSampler;

	C_LightFlares				m_LightFlares;
	C_DynamicLights				m_DynamicLights;
	C_AuxFlares					m_AuxFlares;
	//REMOVE_DIRT
	//C_DirtLayer					m_DirtLayer;

	Btl::Vector<CachedModelParameters>	m_CachedParameters;

	Maths::C_RGBA_F32			m_Lighting;

	F32							m_Alpha;

	F32							m_FrameTime;
};

} //namespace Vehicle

#endif
