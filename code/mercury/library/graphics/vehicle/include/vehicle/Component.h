/////////////////////////////////////////////////////////////
// Part
/////////////////////////////////////////////////////////////

#ifndef __VEHICLE_PART_H__
#define __VEHICLE_PART_H__

#include "Vehicle/PartCodes.h"
#include "Vehicle/Db/VehicleXt.h"
#include "Vehicle/Damage/DamageLayer.h"

#include <gravity/narrowphase/CollisionFilter.h>
#include <gravity/sharedphysics/GraphNode.h>

#include <world_render/MaterialParameter.h>

namespace Gravity { class C_Body; }
namespace SharedPhysics {class C_Constraint; }
namespace Solver2 { class C_ConstraintGraphManager; }
namespace Gravity { class C_KinematicController; }
namespace Gravity { class C_Scene; }
namespace GameResource { class C_Model; class C_ModelElement; class C_ModelResource; }
namespace WorldRender { class C_ModelInstance; }
namespace SceneGraph { class C_VisibleDetect; }
namespace DynamicEffect { namespace Particle { class C_SpawnMesh; } }
namespace Vehicle
{

namespace Damage { class C_BehaviourBase; }

class XtConstraintBase;
class C_MaterialBase;
struct XtVehicle;
struct XtMaterialDefaults;
struct XtWindow;
class C_Part;
class C_ModelInput;
class C_Node;

/* 
===============================================================
					class C_ControlPoint

NOTES: 
===============================================================
*/
class C_ControlPoint
{
public:

	C_ControlPoint()
	{
		m_RestPosition	= Maths::HVec3(0.0f, 0.0f, 0.0f);
		m_Position		= Maths::HVec3(0.0f, 0.0f, 0.0f);
		m_MinExtents	= Maths::HVec3(0.0f, 0.0f, 0.0f);
		m_MaxExtents	= Maths::HVec3(0.0f, 0.0f, 0.0f);
	}

public:

	Maths::HVec3		m_RestPosition;
	Maths::HVec3		m_Position;
	Maths::HVec3		m_MinExtents;
	Maths::HVec3		m_MaxExtents;
};

/* 
===============================================================
					class C_ControlPointHandler

NOTES: 
===============================================================
*/
class C_ControlPointHandler
{
public:

	enum E_Part
	{
		E_Part_BumperFront,
		E_Part_BumperBack,
		E_Part_Bonnet,
		E_Part_Boot,
		E_Part_WingLeft,
		E_Part_WingRight,

		E_Part_Max,
	};

	struct Part
	{
		void Clear()
		{
			m_pNode						= NULL;
			m_pDamageNode				= NULL;

			m_OffsetDirection			= Maths::Constants<Maths::HVec3>::Zero(); //Maths::HVec3(0.0f, 0.0f, 0.0f);
			m_MaxMovement				= 0.0f;
			m_MovementRange				= 0.5f;
			m_MovementIndex0			= -1;
			m_MovementIndex1			= -1;
			m_AnchorIndex				= -1;
			m_ControlIndex				= -1;
		}

		Maths::HVec3					m_OffsetDirection;
		float							m_MaxMovement;
		float							m_MovementRange;
		S32								m_MovementIndex0;
		S32								m_MovementIndex1;
		S32								m_AnchorIndex;
		S32								m_ControlIndex;
		C_Part*							m_pNode;
		C_Node*							m_pDamageNode;
		Btl::Vector<C_ControlPoint, Btl::VectorParameters_Array<3> >	m_ControlPoints;
	};	

	C_ControlPointHandler();

	//! Adds a control point
	void AddControlPoint(Util::C_String p_Name, Maths::HVec3 p_Position, C_Part* p_pNode);

	//! Clear
	void Clear();

	//! Update
	void Update();

	//! Setup
	void Setup(C_Part* p_pNode);

	//! Finalise
	void Finalise(WorldRender::C_ModelInstance* p_pModelInstance);

	//! Updates shader parameters
	void SetParameters();

	//! Gets part
	Part* GetPart(C_Part* p_pPart);

public:
	
	WorldRender::C_ModelInstance*		m_pModelInstance;
	WorldRender::MaterialParameter		m_ParameterHandle;

	Btl::Vector<Part, Btl::VectorParameters_Array<E_Part_Max> >			m_Parts;

	MathsStorage::Float4				m_Data[E_Part_Max*3];
};

/* 
===============================================================
					class C_Part

NOTES: Vehicle part
===============================================================
*/
class C_Part
{
public:

	C_Part();

	//! Shuts down
	void Shutdown();

	//! Gets node visibility
	bool GetVisible(bool getFlag = false);

	//! Sets node visiblity
	void SetVisible(bool p_Visible, bool setRaw = false);

	//! Hides part
	void Hide(bool p_Hidden, bool raw = false);
	bool Hidden() { return m_Hidden; }

	//! Sets permanent visibility
	void SetPermanentVisibility(S8 p_Visibility) 
	{ 
		m_PermanentVisibility = p_Visibility;
		SetVisible(m_PermanentVisibility == 0 ? false : true);
	}

	//! Sets scale for the element
	void SetScale(Maths::HVec3 p_Scale) { m_Scale = p_Scale; }

	//! Activates the physics object
	void ActivatePhysicsObject(Maths::HVec3 p_Velocity, bool forceActive = false);

	//! Detaches object
	void Detach(Maths::HVec3 p_Velocity);

	//! Activates constraints
	void ActivateConstraints();

	//! Reattaches object
	void Reattach();

	//! Deactivates constraints
	void DeactivateConstraints();

	//! Deactivates the physics object
	void DeactivatePhysicsObject();

	//! Update vibration on part
	void UpdateVibration(float p_fNoise);

	//! Adds constraint to the node
	void AddConstraint(SharedPhysics::C_Constraint* p_pConstraint) { m_pConstraints.PushBack(p_pConstraint); }

	//! Sets shader parameter
	void SetShaderParameter(Util::C_String p_Name, MathsStorage::Float4 p_Values);

	void SetTargetDamageLevel(float p_TargetDamageLevel)
	{
		m_Damage.m_TargetDamageLevel = p_TargetDamageLevel; 
		m_Damage.m_CachedDamageLevel = m_Damage.m_DamageLevel;
	}
	void SetImpactSize(float p_ImpactSize) { m_Damage.m_ImpactSize = p_ImpactSize; }

	void CreateDamageBehaviour();

	//! Handle damage
	E_DefferedEvent HandleDamage(float p_NewDamageLevel);

	//! Freeze/resume part
	void Freeze(bool p_bFreeze, const Maths::HMat43& p_ParentMatrix);

	Maths::HVec3 GetCentre();
	Maths::HVec3 GetExtents();

	void OverrideConstraintLimits(float p_LowLimit, float p_HighLimit);
	void RestoreConstraintLimits();

public:
	
	struct C_Damage
	{
		void Clear()
		{
			m_HingeActive	= false;
			m_Detached		= false;
			m_DamageLevel	= 0.0f;
			m_Transfer.Clear();
		}

		XtDamageBehaviour		m_Behaviour;

		struct Transfer
		{
			C_Part*	m_pNode;
			float				m_Percentage;
		};

		bool					m_HingeActive;
		bool					m_Detached;
		float					m_DamageLevel;
		float					m_TargetDamageLevel;
		float					m_CachedDamageLevel;
		float					m_ImpactSize;

		Btl::Vector<Transfer>	m_Transfer;
	};

	Maths::HMat43							m_Matrix;
	Maths::HMat43							m_RestMatrix;
	Maths::HMat43							m_FrozenMatrix;

	Maths::HVec3							m_Scale;
	
	C_Part*									m_pParent;

	MathsStorage::MatrixRow44*				m_pWorldMatrix;
	Gravity::C_Body*						m_pPhysicsBody;
	C_Damage								m_Damage;

	C_MaterialBase*							m_pMaterialParameters;
	E_MaterialType							m_MaterialParametersType;
	Damage::C_BehaviourBase*				m_pDamageBehaviour;
	U32										m_DamageBehaviourType;
	
	bool									m_MaterialParametersUpdate;
	bool									m_bActive; //TODO: HOOK THIS UP!

	const GameResource::C_Model*			m_pModel;
	WorldRender::C_ModelInstance*			m_pModelInstance;
	const GameResource::C_ModelElement*		m_pElement;
	const GameResource::C_ModelResource*	m_pModelResource;
	
	Collision::C_CollisionFilter			m_CollisionFilter;
	Btl::Vector<SharedPhysics::C_Constraint*>	m_pConstraints;
	Maths::HVec3							m_ConstraintAxis;
	Gravity::C_Scene*						m_pPhysicsScene;
	
	C_PartList*								m_pPartList;
	Btl::Vector<C_Part*>					m_pChildren;

	U8										m_CurrentConstraint;
	S8										m_PermanentVisibility;
	
	bool									m_bAddedToGravity;
	bool									m_Hidden;
	bool									m_Visible;
	bool									m_bFrozen;
	
	E_PartCode										m_PartCode;
	E_PartSubCode									m_PartSubCode;
	SharedPhysics::C_Constraint::E_ConstraintState	m_PreviousConstraintState;
	SharedPhysics::C_Constraint::E_ConstraintState	m_CurrentConstraintState;

//#ifndef VEHICLE_MASTER_BUILD
	Util::C_String							debug_Name;
	S32										debug_VehicleIndex;
//#endif

	DynamicEffect::Particle::C_SpawnMesh*	m_pSpawnMesh;

	Btl::Pair<float, float>					m_ConstraintLimits;
};

/* 
===============================================================
					class C_PartGroup

NOTES: Used to map the render node with the vehicle part code 
===============================================================
*/
class C_PartGroup
{
public:

	C_PartGroup();

	void Shutdown();
public:
	
	E_PartCode													m_PartCode;

	Ut::UtVector<Ut::ArrayVector<C_Part, E_PartSubCode_Max> >	m_Nodes;
};

typedef Ut::UtVector<Ut::ArrayVector<C_PartGroup, E_PartCode_Max> > PartList;

/* 
===============================================================
					class C_PartGroup

NOTES: Used to map the render node with the vehicle part code 
===============================================================
*/
class C_PartList
{
public:

	//! Initialises the parts
	void Initialise();

	//! Post-initialises the parts
	void PostInitialise();

	//! Shuts down the parts
	void Shutdown();

	//! Updates matrices
	void Update(C_ModelInput* p_pModelInput, float p_fTimeStep);

	//! Updates matrices
	void UpdateMatrices(Maths::HMat43 p_Matrix, const SceneGraph::C_VisibleDetect* pVisDetect, bool forceUpdateAll = false, Maths::HMat43* pOriginalMatrix = NULL, bool resetEvent = false);

	void SetPartsLinearVelocity(const Maths::HVec3 PCREF velocity);

	void ZeroPartsLinearVelocity();

	void ZeroPartsAngularVelocity();

	void Clear();

	WorldRender::C_ModelInstance* GetModelInstance(E_PartCode p_PartCode, E_PartSubCode p_LocCode);

	SharedPhysics::C_Constraint* CreateConstraint(	C_Part* p_pNode,
													XtConstraintBase* p_pConstraint,
													Solver2::C_ConstraintGraphManager* p_pGraphManager,
													Gravity::C_Body* p_pBodyA, Gravity::C_Body* p_pBodyB,
													Maths::HMat43 p_VehicleMatrix);

	//! Builds dynamic materials
	void BuildDynamicMaterials();

	//! Updates all dynamic materials for the parts
	void UpdateDynamicMaterials(XtVehicle* p_pVehicle, XtMaterialDefaults* p_pMaterialDefaults);

	//! Updates the dynamic materials for a specific part
	void UpdateDynamicMaterial(	XtVehicle* p_pVehicle, XtMaterialDefaults* p_pMaterialDefaults,
								E_PartCode p_PartCode, E_PartSubCode p_SubCode);

	//! Disables all parts and removes physics objects
	void DisableAllParts();

	U32 Size() { return m_Parts.Size(); }

	C_PartGroup& operator[](U32 index) { return m_Parts[index]; }
	
	void DebugDraw(Maths::HMat43& p_Matrix);

	//! CreateDamageBehaviours
	void CreateDamageBehaviours();

	//! recalculate the list of parts that need updating every frame
	void RecalcUpdatePartList();

	//! Checks that the part update list is valid
	void CheckUpdatePartList();

	//! Freeze/resume parts
	void FreezeParts(bool p_bFreeze, const Maths::HMat43& p_ParentMatrix);

	//! Creates spawn mesh for particles 
	void CreateSpawnMesh(C_Part* p_pPart);

	//! Outputs parts with physics meshes
	void PrintPhysicsParts();

	//! Re-creates constraints
	void Debug_DestroyConstraints();
	void Debug_CreateConstraints(XtVehicle* p_pXtVehicle, Gravity::C_Body* p_pCollisionBody, Maths::HMat43 p_VehicleMatrix);

public:
	void UpdateMatrix(C_Part *pNode, Maths::HMat43 &p_Matrix, const SceneGraph::C_VisibleDetect* pVisDetect, Maths::HMat43* pOriginalMatrix = NULL, bool resetEvent = false);

	PartList					m_Parts;

	struct PartToUpdate
	{
		C_Part*				pPart;
		Gravity::C_Body*	pBody;
	};

	Btl::Vector<PartToUpdate>	m_PartsToUpdate;
	bool						m_bRecalculatePartUpdateList;

	C_ControlPointHandler		m_ControlPointHandler;

	struct ActivePart
	{
		E_PartCode				m_PartCode;
		E_PartSubCode			m_PartSubCode;
		C_Part*		m_pNode;
	};

	Ut::Vector<ActivePart>	m_ActiveParts;
	Ut::Vector<C_Part*>		m_DamageParts;
	Btl::Vector<C_Part*>	m_ConstrainedParts;
	Btl::Vector<C_Part*>	m_PhysicsParts;	

	//Light Damage Intensity
	F32					m_HeadLightIntensity;
	F32					m_RearLightIntensity;
	F32					m_BreakLightIntensity;
	F32					m_ReverseLightIntensity;
};

/* 
===============================================================
					class C_VehicleWindow

NOTES: 
===============================================================
*/

class C_VehicleWindow
{
public:

	C_VehicleWindow();

	//! Initialises window
	void	Initialise(XtWindow* p_pXtWindow, C_Part* p_pPart);

	//! Sets window position
	void	SetPosition(float p_fPosition);

	//! Winds window up
	void	WindUp();

	//! Winds window down
	void	WindDown();

	//! Updates window movement
	void	Update(float p_fTimeStep);

public:

	C_Part*						m_pPart;

	Maths::HMat43				m_RestMatrix;

	Maths::HVec3				m_FinalPosition;
	float						m_FinalRotation;

	float						m_Position;
	float						m_TargetPosition;
	float						m_Speed;

	bool						m_Overriden;
};

} //namespace Vehicle

#endif
