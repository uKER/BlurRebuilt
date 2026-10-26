/////////////////////////////////////////////////////////////
// DamageLayer
/////////////////////////////////////////////////////////////

#ifndef __VEHICLE_DAMAGE_LAYER_H__
#define __VEHICLE_DAMAGE_LAYER_H__

#include "Vehicle/Layer.h"
#include "Vehicle/Damage/CollisionInfo.h"
#include "Vehicle/Utils/Curve.h"
#include "Vehicle/PartCodes.h"
#ifdef PLATFORM_PS3
#include "Vehicle/Damage/Spu/SPUDamageContext.h"
#endif // #ifdef PLATFORM_PS3
#include "Vehicle/Damage/NodeLayer.h"
#include "Vehicle/Damage/Node.h"

namespace Gravity { class C_Scene; }
namespace Gravity { class C_Body; }
namespace RacingPhysics { class Car; }

namespace Vehicle
{

struct XtVehicle;
struct XtDeformationSetup;
struct XtWheelDamageSetup;
class C_PartList;
class C_Part;
class C_VehicleModel;

enum E_DefferedEvent
{
	E_DefferedEvent_Unknown = -1,

	E_DefferedEvent_BurstTyre,
	E_DefferedEvent_WindowCrack,
	E_DefferedEvent_WindowSmash,
	E_DefferedEvent_LightCrack,
	E_DefferedEvent_LightSmash,
	E_DefferedEvent_ScrapeStatic,
	E_DefferedEvent_ScrapeDynamic,
	E_DefferedEvent_PartOpened,
	E_DefferedEvent_PartClosed,
	E_DefferedEvent_LicensePlateFront,
	E_DefferedEvent_LicensePlateBack,

	E_DefferedEvent_Max,
};



/* 
===============================================================
					class C_DefferedEvent

NOTES: 
===============================================================
*/
class C_DefferedEvent
{
public:

	C_DefferedEvent(E_DefferedEvent p_Event, Maths::HVec3 p_Position, Maths::HVec3 p_Direction, Maths::HVec3 p_PointVelocity, float p_Scale, E_PartCode p_PartCode, E_PartSubCode p_PartSubCode)
	{
		m_Event			= p_Event;
		m_Position		= p_Position;
		m_Direction		= p_Direction;
		m_PointVelocity	= p_PointVelocity;
		m_Scale			= p_Scale;
		m_PartCode		= p_PartCode;
		m_PartSubCode	= p_PartSubCode;
	}

public:

	E_DefferedEvent			m_Event;

	E_PartCode				m_PartCode;
	E_PartSubCode			m_PartSubCode;
	Maths::HVec3			m_Position;
	Maths::HVec3			m_Direction;
	Maths::HVec3			m_PointVelocity;

	float					m_Scale;
};

struct DeformRange;

typedef Ut::UtVector<Ut::DynamicVector<Vehicle::C_NodeLayer> >	V_Layers;

#define NUM_DAMAGE_TEXTURES 2

#define _DAMAGE_USE_3D_TEX

/* 
===============================================================
					class C_DamageLayer

NOTES: Handles damage processing
===============================================================
*/

class C_WheelDamage
{
public:

	C_WheelDamage()
	{
		m_OffsetVector		= Maths::HVec3(0.0f, 0.0f, 0.0f);
		m_OriginalPosition	= Maths::HVec3(0.0f, 0.0f, 0.0f);
		m_pNode				= NULL;
		m_pMinNode			= NULL;
		m_pMaxNode			= NULL;
		m_Index				= -1;
		m_OffsetLength		= 0.0f;
		m_Radius			= 0.0f;
		m_RadiusScale		= 0.0f;
		m_GripScale			= 0.0f;
		m_Grip				= 0.0f;
		m_FrictionScale		= 0.0f;
		m_Friction			= 0.0f;
		m_TyreSize			= 0.0f;
		m_Detached			= false;
		m_TyreBurst			= false;
#ifndef CFG_GOLD
		m_HackTyreBurst		= false;
		m_HackWheelLoss		= false;
#endif
	}

	Maths::HVec3		m_OffsetVector;
	Maths::HVec3		m_OriginalPosition;
	C_Node*				m_pNode;
	C_Node*				m_pMinNode;
	C_Node*				m_pMaxNode;
	S32					m_Index;
	float				m_OffsetLength;
	float				m_RadiusScale;
	float				m_Radius;
	float				m_GripScale;
	float				m_Grip;
	float				m_FrictionScale;
	float				m_Friction;
	float				m_SeparationLength;
	float				m_TyreSize;
	bool				m_Detached;
	bool				m_TyreBurst;
#ifndef CFG_GOLD
	bool				m_HackTyreBurst;
	bool				m_HackWheelLoss;
#endif
};

class C_DamageState
{
public:
	
	C_DamageState() 
	{
		Clear()	;
	}

	void Clear()
	{
		//memset(m_State, 0, sizeof(DWORD) * 64);
		for (S32 i = 0; i < 64; i++)
		{	
			m_State[i].m_DX				= 16;
			m_State[i].m_DZ				= 32;
			m_State[i].m_Scratch		= 0;
			m_State[i].m_Deform			= 0;
			m_State[i].m_PartCode		= 0;
			m_State[i].m_PartSubCode	= 0;
			m_State[i].m_Hinged			= 0;
			m_State[i].m_Detached		= 0;
			m_State[i].m_Level			= 0;
		}

		m_GamePlayPoints		= 0;
		m_Padding				= 0;
	}

public:

	//Btl::Pair<E_PartCode, E_PartSubCode>	m_PartMapping;

	struct State
	{
		U32		m_DX			: 5;	//!< Node offset in X
		U32		m_DZ			: 6;	//!< Node offset in Z
		U32		m_Scratch		: 3;	//!< Scratch level
		U32		m_Deform		: 3;	//!< Deformation level
		U32		m_PartCode		: 5;	//!< ID/index of the part
		U32		m_PartSubCode	: 4;	//!< ID/index of the part
		U32		m_Hinged		: 1;	//!< If hinge is active
		U32		m_Detached		: 1;	//!< If currently detached
		U32		m_Level			: 4;	//!< Current damage level of part
	};

	Btl::Vector<State, Btl::VectorParameters_Array<64> > m_State;
	struct
	{
		U32		m_GamePlayPoints	: 7;
		U32		m_Padding			: 25;
	};
};

struct HullVertex
{
	HullVertex() : m_pVertex(NULL), m_pNode(NULL) {}

	void SetVertex(C_Node* p_pNode, MathsStorage::Fixed3_S16<14>* p_pVertex)
	{
		if (!p_pNode || !p_pVertex)
			return;

		m_pNode		= p_pNode;
		m_pVertex	= p_pVertex;

		//m_Original = Maths::Load<Maths::HVec3>(m_pVertex);
		m_Original = Maths::HVec3(m_pVertex);
		m_Original = m_Original * m_Range + m_Low;
	}

	void UpdateVertex()
	{
		if (!m_pNode || !m_pVertex)
			return;

		Maths::HVec3 vertex = m_Original + m_pNode->m_OffsetVector;
		vertex = (vertex - m_Low) / m_Range;
		*m_pVertex = (MathsStorage::Fixed3_S16<14>)vertex;
	}

	void ResetVertex()
	{
		if (!m_pNode || !m_pVertex)
			return;

		Maths::HVec3 vertex = m_Original;
		vertex = (vertex - m_Low) / m_Range;
		*m_pVertex = (MathsStorage::Fixed3_S16<14>)vertex;
	}

	MathsStorage::Fixed3_S16<14>*	m_pVertex;
	Maths::HVec3					m_Original;
	Maths::HVec3					m_Low;
	Maths::HVec3					m_Range;
		
	C_Node*							m_pNode;
};

class C_DamageLayer : public C_Layer
{
public:

	explicit C_DamageLayer(C_VehicleModel* pParentModel);

	~C_DamageLayer();

	//! Creates layer
	bool Create(U32 vehicleIndex);

	//! Sets initial damage levels
	void SetInitialDamageLevel(float p_ScratchLevel, float p_DamageLevel)
	{
		m_InitialScratchLevel	= p_ScratchLevel;
		m_InitialDamageLevel	= p_DamageLevel;
	}

	//! Setup nodes
	void Setup(XtVehicle* p_pXtVehicle);

	//! Destroys layer
	bool Destroy();

	//! Updates layer
	void Update();

	//! Updates damage textures
	void UpdateDamageTextures();

	//! Resets the damage volume
	void ResetVolume();

	//! Creates damage volume
	bool CreateVolume();

	//! Creates volume texture
	bool CreateVolumeTexture(U32 vehicleIndex);

	//! Maps collision shape vertices to lattice
	void MapCollisionVertices(Gravity::C_Body* p_pCollisionBody);	

	//! Updates volume texture
	void UpdateVolumeTexture(bool reset = false);
	void UpdateInfoTexture(bool reset = false);

	//! Setup wheel damage information
	void SetupWheelDamage(XtWheelDamageSetup* p_pSetup);

	//! Setup node damage information
	void SetupDeformRanges(XtDeformationSetup* p_pSetup);

	//! Setup node damage information
	void SetupDeformRange(C_Node* p_pNode, S32 p_Layer);
	
	//! Renders deformation zone information
	void RenderDefomationInformation(Maths::HMat43 p_Matrix);

	//! Clamps node to min/max allowable position
	void ClampNodePosition(C_Node* p_pNode, Maths::HVec3 p_Position);

	//! Calculates how far node can move
	void CalculateNodeExtents(S32 p_Layer, C_Node* p_pNode, float& p_X, float& p_Y, float& p_Z, int p_Stage, Maths::HVec3 p_Position);

	//! Processes collisions from the vehicle actor
	C_ImpactEvent*  ProcessCollision(Maths::HMat43 p_Matrix, const CollisionInfo& p_CollisionInfo, const C_DamageFilter& p_Filter);

	//! Adds an impact event to the cache
	void AddImpactEvent(C_ImpactEvent p_ImpactEvent);

	//! Process any stored impact events
	void ProcessImpactEvents();
	void ProcessDeformEvents();

	//! Update the behaviour status of the damaged part
	void UpdateDamageBehaviour(C_DamageFilter* p_pFilter, C_Part* p_pPart, bool repairing = false);

	//! See if any parts need damaging
	void ProcessParts(C_ImpactEvent* p_pEvent);

	//! Handles any damage dependancies
	E_DefferedEvent HandleDamageDependancies(E_DefferedEvent p_Event, C_Part* p_pPart, E_PartCode& p_PartCode, E_PartSubCode& p_PartSubCode);

	//! Deform the node lattice
	void Deform(C_ImpactEvent* p_pEvent, float p_Duration);
	
	//! Scrape the car
	void Scrape(C_ImpactEvent* p_pEvent);

	//! Update transition
	void UpdateTransition(float p_TimeStep);

	//! Repairs a part
	void RepairPart(E_PartCode p_PartCode, E_PartSubCode p_PartSubCode, bool repairAll = false);

	//! Repair
	void Repair(float p_Amount, float p_CurrentDamagePercentage, float p_Duration);

	//! Draws debug information
	void DebugDraw();

	//! Return the index of the closest node to a given position
	void GetClosestNodeIndex(	Maths::HVec3 p_Position, 
								S32& p_LayerIndex, S32& p_RowIndex, S32& p_ColIndex, 
								bool clampToEdge = false);

	//! Return the closest node to a given position
	C_Node* GetClosestNode(	Maths::HVec3 p_Position);

	//! Sets up damage parts from Sorcery
	void SetupDamagedParts(	XtVehicle* p_pVehicle, 
							Gravity::C_Body* p_pCollisionBody, 
							Gravity::C_Scene* p_pPhysicsScene,
							Maths::HMat43 p_VehicleMatrix);

	//! Edit/place constraints
	void EditConstraints(XtVehicle* p_pVehicle, Gravity::C_Body* p_pCollisionBody, Gravity::C_Scene* p_pPhysicsScene);

	//! Updates damaged state of wheels
	void UpdateWheelDamage(C_ImpactEvent* p_pEvent);
	
	//! Bursts a tyre
	bool BurstTyre(C_WheelDamage* p_pWheel);
	
	//! Bursts a tyre
	bool RepairTyre(C_WheelDamage* p_pWheel);
	
	//! Knocks wheel off
	void DetachWheel(C_WheelDamage* p_pWheel);

	//! Puts wheel back on
	void ReattachWheel(C_WheelDamage* p_pWheel);

	//! Adds a damage effect for later processing
	void AddDefferedDamageEffect(E_DefferedEvent p_Event, Maths::HVec3 p_Position, Maths::HVec3 p_Direction, float p_Scale, E_PartCode p_PartCode = E_PartCode_None, E_PartSubCode p_PartSubCode = E_PartSubCode_None);
	void AddDefferedDamageEffect(E_DefferedEvent p_Event, Maths::HVec3 p_Position, Maths::HVec3 p_Direction, Maths::HVec3 p_PointVelocity, float p_Scale, E_PartCode p_PartCode = E_PartCode_None, E_PartSubCode p_PartSubCode = E_PartSubCode_None);
	U32 GetNumDefferedEvents() { return (U32)m_DefferedEvents.Size(); }
	C_DefferedEvent* GetDefferedEvent(U32 p_Event) { return p_Event < GetNumDefferedEvents() ? &m_DefferedEvents[p_Event] : NULL; }
	void ClearDefferedEvents() { m_DefferedEvents.Clear(); }

	C_WheelDamage* GetWheel(U32 p_Index) { return int(p_Index) < m_WheelNodes.Size() ? &m_WheelNodes[p_Index] : NULL; }

	//! Sets vehicle info
	C_VehicleInfo* GetVehicleInfo() { return &m_VehicleInfo; }

#ifdef _DAMAGE_USE_3D_TEX
	//! Return pointer to damage volume texture
	Display::C_Texture3D* GetDamageVolumeTexture() { return m_DamageVolumeTextures[m_CurrentTexture]; }
#else
	//! Return pointer to damage volume texture
	Display::C_Texture2D* GetDamageVolumeTexture() { return m_DamageVolumeTextures[m_CurrentTexture]; }
#endif
	Display::C_Texture2D* GetDamageInfoTexture() { return m_DamageInfoTextures[m_CurrentInfoTexture]; }

	void SwapTextures() { m_CurrentTexture++; if (m_CurrentTexture >= NUM_DAMAGE_TEXTURES) m_CurrentTexture = 0; }
	void SwapInfoTextures() { m_CurrentInfoTexture++; if (m_CurrentInfoTexture >= NUM_DAMAGE_TEXTURES) m_CurrentInfoTexture = 0; }

	void SetUpdateVolumeTexture(bool p_Update);
	void SetUpdateInfoTexture(bool p_Update);

	void SetPartListPtr(C_PartList* p_pPartList) { m_pPartList = p_pPartList; }

	void SetPhysicsScene(Gravity::C_Scene* p_pPhysicsScene) { m_pPhysicsScene = p_pPhysicsScene; }

	void SetVehiclePhysics(RacingPhysics::Car* p_pVehiclePhysics) { m_pVehiclePhysics = p_pVehiclePhysics; }

#ifdef PLATFORM_PS3
	//! Swap the buffers for the node layers.
	void SwapBuffers()
	{
		m_BufferValue = ((m_BufferValue+1)&0x1);
		memcpy(&m_DamageVolume[m_BufferValue][0], &(*m_pDamageVolume)[0], sizeof(Vehicle::C_NodeLayer)*3);		// copy the double buffer.
		m_pDamageVolume = &m_DamageVolume[m_BufferValue];
		return;
	}
#endif // #ifndef PLATFORM_PS3

	void SetFatalDamageFlag(bool p_Fatal) { m_FatalDamage = p_Fatal; }
	bool FatallyDamaged() { return m_FatalDamage; }

	S32 GetNumLayers() { return m_pDamageVolume->Size(); }
	S32 GetNumWidth(S32 p_Layer) { return (p_Layer >= 0 && p_Layer < m_pDamageVolume->Size()) ? (*m_pDamageVolume)[p_Layer].GetWidth() : 0; }
	S32 GetNumLength(S32 p_Layer) { return (p_Layer >= 0 && p_Layer < m_pDamageVolume->Size()) ? (*m_pDamageVolume)[p_Layer].GetLength() : 0; }
	C_Node* GetNode(S32 p_Layer, S32 p_Row, S32 p_Column) { return (p_Layer < 0 || p_Layer >= m_pDamageVolume->Size()) ? NULL : (*m_pDamageVolume)[p_Layer].GetNode(p_Row, p_Column); }

	void						SaveDamageState(C_DamageState* p_pState);
	void						RestoreDamageState(const C_DamageState* p_pState);
	C_DamageState*				GetDamageState() { return &m_DamageState; }

#if PLATFORM_PS3
	void InitSPUDamage(MathsStorage::Float4 *pData);
#endif

	void ForceDamageContextToRun();

private:

	void FinaliseSPUDamageJob();

	// Disallow calls to this.
	explicit C_DamageLayer();
	
	struct DeformSetup
	{
		float m_FrontCrumpleZone;
		float m_RearCrumpleZone;
		float m_SideCrumpleZone;
		float m_FrontNoDamageZone;
		float m_RearNoDamageZone;
		float m_SideNoDamageZone;
		float m_FrontYMovement;
		float m_RearYMovement;
		float m_SideYMovement;
		bool m_Valid;
	};
	DeformSetup m_DeformSetup;

	struct WheelSetup
	{
		float m_FrontTyreLossRadiusScale;
		float m_RearTyreLossRadiusScale;
		float m_FrontWheelLossRadiusScale;
		float m_RearWheelLossRadiusScale;
	};
	WheelSetup m_WheelSetup;
	
	C_HermiteCurve				m_WidthCurve;
	C_HermiteCurve				m_LengthCurve;
	C_HermiteCurve				m_HeightCurve;
	float						m_HeightRange;
	float						m_ZOffset;

	C_VehicleInfo				m_VehicleInfo;					//!< Information about the vehicle that the damage layer needs to know

#ifdef _DAMAGE_USE_3D_TEX
	Btl::Vector<Display::C_Texture3D*, Btl::VectorParameters_Array<NUM_DAMAGE_TEXTURES> > m_DamageVolumeTextures;	//!< The textual representation of the damage volume for use in the vertex shaders
#else
	Btl::Vector<Display::C_Texture2D*, Btl::VectorParameters_Array<NUM_DAMAGE_TEXTURES> > m_DamageVolumeTextures;	//!< The textual representation of the damage volume for use in the vertex shaders
#endif
	Btl::Vector<Display::C_Texture2D*, Btl::VectorParameters_Array<NUM_DAMAGE_TEXTURES> > m_DamageInfoTextures;

	U8							m_CurrentTexture;
	U8							m_CurrentInfoTexture;

	bool						m_FatalDamage;
	bool						m_UpdateVolumeTexture;			//!< Flag the volume texture to update
	bool						m_UpdateInfoTexture;			//!< Flag the info texture to update
	bool						m_Transitioning;
	bool						m_SlowMotionDamage;
	bool						m_CanBurstTyres;
	bool						m_CanLoseWheels;

	int							m_NextVolumeTextureUpdate;
	int							m_NextInfoTextureUpdate;

	float						m_TransitionPeriod;
	float						m_TransitionTime;
	float						m_InitialScratchLevel;
	float						m_InitialDamageLevel;
	
	Ut::UtVector<Ut::DynamicVector<Vehicle::C_NodeLayer> >*
								m_pDamageVolume;				//!< Pointer to the current damamge volume.
	U32							m_BufferValue;					//!< Double buffer value -- figures out which buffer we're using.
								
#ifdef PLATFORM_PS3
	Ut::UtVector<Ut::DynamicVector<Vehicle::C_NodeLayer> > 	ALIGN128				
								m_DamageVolume[2];				//!< The current state of the damage volume -- double buffer it on PS3.
#else
	Ut::UtVector<Ut::DynamicVector<Vehicle::C_NodeLayer> > 	ALIGN128				
								m_DamageVolume;					//!< The current state of the damage volume.
#endif

	C_PartList*					m_pPartList;

	Gravity::C_Scene*			m_pPhysicsScene;

	RacingPhysics::Car*			m_pVehiclePhysics;

	C_DamageState				m_DamageState;
	C_ImpactEvent				m_CachedDamageEvent;

	Btl::Vector<float>			m_HeightProfile;
	Btl::Vector<C_WheelDamage>	m_WheelNodes;
	Btl::Vector<C_ImpactEvent>	m_ImpactEvents;
	Btl::Vector<C_ImpactEvent>	m_CachedImpactEvents;
	Btl::Vector<C_DefferedEvent>	
								m_DefferedEvents;
	C_VehicleModel*				m_pParentModel;
	
#if defined(PLATFORM_PS3)
	SpuDamage::C_DamageContext* m_pSpuDamageContext;	
	static SpuDamage::C_ContextManager	
								m_contextManager;			//!< Context manager for SPU damage.
#endif // #if defined(PLATFORM_PS3)

	Btl::Vector<HullVertex>		m_CollisionShapeVertices;

	//Texture Resources
	static U32	ms_NumTextureResourcesVehicles;
	static Display::C_Texture3D ms_TextureVolumeResources[NUM_DAMAGE_TEXTURES][20];
	static Display::C_Texture2D ms_TextureInfoResources[NUM_DAMAGE_TEXTURES][20];

public:
	static bool CreateTextureResources(U32 numVehicles);
	static void DestroyTextureResources();
};

} //namespace Vehicle

#endif

