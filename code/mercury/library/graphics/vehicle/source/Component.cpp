/////////////////////////////////////////////////////////////
// Component
/////////////////////////////////////////////////////////////

#include "VehiclePCH.h"

#include <Vehicle/Component.h>
#include <Vehicle/Db/VehicleXt.h>
#include <Vehicle/Material/Material.h>
#include <Vehicle/Damage/DamageBehaviours.h>
#include <Vehicle/Input.h>

#include <game_resource/ModelResource.h>
#include <game_resource/ModelData.h>

#include <world_render/RenderModelData.h>
#include <world_render/ModelInstance.h>

#include <gravity/sdk/Body.h>
#include <gravity/sdk/Scene.h>
#include <gravity/sdk/ConstraintDescriptors.h>
#include <gravity/sharedphysics/GraphNode.h>

#include <gravity/sdk/KinematicController.h>
#include <gravity/narrowphase/Shape.h>
#include <Gravity/solver2/Constraint.h>
#include <gravity/narrowphase/shapes.h>

#include <scenegraph/visibledetect.h>

#include <System/Misc/Random.h>

#include <dynamic/particle/SpawnMesh.h>
#include <Core/XtStdX.h>

//#include <Misc_shared/NewCurveHermite.h>
//#include <Misc_Shared/NewCurve/Actor/ActorSplineTypes.h>

Debug::Logger &g_VehiclePartLog( Debug::GetLogger("amax.actors.vehicle.parts") );

#if defined(TURN_OFF_VEHICLE_OPTIMISATIONS)
	#pragma optimize("", off)
#endif

//#pragma optimize("", off)

Debug::Bool g_debugVibrateAddTorque("Vehicle.BodyShake.Damage.Parts.AddTorque", true);
Debug::Float g_debugPartVibrateScaleOverride("Vehicle.BodyShake.Damage.Parts.ScaleOverride", 0.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Bool g_dbPostStabilise("Vehicle.Damage.Parts.PostStabilise", true);
Debug::Bool g_debugPartVibrateScaleEnable("Vehicle.BodyShake.Damage.Parts.Enable", true);
Debug::Bool g_debugDrawPartInTheSameFrame("Vehicle.Collision.DrawPartShapeInstantly", false);

static Util::C_Random g_PartVibrateRandom;

namespace Vehicle
{
/* 
===============================================================
					class C_ControlPointHandler

NOTES: 
===============================================================
*/

/*
===============
NAME: C_ControlPointHandler

NOTE: Constructor
===============
*/
C_ControlPointHandler::C_ControlPointHandler() 
{
	Clear();	
}

/*
===============
NAME: Clear

NOTE: Clear
===============
*/
void C_ControlPointHandler::Clear() 
{
	m_ParameterHandle.index		= -1;
	m_ParameterHandle.offset	= -1;

	m_pModelInstance			= NULL;
	
	for (S32 iPart = 0; iPart < m_Parts.Size(); iPart++)
	{
		m_Parts[iPart].Clear();
	}
	memset(m_Data, 0, sizeof(MathsStorage::Float4)*E_Part_Max*3);
}

Debug::Float g_debugBonnetMaxMovement("Vehicle.Damage.Code.ControlPoints.BonnetMaxMovement", 0.2f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugBonnetMovementRange("Vehicle.Damage.Code.ControlPoints.BonnetMovementRange", 0.35f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugBootMaxMovement("Vehicle.Damage.Code.ControlPoints.BootMaxMovement", 0.1f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugBootMovementRange("Vehicle.Damage.Code.ControlPoints.BootMovementRange", 0.25f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugBumperMaxMovement("Vehicle.Damage.Code.ControlPoints.BumperMaxMovement", 0.15f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugBumperMovementRange("Vehicle.Damage.Code.ControlPoints.BumperMovementRange", 0.2f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugWingMaxMovement("Vehicle.Damage.Code.ControlPoints.WingMaxMovement", 0.15f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugWingMovementRange("Vehicle.Damage.Code.ControlPoints.WingMovementRange", 0.35f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

/*
===============
NAME: Setup

NOTE: Setup
===============
*/
void C_ControlPointHandler::Setup(C_Part* p_pNode)
{
	if (!p_pNode)
		return;

	Part* pPart = NULL;

	switch (p_pNode->m_PartCode)
	{
	case E_PartCode_Bumper:
		{
			if (p_pNode->m_PartSubCode == E_PartSubCode_Front)
				pPart = &m_Parts[E_Part_BumperFront];
			else if (p_pNode->m_PartSubCode == E_PartSubCode_Back)
				pPart = &m_Parts[E_Part_BumperBack];

			if (pPart)
			{
				pPart->m_MovementIndex0		= 0;
				pPart->m_MovementIndex1		= 2;
				pPart->m_ControlIndex		= 0;
				if (p_pNode->m_PartSubCode == E_PartSubCode_Back)
					pPart->m_OffsetDirection	= Maths::Constants<Maths::HVec3>::UnitAxisNegativeZ();
				else
					pPart->m_OffsetDirection	= Maths::Constants<Maths::HVec3>::UnitAxisPositiveZ();
				pPart->m_MaxMovement		= (float)g_debugBumperMaxMovement;
				pPart->m_MovementRange		= (float)g_debugBumperMovementRange;
			}			
		}
		break;	
	case E_PartCode_Bonnet:
		{
			pPart = &m_Parts[E_Part_Bonnet];
			pPart->m_MovementIndex0		= 1;
			pPart->m_MovementIndex1		= -1;
			pPart->m_AnchorIndex		= 2;
			pPart->m_ControlIndex		= 0;
			pPart->m_OffsetDirection	= Maths::Constants<Maths::HVec3>::UnitAxisPositiveY();
			pPart->m_MaxMovement		= (float)g_debugBonnetMaxMovement;
			pPart->m_MovementRange		= (float)g_debugBonnetMovementRange;
			if (pPart->m_ControlPoints[2].m_RestPosition.Z() > pPart->m_ControlPoints[0].m_RestPosition.Z())
			{
				pPart->m_ControlIndex	= 2;
				pPart->m_ControlIndex	= 0;
			}
			break;
		}
	case E_PartCode_Boot:
		{
			pPart = &m_Parts[E_Part_Boot];
			pPart->m_MovementIndex0		= 1;
			pPart->m_MovementIndex1		= -1;
			pPart->m_AnchorIndex		= 2;
			pPart->m_ControlIndex		= 0;
			pPart->m_OffsetDirection	= Maths::Constants<Maths::HVec3>::UnitAxisPositiveY();
			pPart->m_MaxMovement		= (float)g_debugBonnetMaxMovement;
			pPart->m_MovementRange		= (float)g_debugBonnetMovementRange;
			if (pPart->m_ControlPoints[2].m_RestPosition.Z() > pPart->m_ControlPoints[0].m_RestPosition.Z())
			{
				pPart->m_ControlIndex	= 2;
				pPart->m_AnchorIndex	= 0;
			}
			break;
		}
	case E_PartCode_Wing:
		{
			if (p_pNode->m_PartSubCode == E_PartSubCode_FrontLeft || p_pNode->m_PartSubCode == E_PartSubCode_Left)
				pPart = &m_Parts[E_Part_WingLeft];
			else if (p_pNode->m_PartSubCode == E_PartSubCode_FrontRight || p_pNode->m_PartSubCode == E_PartSubCode_Right)
				pPart = &m_Parts[E_Part_WingRight];

			if (pPart)
			{
				pPart->m_MovementIndex0	= 1;
				pPart->m_MovementIndex1	= -1;
				pPart->m_ControlIndex		= 0;
				if (p_pNode->m_PartSubCode == E_PartSubCode_Right || p_pNode->m_PartSubCode == E_PartSubCode_FrontRight)
					pPart->m_OffsetDirection	= Maths::Constants<Maths::HVec3>::UnitAxisPositiveX();
				else
					pPart->m_OffsetDirection	= Maths::Constants<Maths::HVec3>::UnitAxisNegativeX();
				pPart->m_MaxMovement		= (float)g_debugWingMaxMovement;
				pPart->m_MovementRange		= (float)g_debugWingMovementRange;
				
				if (pPart->m_ControlPoints[2].m_RestPosition.Z() > pPart->m_ControlPoints[0].m_RestPosition.Z())
					pPart->m_ControlIndex	= 2;
			}
		}
		break;
	}	
}

/*
===============
NAME: Finalise

NOTE: Finalise
===============
*/
void C_ControlPointHandler::Finalise(WorldRender::C_ModelInstance* p_pModelInstance)
{
	m_pModelInstance		= p_pModelInstance;
	m_ParameterHandle		= p_pModelInstance->GetRenderModel()->GetParameter("g_vControlPoints");
	Update();
	SetParameters();	
}

/*
===============
NAME: AddControlPoint

NOTE: Adds a control point
===============
*/
void C_ControlPointHandler::AddControlPoint(Util::C_String p_Name, Maths::HVec3 p_Position, C_Part* p_pNode)
{
	if (!p_pNode)
		return;

	int num = -1;
	if (p_Name.ISearch("bone0") != -1)
		num = 0;
	else if (p_Name.ISearch("bone1") != -1)
		num = 1;
	else if (p_Name.ISearch("bone2") != -1)
		num = 2;

	if (num < 0 || num > 2)
		return;

	Part* pPart = GetPart(p_pNode);

	if (!pPart)
		return;

	pPart->m_pNode = p_pNode;

	pPart->m_ControlPoints[num].m_RestPosition	= p_Position;
	pPart->m_ControlPoints[num].m_Position		= Maths::HVec3(0.0f, 0.0f, 0.0f);
	pPart->m_ControlPoints[num].m_MinExtents	= Maths::HVec3(-0.25f, -0.25f, -0.25f);
	pPart->m_ControlPoints[num].m_MaxExtents	= Maths::HVec3(0.25f, 0.25f, 0.25f);
}

Debug::Bool g_debugOverrideControlPoints("Vehicle.Damage.Code.ControlPoints.Override", false);

/*
===============
NAME: Update

NOTE: Update
===============
*/
void C_ControlPointHandler::Update()
{
	for (S32 iPart = 0; iPart < E_Part_Max; iPart++)
	{
		Part* pPart = &m_Parts[iPart];

		if (!pPart->m_pNode || !pPart->m_pDamageNode)
			continue;

		if (pPart->m_ControlIndex == -1 || pPart->m_MovementIndex0 == -1 || g_debugOverrideControlPoints)
			return;

		float fOffset = Maths::Clamp(pPart->m_pDamageNode->m_OffsetLength / pPart->m_MovementRange, 0.0f, 1.0f) * pPart->m_MaxMovement;
		pPart->m_ControlPoints[pPart->m_MovementIndex0].m_Position = pPart->m_OffsetDirection * fOffset;

		if (pPart->m_MovementIndex1 != -1)
		{
			fOffset = Maths::Clamp(pPart->m_pDamageNode->m_OffsetLength / pPart->m_MovementRange, 0.0f, 1.0f) * pPart->m_MaxMovement;
			pPart->m_ControlPoints[pPart->m_MovementIndex1].m_Position = pPart->m_OffsetDirection * fOffset;
		}

		S32 base = iPart * 3;
		m_Data[base]	= MathsStorage::Float4(pPart->m_ControlPoints[0].m_Position.X(), pPart->m_ControlPoints[0].m_Position.Y(), pPart->m_ControlPoints[0].m_Position.Z(), 1.0f);
		m_Data[base+1]	= MathsStorage::Float4(pPart->m_ControlPoints[1].m_Position.X(), pPart->m_ControlPoints[1].m_Position.Y(), pPart->m_ControlPoints[1].m_Position.Z(), 1.0f);
		m_Data[base+2]	= MathsStorage::Float4(pPart->m_ControlPoints[2].m_Position.X(), pPart->m_ControlPoints[2].m_Position.Y(), pPart->m_ControlPoints[2].m_Position.Z(), 1.0f);	
	}
}

/*
===============
NAME: SetParameters

NOTE: Updates shader parameters
===============
*/
void C_ControlPointHandler::SetParameters()
{
	if (!m_pModelInstance || m_ParameterHandle.index == -1 || m_ParameterHandle.offset == -1)
		return;

	m_pModelInstance->SetParameter(m_ParameterHandle, (void*)m_Data);
}

/*
===============
NAME: SetParameters

NOTE: Gets part
===============
*/
C_ControlPointHandler::Part* C_ControlPointHandler::GetPart(C_Part* p_pPart)
{
	Part* pPart = NULL;

	switch (p_pPart->m_PartCode)
	{
	case E_PartCode_Bumper:
		{
			if (p_pPart->m_PartSubCode == E_PartSubCode_Front)
				pPart = &m_Parts[E_Part_BumperFront];
			else if (p_pPart->m_PartSubCode == E_PartSubCode_Back)
				pPart = &m_Parts[E_Part_BumperBack];
		}
		break;	
	case E_PartCode_Bonnet:
			pPart = &m_Parts[E_Part_Bonnet];
		break;
	case E_PartCode_Boot:
			pPart = &m_Parts[E_Part_Boot];
		break;
	case E_PartCode_Wing:
		{
			if (p_pPart->m_PartSubCode == E_PartSubCode_FrontLeft || p_pPart->m_PartSubCode == E_PartSubCode_Left)
				pPart = &m_Parts[E_Part_WingLeft];
			else if (p_pPart->m_PartSubCode == E_PartSubCode_FrontRight || p_pPart->m_PartSubCode == E_PartSubCode_Right)
				pPart = &m_Parts[E_Part_WingRight];
		}
		break;
	}

	return pPart;
}

/* 
===============================================================
					class C_Part

NOTES: Component node 
===============================================================
*/

/*
===============
NAME: C_VehicleActor

NOTE: Constructor
===============
*/
C_Part::C_Part() 
{  
	m_pModel					= NULL;
	m_pModelInstance			= NULL;
	m_pElement					= NULL;
	m_pWorldMatrix				= NULL;
	m_pPhysicsBody				= NULL;
	m_pMaterialParameters		= NULL;
	m_pPhysicsScene				= NULL;
	m_pParent					= NULL;
	m_pModelResource			= NULL;
	m_pDamageBehaviour			= NULL;

	m_pPartList					= NULL;
	m_pSpawnMesh				= NULL;

	m_PartCode					= E_PartCode_None;
	m_PartSubCode				= E_PartSubCode_None;
	m_PreviousConstraintState	= SharedPhysics::C_Constraint::E_Normal;
	m_CurrentConstraintState	= SharedPhysics::C_Constraint::E_Normal;

	m_RestMatrix.SetIdentity();
	m_Matrix.SetIdentity();

	m_PermanentVisibility	= -1;

	m_CurrentConstraint		= 0;

	m_MaterialParametersUpdate	= true;

	m_bAddedToGravity		= false;
	m_bActive				= false;
	m_Visible				= true;
	m_Hidden				= false;
	m_bFrozen				= false;

	m_Damage.Clear();

	m_Scale				= Maths::HVec3(1.0f, 1.0f, 1.0f);
#ifndef VEHICLE_MASTER_BUILD
	debug_VehicleIndex = -1;
#endif
}

bool C_Part::GetVisible(bool getFlag) 
{ 
	if (getFlag)
		return m_Visible;
	else if (m_PermanentVisibility == 0)
		return false;
	else if (m_PermanentVisibility == 1)
		return true;
	//else if (!getRaw)
	//	return m_Visible;

	if (!m_pModelInstance || !m_pElement)
		return false;

	//if (m_pElement->GetIndex() >= m_pModelInstance->GetNumElements())
	//	return true; //V. V. V. TEMP CRASH FIX??????

	return m_pModelInstance->GetElementVisibility(m_pElement->GetIndex());
}

//! Sets node visiblity
void C_Part::SetVisible(bool p_Visible, bool setRaw) 
{ 
	if (/*(!setRaw && m_Hidden) ||*/ !m_pModelInstance || !m_pElement || (p_Visible == true && m_PermanentVisibility == 0))
		return;

	//if (m_pElement->GetIndex() >= m_pModelInstance->GetNumVisibleElements())
	//	return; //V. V. V. TEMP CRASH FIX??????

	m_pModelInstance->SetElementVisibility(m_pElement->GetIndex(), p_Visible);
	m_pPartList->m_bRecalculatePartUpdateList = true;
	m_Visible = p_Visible;
}

//! Hides part
void C_Part::Hide(bool p_Hidden, bool raw) 
{
	/*if (p_Hidden)
	{
		if (!m_Hidden && m_Visible)
		{
			SetVisible(false);
			m_Hidden = true;
		}
	}
	else
	{
		if (m_Hidden && !m_Visible)
		{
			m_Hidden = false;
			SetVisible(true);
		}
	}
	m_Hidden = p_Hidden;*/

	if (p_Hidden)
	{
		if (!m_Hidden && m_Visible)
		{
			SetVisible(false);
		}
	}
	else
	{
		if (m_Hidden && !m_Visible)
		{
			SetVisible(true);
		}
	}
	m_Hidden = p_Hidden;
}

/*
===============
NAME: ActivatePhysicsObject

NOTE: Activates the physics object
===============
*/
void C_Part::ActivatePhysicsObject(Maths::HVec3 p_Velocity, bool forceActive)
{
	if (!m_pPhysicsBody || !m_pPhysicsScene || m_bFrozen)
		return;

	if (!forceActive && m_bActive)//m_pPhysicsBody->IsActive())
		return;

	if (!m_bAddedToGravity)
	{
	#ifndef VEHICLE_MASTER_BUILD
		LOGDEBUG(g_VehiclePartLog, "ActivatePhysicsObject : Vehicle[%d] : Body '%s' getting added to Gravity", debug_VehicleIndex, debug_Name.Chars());
	#endif
		m_pPhysicsScene->AddBody(m_pPhysicsBody);
		m_bAddedToGravity = true;		
	}

	m_pPhysicsBody->ElementMatrix(m_Matrix);
	m_pPhysicsBody->Activate();
	m_bActive = true;
	m_pPhysicsBody->LinearVelocity(p_Velocity);
	
	if (m_pPhysicsBody->GetShape())
	{
		m_CollisionFilter = *m_pPhysicsBody->GetShape()->GetFilter();
		Collision::C_CollisionFilter filter;
		m_pPhysicsBody->GetShape()->SetGroupFilter(&filter);
	}

	if (m_CurrentConstraint < m_pConstraints.Size())
	{
		m_pConstraints[m_CurrentConstraint]->SetScale(1.0f);
		m_pConstraints[m_CurrentConstraint]->ZeroWarmStartValues();
	}
}

Debug::Bool g_debugAllowPartHinging("Vehicle.Damage.Parts.AllowHinging", true);
Debug::Bool g_debugAllowPartDetaching("Vehicle.Damage.Parts.AllowDetaching", false);

/*
===============
NAME: Detach

NOTE: Detaches object
===============
*/
void C_Part::Detach(Maths::HVec3 p_Velocity)
{
	if (!g_debugAllowPartDetaching || m_bFrozen)
		return;

	if (!m_pPhysicsBody && GetVisible())
	{
		SetVisible(false);
		//ToDo:
		//m_pBreakParticleEffect = go!;
	}

	if (!m_pPhysicsBody || !m_pPhysicsScene || m_Damage.m_Detached)
		return;

	//8 == Gravity::C_Body::E_InsertedIntoWorld
	if (!m_bAddedToGravity)
	{
	#ifndef VEHICLE_MASTER_BUILD
		LOGDEBUG(g_VehiclePartLog, "Detach : Vehicle[%d] : Body '%s' getting added to Gravity", debug_VehicleIndex, debug_Name.Chars());
	#endif
		m_pPhysicsScene->AddBody(m_pPhysicsBody);
		m_bAddedToGravity = true;
	}

	if (!m_bActive)//m_pPhysicsBody->IsActive())
	{
		m_pPhysicsBody->ElementMatrix(m_Matrix);
		m_pPhysicsBody->Activate();
		m_bActive = true;
		m_pPhysicsBody->LinearVelocity(p_Velocity);
	}

	if (m_pPhysicsBody->GetShape())
	{
		m_CollisionFilter = *m_pPhysicsBody->GetShape()->GetFilter();
		Collision::C_CollisionFilter filter;
		m_pPhysicsBody->GetShape()->SetGroupFilter(&filter);
	}

	if (m_CurrentConstraint < m_pConstraints.Size())
	{
		m_pConstraints[m_CurrentConstraint]->SetScale(0.0f);
		m_pConstraints[m_CurrentConstraint]->ZeroWarmStartValues();
	}

	m_Damage.m_Detached = true;

	for (int iChild = 0; iChild < m_pChildren.Size(); iChild++)
	{
		if (!m_pChildren[iChild])
			continue;

		m_pChildren[iChild]->Detach(p_Velocity);
	}
}

/*
===============
NAME: UpdateVibration

NOTE: Update vibration on part
===============
*/
void C_Part::UpdateVibration(float p_fNoise)
{
	if (!m_Damage.m_HingeActive || m_bFrozen)
		return;

	Maths::HVec3 vTorque = m_ConstraintAxis * p_fNoise;
	
	vTorque = vTorque * m_Matrix.Rotation();

	m_pPhysicsBody->AddTorque(vTorque);	
}

/*
===============
NAME: ActivateConstraints

NOTE: Activates constraints
===============
*/
void C_Part::ActivateConstraints()
{
	if (!g_debugAllowPartHinging || m_bFrozen || m_pConstraints.Size() == 0)
		return;

	if (!m_pPhysicsBody)
		SetVisible(true);

	if (!m_pPhysicsBody || !m_pPhysicsScene || m_Damage.m_Detached || m_Damage.m_HingeActive)
		return;
	
	//m_pPhysicsBody->ElementMatrix(m_Matrix);
	const Maths::HVec3 zero(Maths::Constants<Maths::HVec3>::Zero());
	const Maths::HVec3 newPos(m_pPhysicsBody->ElementOffset() * m_Matrix);

	const Maths::HVec3 linVel = m_pPhysicsBody->GetBody().PointVelocity(newPos);

	const Maths::C_Quaternion orientation(m_Matrix.Rotation());
	m_pPhysicsBody->GetBody().StoreVelocitiesAndPosition(linVel, zero, orientation, newPos);

	//8 == Gravity::C_Body::E_InsertedIntoWorld
	if (!m_bAddedToGravity)
	{
	#ifndef VEHICLE_MASTER_BUILD
		LOGDEBUG(g_VehiclePartLog, "ActivateConstraints : Vehicle[%d] : Body '%s' getting added to Gravity", debug_VehicleIndex, debug_Name.Chars());
	#endif
		m_pPhysicsScene->AddBody(m_pPhysicsBody);
		m_bAddedToGravity = true;
	}

	if (!m_bActive)//m_pPhysicsBody->IsActive())
	{
		//m_pPhysicsBody->ElementMatrix(m_Matrix);
		m_pPhysicsBody->Activate();
		m_bActive = true;
	}

	/*if (m_pPhysicsBody->GetShape())
	{
		m_CollisionFilter = *m_pPhysicsBody->GetShape()->GetFilter();
		Collision::C_CollisionFilter filter;
		m_pPhysicsBody->GetShape()->SetGroupFilter(&filter);
	}*/

	if (m_CurrentConstraint < m_pConstraints.Size())
	{
		m_pConstraints[m_CurrentConstraint]->SetScale(1.0f);
		m_pConstraints[m_CurrentConstraint]->ZeroWarmStartValues();
		m_PreviousConstraintState	= m_pConstraints[m_CurrentConstraint]->GetConstraintState();
	}
	
	m_Damage.m_HingeActive = true;

	for (int iChild = 0; iChild < m_pChildren.Size(); iChild++)
	{
		if (!m_pChildren[iChild])
			continue;

		m_pChildren[iChild]->Detach(Maths::HVec3(0.0f, 0.0f, 0.0f));
	}	
}

/*
===============
NAME: Reattach

NOTE: Reattaches object
===============
*/
void C_Part::Reattach()
{
	if (!m_pPhysicsBody || !m_pPhysicsScene || !m_Damage.m_Detached || m_bFrozen)
		return;

	if (m_pParent && m_pParent->m_Damage.m_Detached)
		return;

	m_pPhysicsBody->ElementMatrix(m_Matrix);
	
	//if (m_pPhysicsBody->IsActive() && !m_Damage.m_HingeActive)
	if (m_bActive && !m_Damage.m_HingeActive)
	{
		m_pPhysicsBody->Deactivate();
		m_bActive = false;

		if (m_bAddedToGravity)
		{
			m_pPhysicsScene->TemporarilyRemoveBody(m_pPhysicsBody);
			m_bAddedToGravity = false;			
		}
	}

	if (m_pPhysicsBody->GetShape())
	{
		m_pPhysicsBody->GetShape()->SetGroupFilter(&m_CollisionFilter);
	}

	DeactivateConstraints();
	/*if (m_Damage.m_HingeActive)
	{
		for (int iConstraint = 0; iConstraint < m_pConstraints.Size(); iConstraint++)
		{
			if (m_pConstraints[iConstraint])
				m_pConstraints[iConstraint]->SetScale(1.0f);
		}
	}*/

	m_Damage.m_Detached = false;

	for (int iChild = 0; iChild < m_pChildren.Size(); iChild++)
	{
		if (!m_pChildren[iChild])
			continue;

		m_pChildren[iChild]->Reattach();
	}
}

/*
===============
NAME: DeactivateConstraints

NOTE: Deactivates constraints
===============
*/
void C_Part::DeactivateConstraints()
{
	if (!m_pPhysicsBody || !m_pPhysicsScene || m_Damage.m_Detached || !m_Damage.m_HingeActive)
		return;
	
	//8 == Gravity::C_Body::E_InsertedIntoWorld
	if (m_bAddedToGravity)
	{
		m_pPhysicsScene->TemporarilyRemoveBody(m_pPhysicsBody);
		m_bAddedToGravity = false;		
	}
	
	if (m_bActive)//m_pPhysicsBody->IsActive())
	{
		m_pPhysicsBody->Deactivate();
		m_bActive = false;
	}

	if (m_CurrentConstraint < m_pConstraints.Size())
	{
		m_pConstraints[m_CurrentConstraint]->SetScale(0.0f);
		m_pConstraints[m_CurrentConstraint]->ZeroWarmStartValues();
		m_PreviousConstraintState	= m_pConstraints[m_CurrentConstraint]->GetConstraintState();
	}

	m_Damage.m_HingeActive = false;
}

Debug::Bool g_debugReattachPartOnDeactivate("Vehicle.Debug.ReattachPartOnDeactivate", false);

/*
===============
NAME: DeactivatePhysicsObject

NOTE: Deactivates the physics object
===============
*/
void C_Part::DeactivatePhysicsObject()
{
	if (!m_pPhysicsBody || m_bFrozen)
		return;

	m_pPhysicsBody->Deactivate();
	m_bActive = false;

	if (g_debugReattachPartOnDeactivate)
		m_Damage.m_Detached = false;

	if (m_CurrentConstraint < m_pConstraints.Size())
	{
		m_pConstraints[m_CurrentConstraint]->SetScale(0.0f);
		m_pConstraints[m_CurrentConstraint]->ZeroWarmStartValues();
	}

	//m_pPhysicsBody->MakeStaticMesh();

	/*if (m_pPhysicsBody->GetShape())
		m_pPhysicsBody->GetShape()->SetGroupFilter(&m_CollisionFilter);*/
}

/*
===============
NAME: Shutdown

NOTE: Shutdown
===============
*/
void C_Part::Shutdown() 
{  
	m_pModel				= NULL;
	m_pModelInstance		= NULL;
	m_pElement				= NULL;
	m_pPhysicsBody			= NULL;
	m_pPhysicsScene			= NULL;
	m_pParent				= NULL;
	m_pModelResource		= NULL;

	m_bAddedToGravity		= false;
	m_bFrozen				= false;

	m_PermanentVisibility	= -1;

	m_CurrentConstraint		= 0;

#ifndef VEHICLE_MASTER_BUILD
	debug_VehicleIndex = -1;
#endif

	SAFE_DELETE_PTR(m_pMaterialParameters)
	
	SAFE_DELETE_PTR(m_pDamageBehaviour)

	SAFE_DELETE_PTR(m_pSpawnMesh)

	m_Damage.Clear();
		
	m_pConstraints.Clear();

	m_pChildren.Clear();
}

/*
===============
NAME: SetShaderParameter

NOTE: Sets shader parameter
===============
*/
void C_Part::SetShaderParameter(Util::C_String p_Name, MathsStorage::Float4 p_Values) 
{ 
	if (!m_pModelInstance)
		return;

	WorldRender::MaterialParameter handle = m_pModelInstance->GetRenderModel()->GetParameter(p_Name.Chars());
	if (handle.index != -1 && handle.offset != -1)
		m_pModelInstance->SetParameter(handle, (void*)&p_Values);
}

Debug::Float g_debugWindowDamageLevelThreshold("Vehicle.Damage.Parts.WindowDamageThreshold", 0.15f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugWindowDamageImpactThreshold("Vehicle.Damage.Parts.WindowDamageImpactThreshold", 0.15f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugLightDamageImpactThreshold("Vehicle.Damage.Lights.DamageImpactThreshold", 0.25f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugDamageLightInstensityScale("Vehicle.Damage.Lights.IntensityScale", 0.5f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

/*
===============
NAME: HandleDamage

NOTE: Handle damage - for now just pass in damage level, sort out proper like soon!
===============
*/
E_DefferedEvent C_Part::HandleDamage(float p_NewDamageLevel) 
{
	E_DefferedEvent dEvent = E_DefferedEvent_Unknown;

	//Hax0red for now
	if (m_PartCode == E_PartCode_Window && m_pMaterialParameters && m_MaterialParametersType == E_MaterialType_Glass)// &&
		//m_Damage.m_DamageLevel != p_NewDamageLevel)
	{
		C_MaterialGlass* pMaterial = (C_MaterialGlass*)m_pMaterialParameters;

		if (p_NewDamageLevel >= (float)g_debugWindowDamageLevelThreshold)
		{	
			//if (m_Damage.m_ImpactSize >= (float)g_debugWindowDamageImpactThreshold)
			if (m_Damage.m_ImpactSize >= m_Damage.m_Behaviour.m_BreakDamageLevel)
			{
				pMaterial->m_DamageLevel = 1.0f;
				m_MaterialParametersUpdate = true;
				dEvent = E_DefferedEvent_WindowCrack;			
			}
		}
		else
		{
			pMaterial->m_DamageLevel = 0.0f;
			m_MaterialParametersUpdate = true;
		}
	}
	else if (m_PartCode == E_PartCode_LicensePlate && !m_Damage.m_Detached)
	{
		if (m_Damage.m_ImpactSize >= m_Damage.m_Behaviour.m_BreakDamageLevel)
		{
			SetPermanentVisibility(0);
			m_Damage.m_Detached = true;
			if (m_PartSubCode == E_PartSubCode_Front)
				dEvent = E_DefferedEvent_LicensePlateFront;
			else
				dEvent = E_DefferedEvent_LicensePlateBack;
		}
	}
	//else if (m_pDamageBehaviour && m_pDamageBehaviour->GetType() == Damage::E_BehaviourType_Light)
	//{
	//	Damage::C_BehaviourLight* pBehaviour = (Damage::C_BehaviourLight*)m_pDamageBehaviour;
	//	if (p_NewDamageLevel <= 0.3f)
	//		pBehaviour->SetState(2);
	//	else if (p_NewDamageLevel <= 0.6f)
	//		pBehaviour->SetState(1);
	//	else
	//		pBehaviour->SetState(0);
	//}
	
	m_Damage.m_DamageLevel = p_NewDamageLevel;

	m_pPartList->m_bRecalculatePartUpdateList = true;

	return dEvent;
}

/*
===============
NAME: FreezeParts

NOTE: Freeze/resume part
===============
*/
void C_Part::Freeze(bool p_bFreeze, const Maths::HMat43& p_ParentMatrix)
{
	if (p_bFreeze == m_bFrozen || !m_pPhysicsBody)
		return;

	if (p_bFreeze)
	{
		m_FrozenMatrix = m_pPhysicsBody->ElementMatrix() * p_ParentMatrix.InverseRotTrans();
		m_bFrozen = true;
	}
	else
	{
		m_bFrozen = false;
	}
}

/*
===============
NAME: GetCentre

NOTE: Returns centre of the part
===============
*/
Maths::HVec3 C_Part::GetCentre()
{
	if (!m_pElement)
		return m_RestMatrix.AW();
	else if (m_pElement->GetMatrix().AW().All_LE(Maths::HVec3(0.0001f, 0.0001f, 0.0001f)) && m_pElement->GetMatrix().AW().All_GE(Maths::HVec3(-0.0001f, -0.0001f, -0.0001f)))
		return m_pElement->GetLocalBounds().Centre();

	//return m_pElement->GetLocalBounds().Centre();
	return m_pElement->GetMatrix().AW() + m_pElement->GetLocalBounds().Centre();
}

/*
===============
NAME: GetExtents

NOTE: Returns extents of the part
===============
*/
Maths::HVec3 C_Part::GetExtents()
{
	if (!m_pElement)
		return Maths::HVec3(0.0f, 0.0f, 0.0f);
	
	return (m_pElement->GetLocalBounds().Max() - m_pElement->GetLocalBounds().Min());
}

/*
===============
NAME: OverrideConstraintLimits

NOTE: OverrideConstraintLimits
===============
*/
void C_Part::OverrideConstraintLimits(float p_LowLimit, float p_HighLimit)
{
	if (m_pConstraints.Size() == 0)
		return;

	if (m_pConstraints.Size() > 1)
	{
		m_CurrentConstraint = 1;
		m_pConstraints[0]->SetScale(0.0f);
	}
	else
		m_CurrentConstraint = 0;

	if (m_pConstraints[m_CurrentConstraint]->ConstraintType() == SharedPhysics::C_Constraint::E_Type_Proxy)
	{
		Solver2::C_ProxyConstraint* pProxy	= (Solver2::C_ProxyConstraint*)m_pConstraints[m_CurrentConstraint];
		if(pProxy->m_ProxyConstraintType == SharedPhysics::C_Constraint::E_Type_RevoluteLimit)
		{
			Solver2::C_RevoluteLimitConstraint* pConstraint = (Solver2::C_RevoluteLimitConstraint*)pProxy->m_Constraints[2];
			pConstraint->m_LowJointLimit		= p_LowLimit;
			pConstraint->m_HighJointLimit		= p_HighLimit;
		}
	}
	
	if (!m_Damage.m_HingeActive)
		ActivateConstraints();
}

/*
===============
NAME: RestoreConstraintLimits

NOTE: RestoreConstraintLimits
===============
*/
void C_Part::RestoreConstraintLimits()
{
	if (m_pConstraints.Size() == 0)
		return;

	if (m_pConstraints.Size() > 1)
	{
		m_CurrentConstraint = 0;
		m_pConstraints[1]->SetScale(0.0f);
	}
	else
		m_CurrentConstraint = 0;

	if (m_pConstraints[m_CurrentConstraint]->ConstraintType() == SharedPhysics::C_Constraint::E_Type_Proxy)
	{
		Solver2::C_ProxyConstraint* pProxy	= (Solver2::C_ProxyConstraint*)m_pConstraints[m_CurrentConstraint];
		if(pProxy->m_ProxyConstraintType == SharedPhysics::C_Constraint::E_Type_RevoluteLimit)
		{
			Solver2::C_RevoluteLimitConstraint* pConstraint = (Solver2::C_RevoluteLimitConstraint*)pProxy->m_Constraints[2];
			pConstraint->m_LowJointLimit		= m_ConstraintLimits.first;
			pConstraint->m_HighJointLimit		= m_ConstraintLimits.second;
		}
	}
	
	if (m_Damage.m_HingeActive)
		DeactivateConstraints();
}

/* 
===============================================================
					class C_PartGroup

NOTES: Used to map the render node with the vehicle part code 
===============================================================
*/

/*
===============
NAME: C_PartGroup

NOTE: Constructor
===============
*/
C_PartGroup::C_PartGroup() 
{  
	m_PartCode = E_PartCode_None;
}

/*
===============
NAME: Shutdown

NOTE: Shutdown
===============
*/
void C_PartGroup::Shutdown() 
{  
	for (int iNode = 0; iNode < m_Nodes.Size(); iNode++)
	{
		m_Nodes[iNode].Shutdown();
	}
}

/* 
===============================================================
					class C_PartGroup

NOTES: Used to map the render node with the vehicle part code 
===============================================================
*/

/*
===============
NAME: Initialise

NOTE: Initialises the components
===============
*/
void C_PartList::Initialise()
{
	for (int iComponent = 0; iComponent < m_Parts.Size(); iComponent++)
	{
		C_PartGroup* pComponent = &m_Parts[iComponent];

		if (pComponent->m_PartCode == E_PartCode_None)
			continue;

		for (int iNode = 0; iNode < pComponent->m_Nodes.Size(); iNode++)
		{
			C_Part* pNode = &pComponent->m_Nodes[iNode];

			if (!pNode->m_pModel || !pNode->m_pModelInstance || !pNode->m_pElement)
				continue;

			m_ActiveParts.PushBack();
			m_ActiveParts.Back().m_PartCode = pComponent->m_PartCode;
			m_ActiveParts.Back().m_PartSubCode = (E_PartSubCode)iNode;
			m_ActiveParts.Back().m_pNode = pNode;

			if (pNode->m_pConstraints.Size() > 0 && pNode->m_pPhysicsBody)
				m_ConstrainedParts.PushBack(pNode);

			if (pNode->m_pPhysicsBody)
				m_PhysicsParts.PushBack(pNode);			
		}
	}

	PostInitialise();

	CreateDamageBehaviours();

	m_bRecalculatePartUpdateList = true;
}

/*
===============
NAME: PostInitialise

NOTE: Post-initialises the components
===============
*/
void C_PartList::PostInitialise()
{
	for (int iComponent = 0; iComponent < m_ActiveParts.Size(); iComponent++)
	{
		C_Part* pNode = m_ActiveParts[iComponent].m_pNode;

		m_ControlPointHandler.Setup(pNode);
	}

	m_bRecalculatePartUpdateList = true;
}

/*
===============
NAME: Shutdown

NOTE: Shuts down the components
===============
*/
void C_PartList::Shutdown()
{
	for (int iComponent = 0; iComponent < m_Parts.Size(); iComponent++)
	{
		m_Parts[iComponent].Shutdown();
	}
	m_ActiveParts.Clear();
	m_DamageParts.Clear();
	m_ConstrainedParts.Clear();
	m_PhysicsParts.Clear();
	m_ControlPointHandler.Clear();
}

Debug::Float g_debugCarPartKillPLane("Vehicle.PartKillPlane", -500.0f, Debug::Range(-1000.0f, 1000.0f), Debug::Step(0.001f));

void C_PartList::Debug_DestroyConstraints()
{
	for (int iPart = 0; iPart < m_ActiveParts.Size(); iPart++)
	{
		C_Part* pPart = m_ActiveParts[iPart].m_pNode;
		if (!pPart)
			continue;

		bool bHingeActive = pPart->m_Damage.m_HingeActive;
		if (bHingeActive)
			pPart->DeactivateConstraints();

		if (pPart->m_pConstraints.Size() > 0)
		{
			for (S32 iConstraint = 0; iConstraint < pPart->m_pConstraints.Size(); iConstraint++)
			{
				pPart->m_pPhysicsScene->RemoveConstraint(pPart->m_pConstraints[iConstraint]);
			}
			pPart->m_pConstraints.Clear();
		}
	}

	m_ConstrainedParts.Clear();
}

void C_PartList::Debug_CreateConstraints(XtVehicle* p_pXtVehicle, Gravity::C_Body* p_pCollisionBody, Maths::HMat43 p_VehicleMatrix)
{
	if (!p_pXtVehicle->m_Constraints)
		return;

	m_ConstrainedParts.Clear();
	
	for (int iPart = 0; iPart < m_ActiveParts.Size(); iPart++)
	{
		C_Part* pPart = m_ActiveParts[iPart].m_pNode;
		if (!pPart || !pPart->m_pPhysicsBody)
			continue;

		int constraintCount = p_pXtVehicle->m_Constraints->m_Constraints.count;
		for (int iConstraint = 0; iConstraint < p_pXtVehicle->m_Constraints->m_Constraints.count; iConstraint++)
		{
			if (!p_pXtVehicle->m_Constraints->m_Constraints[iConstraint]->m_Part)
				continue;

			E_PartCode cPartCode = p_pXtVehicle->m_Constraints->m_Constraints[iConstraint]->m_Part->m_PartCode;
			E_PartSubCode cPartSubCode = p_pXtVehicle->m_Constraints->m_Constraints[iConstraint]->m_Part->m_PartSubCode;

			if (p_pXtVehicle->m_Constraints->m_Constraints[iConstraint]->m_Part->m_PartCode == pPart->m_PartCode &&
				p_pXtVehicle->m_Constraints->m_Constraints[iConstraint]->m_Part->m_PartSubCode == pPart->m_PartSubCode)
			{
				Vehicle::XtConstraintBase* pBase = p_pXtVehicle->m_Constraints->m_Constraints[iConstraint];

				SharedPhysics::C_Constraint* pConstraint = CreateConstraint(	pPart,
																							pBase,
																							pPart->m_pPhysicsScene->m_pGraphManager,
																							p_pCollisionBody, 
																							pPart->m_pPhysicsBody, 
																							p_VehicleMatrix);
				if (pConstraint)
				{
					pPart->AddConstraint(pConstraint);
					pConstraint->SetScale(0.0f);
					pPart->m_pPhysicsScene->AddDebugConstraint(pConstraint);
					m_ConstrainedParts.PushBack(pPart);
				}
			}
		}

		//pPart->ActivateConstraints();
	}
}


static void TestCarPartAgainstKillPlane(C_Part* pNode)
{
	if (pNode->m_pPhysicsBody && pNode->m_bActive && pNode->m_pPhysicsBody->Position().Y() < (float)g_debugCarPartKillPLane)
	{
		if (!pNode->m_Damage.m_Detached && pNode->m_Damage.m_HingeActive)
		{
			LOGWARN(g_VehiclePartLog, "Vehicle part constraint gone mental, need to handle this!");

			if (pNode->m_CurrentConstraint < pNode->m_pConstraints.Size())
			{
				//Sort it out!
				pNode->m_pConstraints[pNode->m_CurrentConstraint]->ZeroWarmStartValues();
			}
		}
		else if (pNode->m_Damage.m_Detached)
		{
			LOGWARN(g_VehiclePartLog, "Vehicle part fell through world! Making Static");

			pNode->m_pPhysicsBody->MakeStatic();
		}
	}
	//if (pNode->m_pPhysicsBody && pNode->m_pPhysicsBody->IsActive() && pNode->m_pPhysicsBody->Position().Y() < (float)g_debugCarPartKillPLane)
	//if (pNode->m_pPhysicsBody && pNode->m_bActive && pNode->m_pPhysicsBody->Position().Y() < (float)g_debugCarPartKillPLane)
	//{
	//	LOGWARN(g_VehiclePartLog, "Killing car part has fallen off the world");		
	//	pNode->m_pConstraints[0]->ZeroWarmStartValues();

	//	//pNode->DeactivateConstraints();

	//	//if (pNode->m_bActive)//pNode->m_pPhysicsBody->IsActive())
	//	//{
	//	//	pNode->m_pPhysicsBody->Deactivate();
	//	//	pNode->m_bActive = false;

	//	//	if (pNode->m_bAddedToGravity)
	//	//	{
	//	//		pNode->m_pPhysicsScene->TemporarilyRemoveBody(pNode->m_pPhysicsBody);
	//	//		pNode->m_bAddedToGravity = false;
	//	//	}
	//	//}
	//}
}

//Debug::Float g_debugPartBaumgarte("Vehicle.Damage.Parts.Baumgarte", 0.8f, Debug::Range(0.0f, 10.0f), Debug::Step(0.001f));

/*
===============
NAME: Update

NOTE: Updates
===============
*/

void C_PartList::Update(C_ModelInput* p_pModelInput, float p_fTimeStep)
{
	PERF_EVENT("C_PartList.Update");

	const F32 fRecipSpeedRange = Maths::Reciprocal( p_pModelInput->m_PartShakeParams.m_SpeedRange );
	const F32 fNoiseSpeedScale = Maths::Clamp((p_pModelInput->m_fSpeed - p_pModelInput->m_PartShakeParams.m_MinSpeed) * fRecipSpeedRange, 0.0f, 1.0f);

	F32 fHeadLightIntensity = 0.f;
	F32 fNumHeadLights = 0.f;
	F32 fRearLightIntensity = 0.f;
	F32 fBreakLightIntensity = 0.f;
	F32 fReverseLightIntensity = 0.f;
	const U32 iSize = m_ActiveParts.Size();

	for (int iNode = 0; iNode < iSize; iNode++)
	{
		C_Part* pNode = m_ActiveParts[iNode].m_pNode;
		DCBT( 128, &m_ActiveParts[iNode]);

		// damage, material parameters, and part code are all before 0x100
		//
		// TODO think of clever way to avoid branch?
		if( iNode < ( iSize - 1 ) )
		{
			C_Part* pNextNode = m_ActiveParts[iNode + 1].m_pNode;
			DCBT( 0x80, pNextNode );
			DCBT( 0x100, pNextNode );
		}

		//if (g_debugPartVibrateScaleEnable && pNode->m_Damage.m_HingeActive)	
		//{
		//	float fNoise = 0.0f;
		//	
		//	if (g_debugPartVibrateScaleOverride > 0.0f)
		//		fNoise = (float)g_PartVibrateRandom.FRand(-p_pModelInput->m_PartShakeParams.m_Torque, p_pModelInput->m_PartShakeParams.m_Torque) * g_debugPartVibrateScaleOverride;
		//	else
		//		fNoise = (float)g_PartVibrateRandom.FRand(-p_pModelInput->m_PartShakeParams.m_Torque, p_pModelInput->m_PartShakeParams.m_Torque) * fNoiseSpeedScale;
		//	pNode->UpdateVibration(fNoise);
		//}

		if (pNode->m_pDamageBehaviour && pNode->m_DamageBehaviourType == Damage::E_BehaviourType_Light)
			pNode->m_pDamageBehaviour->Update(p_pModelInput, p_fTimeStep);	
	
		//Accumulate light intensities if needed
		float intensity = 1.0f;

		if (pNode->m_pMaterialParameters && 
			pNode->m_MaterialParametersType == Vehicle::E_MaterialType_Light)
		{
			Vehicle::C_MaterialLight* pMaterial = (Vehicle::C_MaterialLight*)pNode->m_pMaterialParameters;
			intensity = pMaterial->Intensity();
		}

		switch(m_ActiveParts[iNode].m_PartCode)
		{
		case Vehicle::E_PartCode_HeadLight:
			fHeadLightIntensity += intensity;
			fNumHeadLights += 1.0f;
			break;
		case Vehicle::E_PartCode_BrakeLight:
		case Vehicle::E_PartCode_RearAndBrakeLight:
			fBreakLightIntensity += intensity;
			break;	
		case Vehicle::E_PartCode_RearLight:
			fRearLightIntensity += intensity;
			break;	
		case Vehicle::E_PartCode_ReverseLight:
			fReverseLightIntensity += intensity;
			break;	
		}
	}

	m_HeadLightIntensity = Maths::Clamp(fHeadLightIntensity * Maths::Reciprocal(fNumHeadLights), 0.f, 1.f);
	m_BreakLightIntensity = Maths::Clamp(fBreakLightIntensity, 0.f, 1.f);
	m_RearLightIntensity = Maths::Clamp(fRearLightIntensity, 0.f, 1.f);
	m_ReverseLightIntensity = Maths::Clamp(fReverseLightIntensity, 0.f, 1.f);


	m_ControlPointHandler.Update();

	//if(g_debugPartVibrateScaleEnable)
	//{
		for (S32 iPart = 0; iPart < m_ConstrainedParts.Size(); iPart++)
		{
			m_ConstrainedParts[iPart]->UpdateVibration((float)g_PartVibrateRandom.FRand(-p_pModelInput->m_PartShakeParams.m_Torque, p_pModelInput->m_PartShakeParams.m_Torque) * fNoiseSpeedScale);
			SharedPhysics::C_Constraint::E_ConstraintState cState = m_ConstrainedParts[iPart]->m_pConstraints[0]->GetConstraintState();
			m_ConstrainedParts[iPart]->m_CurrentConstraintState = SharedPhysics::C_Constraint::E_Normal;
			if (cState != SharedPhysics::C_Constraint::E_Normal && cState != m_ConstrainedParts[iPart]->m_PreviousConstraintState)
			{
				//The constraint has either opened or closed...
				m_ConstrainedParts[iPart]->m_CurrentConstraintState = cState;
			}
			m_ConstrainedParts[iPart]->m_PreviousConstraintState = cState;		
		}
	//}
	
	/*
	for (S32 iPart = 0; iPart < m_PhysicsParts.Size(); iPart++)
	{
		//m_PhysicsParts[iPart]->m_pPhysicsBody->Baumgarte((float)g_debugPartBaumgarte);
		TestCarPartAgainstKillPlane(m_PhysicsParts[iPart]);
	}
	*/

}

Debug::Bool g_debugSetParams("Vehicle.SetParams", false);

/*
===============
NAME: RecalcUpdatePartList

NOTE: Recalculate the list of components that need updating every frame
===============
*/
void C_PartList::RecalcUpdatePartList()
{
	m_PartsToUpdate.Clear();
	int iSize = m_ActiveParts.Size();
	for (int iNode = 0; iNode < iSize; iNode++)
	{
		C_Part* pNode = m_ActiveParts[iNode].m_pNode;

		assert(pNode && pNode->m_pModelInstance && pNode->m_pElement);

		if( iNode < ( iSize - 1 ) )
		{
			DCBT( 0x80, m_ActiveParts[iNode + 1].m_pNode );
			DCBT( 0x100, m_ActiveParts[iNode + 1].m_pNode );
		}

		bool updateMaterial = false;
		if (pNode->m_pMaterialParameters && pNode->m_MaterialParametersUpdate)
			updateMaterial = true;

		//OK, have to remove this optimisation as things aren't rendering in the correct place...
		//if (!pNode->m_pPhysicsBody && !updateMaterial)
		//	continue;	
		//Replace with this temporary one
		//if (!pNode->m_pPhysicsBody && pNode->m_RestMatrix.AW().All_EQ(Maths::HVec3(0.0f, 0.0f, 0.0f)) == 1 && !updateMaterial)
		if (!pNode->m_pPhysicsBody && pNode->m_RestMatrix.AW().All_EQ(Maths::HVec3(0.0f, 0.0f, 0.0f)) == 1 && !updateMaterial && !pNode->m_pMaterialParameters)
			continue;	

		if (pNode->GetVisible() || pNode->Hidden())
		{
			m_PartsToUpdate.PushBack();
			PartToUpdate& partToUpdate = m_PartsToUpdate.Back();
			partToUpdate.pPart = pNode;
			partToUpdate.pBody = pNode->m_pPhysicsBody;
		}
	}

	m_bRecalculatePartUpdateList = false;

}

/*
===============
NAME: CheckUpdatePartList

NOTE: Checks that the component update list is valid
===============
*/
void C_PartList::CheckUpdatePartList()
{
	for (int iNode = 0; iNode < m_ActiveParts.Size(); iNode++)
	{
		C_Part* pNode = m_ActiveParts[iNode].m_pNode;

		if (!pNode || !pNode->m_pModelInstance || !pNode->m_pElement)
			continue;

		bool updateMaterial = false;
		if (pNode->m_pMaterialParameters && pNode->m_MaterialParametersUpdate)
			updateMaterial = true;

		//OK, have to remove this optimisation as things aren't rendering in the correct place...
		//if (!pNode->m_pPhysicsBody && !updateMaterial)
		//	continue;	
		//Replace with this temporary one
		if (!pNode->m_pPhysicsBody && pNode->m_RestMatrix.AW().All_EQ(Maths::Constants<Maths::HVec3>::Zero()) == 1 && !updateMaterial && !pNode->m_pMaterialParameters)
			continue;	

		if (pNode->GetVisible() || pNode->Hidden())
		{
			// must be in list
			int i;
			for (i=0; i<m_PartsToUpdate.Size(); i++)
			{
				if (m_PartsToUpdate[i].pPart == pNode)
					break;
			}
			//[ChadW] - Knock these out as not got time to fix right now
			//assert(i < m_PartsToUpdate.Size());	// if this goes out of sync, it means the list needs recalculating
		}
		//[ChadW] - Knock these out as not got time to fix right now
		/*else
		{
			// mustn't be in list
			int i;
			for (i=0; i<m_PartsToUpdate.Size(); i++)
			{
				assert(m_PartsToUpdate[i] != pNode); // if this goes out of sync, it means the list needs recalculating
			}
		}*/
	}
}

/*
===============
NAME: UpdateMatrix

NOTE: Updates matrices for a single component node
===============
*/
 

void C_PartList::UpdateMatrix(C_Part *pNode, Maths::HMat43 &p_Matrix, const SceneGraph::C_VisibleDetect* pVisDetect, Maths::HMat43* pOriginalMatrix, bool resetEvent)
{	
	DCBT(0, pNode->m_pWorldMatrix);
	
	if (!resetEvent && pNode->m_pPhysicsBody && (pNode->m_Damage.m_Detached || pNode->m_Damage.m_HingeActive))
	{
		if (pOriginalMatrix)
		{
			Maths::HMat43 localMat;
			if (!pNode->m_bFrozen)
				localMat = pNode->m_pPhysicsBody->ElementMatrix() * pOriginalMatrix->InverseRotTrans();
			else
				localMat = pNode->m_FrozenMatrix;
				
			pNode->m_Matrix = localMat * p_Matrix;

		}
		else
		{
			if (pNode->m_pParent && !pNode->m_Damage.m_Detached)
				pNode->m_Matrix = pNode->m_RestMatrix * pNode->m_pParent->m_Matrix;		
			else
				pNode->m_Matrix = pNode->m_pPhysicsBody->ElementMatrix();
		}
	}
	else
	{
		if (pNode->m_pParent && !pNode->m_Damage.m_Detached)
			pNode->m_Matrix = pNode->m_RestMatrix * pNode->m_pParent->m_Matrix;		
		else
			pNode->m_Matrix = pNode->m_RestMatrix * p_Matrix;

		if(pNode->m_pPhysicsBody)
		{
			const Maths::HVec3 zero(Maths::Constants<Maths::HVec3>::Zero());
			pNode->m_pPhysicsBody->ElementMatrix(pNode->m_Matrix);
		#ifndef CFG_SPEED
			// synch for debug rendering?
			pNode->m_pPhysicsBody->SynchronisePosRot();
		#endif
		}
	}

#if !defined( CFG_SPEED )
	if(g_debugDrawPartInTheSameFrame && pNode->m_pPhysicsBody != NULL)
	{
		if(pNode->m_pPhysicsBody->GetShape()->m_ShapeType == Collision::E_Composite_Shape)
		{
			U8 data[1024 * sizeof(void*)];
			SharedPhysics::C_SafeStack<true> stack(data, 1024);
			Collision::C_CollisionFilter allSolid;
			Collision::C_CompositeShape* pShape = (Collision::C_CompositeShape*)pNode->m_pPhysicsBody->GetShape();
			if(pShape->m_NumShapes > 0)
			{
				Gravity::DrawCompositeShape(DbUtil::DefaultDebugRenderSys()->Deferred(),pShape, stack,	&allSolid);
			}
		}
	}
#endif

	if (pNode->m_pMaterialParameters && pNode->m_MaterialParametersUpdate)
	{
		pNode->m_pMaterialParameters->Build();
		pNode->m_MaterialParametersUpdate = false;
	}
	
	if (pNode->m_pWorldMatrix)
	{
		Maths::Store((Maths::HMat44)pNode->m_Matrix, pNode->m_pWorldMatrix);
#if PLATFORM_PC
		WorldRender::Blur2PCInterpRecordStore(pNode->m_pModelInstance, pNode->m_pWorldMatrix, pNode->m_Matrix);	// [Blur2PC] render interpolation
#endif
	}
}


void C_PartList::SetPartsLinearVelocity(const Maths::HVec3 PCREF velocity)
{
	for (S32 i=0; i<m_PhysicsParts.Size(); i++)
	{
		Gravity::C_Body* pBody = m_PhysicsParts[i]->m_pPhysicsBody;
		const Maths::HVec3 newPos(pBody->ElementOffset() * m_PhysicsParts[i]->m_Matrix);
		const Maths::HVec3 linVel = pBody->GetBody().PointVelocity(newPos);

		pBody->GetBody().StoreLinearVelocity(linVel);
	}
}

void C_PartList::ZeroPartsAngularVelocity()
{
	Maths::HVec3 zero(Maths::Constants<Maths::HVec3>::Zero());
	for (S32 i=0; i<m_PhysicsParts.Size(); i++)
	{
		m_PhysicsParts[i]->m_pPhysicsBody->GetBody().StoreAngularVelocity(zero);
	}
}

void C_PartList::ZeroPartsLinearVelocity()
{
	Maths::HVec3 zero(Maths::Constants<Maths::HVec3>::Zero());
	for (S32 i=0; i<m_PhysicsParts.Size(); i++)
	{
		m_PhysicsParts[i]->m_pPhysicsBody->GetBody().StoreLinearVelocity(zero);
	}
}

/*
===============
NAME: UpdateMatrices

NOTE: Updates matrices
===============
*/
void C_PartList::UpdateMatrices(Maths::HMat43 p_Matrix, const SceneGraph::C_VisibleDetect* pVisDetect, bool forceUpdateAll, Maths::HMat43* pOriginalMatrix, bool resetEvent)
{
	if (m_bRecalculatePartUpdateList)
		RecalcUpdatePartList();
#ifndef CFG_SPEED
	else
		CheckUpdatePartList();
#endif

#if !defined( CFG_SPEED )
	bool setParams = g_debugSetParams;

	if (setParams)
	{
		for (int i = 0; i < m_Parts.Size(); i++)
		{
			for (int j = 0; j < m_Parts[i].m_Nodes.Size(); j++)
			{
				if (m_Parts[i].m_Nodes[j].m_pMaterialParameters)
					m_Parts[i].m_Nodes[j].m_pMaterialParameters->Set();
			}
		}

		g_debugSetParams = false;
	}
#endif

	if (forceUpdateAll)
		return; // replicates original behaviour of "if (pNode->GetVisible() && !forceUpdateAll) { do stuff }"

	// run matrix update on pre-calculated list of nodes that need updating
	U32 i;
	if (g_dbPostStabilise)
	{
		const U32 uConstrSize = m_ConstrainedParts.Size();
		for (i = 0; i < uConstrSize; i++)
		{
			if(m_ConstrainedParts[i]->m_Damage.m_HingeActive)
			{	
				SharedPhysics::C_Constraint* pConstraint = m_ConstrainedParts[i]->m_pConstraints[0];
				pConstraint->PostStabilise();
				m_ConstrainedParts[i]->m_pPhysicsBody->SynchroniseState();
			}
		}
	}

	const U32 uUpdateSize = m_PartsToUpdate.Size();
	for (i=0; i<(uUpdateSize-1); i++)
	{
		// the pointers that control the logic in UpdateMatrix
		// all live above the 0x80 and 0x100 offsets of the C_Part
		// pointer, while the destination matrix lives at the 0x00
		// cache line
		DCBT(0x000, m_PartsToUpdate[i+1].pPart);
		DCBT(0x080, m_PartsToUpdate[i+1].pPart);
		DCBT(0x100, m_PartsToUpdate[i+1].pPart);
		DCBT(0x380, m_PartsToUpdate[i+1].pBody);
		DCBT(0x400, m_PartsToUpdate[i+1].pBody);
		DCBT(0x480, m_PartsToUpdate[i+1].pBody);
		UpdateMatrix(m_PartsToUpdate[i].pPart, p_Matrix, pVisDetect, pOriginalMatrix, resetEvent);
	}
	if (i<uUpdateSize)
		UpdateMatrix(m_PartsToUpdate[i].pPart, p_Matrix, pVisDetect, pOriginalMatrix, resetEvent);

	m_ControlPointHandler.SetParameters();
}

/*
===============
NAME: DisableAllParts

NOTE: Disables all parts and removes physics objects
===============
*/
void C_PartList::DisableAllParts()
{
	for (int iPart = 0; iPart < m_ActiveParts.Size(); iPart++)
	{
		C_Part* pNode = m_ActiveParts[iPart].m_pNode;
		if (!pNode)
			continue;

		if (pNode->m_pPhysicsBody && pNode->m_pPhysicsScene && pNode->m_bAddedToGravity)
		{
			pNode->m_pPhysicsScene->TemporarilyRemoveBody(pNode->m_pPhysicsBody);
			pNode->m_bAddedToGravity = false;
		}			
	}
}

/*
===============
NAME: Clear

NOTE: 
===============
*/
void C_PartList::Clear()
{
	for (int iComponent = 0; iComponent < m_Parts.Size(); iComponent++)
	{
		C_PartGroup* pComponent = &m_Parts[iComponent];

		for (int iNode = 0; iNode < pComponent->m_Nodes.Size(); iNode++)
		{
			pComponent->m_Nodes[iNode].m_pConstraints.Clear();
			pComponent->m_Nodes[iNode].m_pModel			= NULL;
			pComponent->m_Nodes[iNode].m_pModelInstance	= NULL;
			pComponent->m_Nodes[iNode].m_pElement		= NULL;
			pComponent->m_Nodes[iNode].m_pWorldMatrix	= NULL;
			pComponent->m_Nodes[iNode].m_pPhysicsBody	= NULL;	
			pComponent->m_Nodes[iNode].m_RestMatrix.SetIdentity();
			pComponent->m_Nodes[iNode].m_pConstraints.Clear();
			pComponent->m_Nodes[iNode].m_pPartList = this;
		}
	}			
}

/*
===============
NAME: GetModelInstance

NOTE: 
===============
*/
WorldRender::C_ModelInstance* C_PartList::GetModelInstance(E_PartCode p_PartCode, E_PartSubCode p_LocCode)
{
	C_PartGroup* pComponent = &m_Parts[p_PartCode];

	return const_cast<WorldRender::C_ModelInstance*>(pComponent->m_Nodes[p_LocCode].m_pModelInstance);
}

Debug::Float g_debugSpringAngDampCo("Vehicle.Parts.Spring.AngularDampCoeff", 1.0f, Debug::Range(0.01f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugSpringAngSprCo("Vehicle.Parts.Spring.AngularSpringCoeff", 1.0f, Debug::Range(0.01f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugSpringLinDampCo("Vehicle.Parts.Spring.LinearDampCoeff", 1.0f, Debug::Range(0.01f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugSpringLinSprCo("Vehicle.Parts.Spring.LinearSpringCoeff", 1.0f, Debug::Range(0.01f, 1.0f), Debug::Step(0.001f));

/*
===============
NAME: CreateConstraint

NOTE: CreateConstraint
===============
*/

Maths::HVec3 LocalShapeSpace(const Maths::HVec3 PCREF position, Gravity::C_Body* pBody)
{
	return position - pBody->ElementOffset();
}

SharedPhysics::C_Constraint* C_PartList::CreateConstraint(	C_Part* p_pNode,
																XtConstraintBase* p_pConstraint,
																Solver2::C_ConstraintGraphManager* p_pGraphManager,
																Gravity::C_Body* p_pBodyA, Gravity::C_Body* p_pBodyB,
																Maths::HMat43 p_VehicleMatrix)
{
	if (!p_pConstraint || !p_pBodyA || !p_pBodyB)
		return NULL;

	SharedPhysics::C_Constraint* pConstraint = NULL;

	switch (p_pConstraint->m_Type)
	{
	case E_ConstraintType_SphericalJoint:
		{
			XtSphericalJoint* pJoint = (XtSphericalJoint*)p_pConstraint;

			const Maths::HVec3 _position(	Load(pJoint->m_Position));
			const Maths::HVec3 position(LocalShapeSpace(_position, p_pBodyA));
			
			const Maths::HMat33 rotA(p_pBodyA->Orientation());
			const Maths::HVec3 posA(p_pBodyA->Position());
			Maths::HMat43 matA(rotA, posA);
			//Maths::HMat43 matA = p_VehicleMatrix;
			const Maths::HMat43 invMatA = matA.InverseRotTrans();
			const Maths::HVec3 anchorPoint1 = position;
			
			const Maths::HMat33 rotB	= p_pBodyB->Orientation();
			const Maths::HVec3 posB	= p_pBodyB->Position();
			const Maths::HMat43 matB(rotB, posB);
			const Maths::HMat43 invMatB = matB.InverseRotTrans();

			const Maths::HVec3 worldPos = position * matA;
			const Maths::HVec3 localOffset = worldPos * invMatB;
			const Maths::HVec3 anchorPoint2 = localOffset;

			Gravity::SphericalJointDescriptor desc;
			desc.SetBreakError(0.1f);
			desc.DefineAnchorPoints(anchorPoint1, anchorPoint2);
			pConstraint = desc.CreateConstraint(*p_pGraphManager, p_pBodyA, p_pBodyB);
			pConstraint->SetBodyAMassScale(0.f);
			//pConstraint->NormaliseMassRatios(0.f, 0.f);
		}
		break;
	case E_ConstraintType_RevoluteJoint:
		{
			XtRevoluteJoint* pJoint = (XtRevoluteJoint*)p_pConstraint;

			const Maths::HVec3 _position(Load(pJoint->m_Position));

			const Maths::HVec3 position(LocalShapeSpace(_position, p_pBodyA));
			const Maths::HVec3 jointAxis(Load(pJoint->m_Axis));
			const Maths::HMat33 rotA	= p_pBodyA->Orientation();
			const Maths::HVec3 posA	= p_pBodyA->Position();
			const Maths::HMat43 matA(rotA, posA);
			//Maths::HMat43 matA = p_VehicleMatrix;
			const Maths::HMat43 invMatA = matA.InverseRotTrans();
			const Maths::HVec3 anchorPoint1 = position;
			
			const Maths::HMat33 rotB	= p_pBodyB->Orientation();
			const Maths::HVec3 posB	= p_pBodyB->Position();
			const Maths::HMat43 matB(rotB, posB);
			const Maths::HMat43 invMatB = matB.InverseRotTrans();

			const Maths::HVec3 worldPos = position * matA;
			const Maths::HVec3 localOffset = worldPos * invMatB;
			const Maths::HVec3 anchorPoint2 = localOffset;
			
			const Maths::HVec3 jointAxisB = (jointAxis * matA.Rotation()) * matB.Rotation().InverseRotation();

			Gravity::RevoluteJointDescriptor desc;
			desc.DefineAnchorPoints(anchorPoint1, anchorPoint2);
			desc.DefineJointAxis(jointAxis, jointAxisB);
			desc.SetBreakError(0.1f);
			pConstraint = desc.CreateConstraint(*p_pGraphManager, p_pBodyA, p_pBodyB);
			pConstraint->SetBodyAMassScale(0.f);
			//pConstraint->NormaliseMassRatios(0.f, 0.f);
		}
		break;
	case E_ConstraintType_RevoluteJointLimit:
		{
			XtRevoluteJointLimit* pJoint = (XtRevoluteJointLimit*)p_pConstraint;

			const Maths::HVec3 _position(Load(pJoint->m_Position));

			const Maths::HVec3 position(LocalShapeSpace(_position, p_pBodyA));

			Maths::HVec3 jointAxis(Load(pJoint->m_Axis));
			jointAxis.Normalise();

			const Maths::HVec3 jointAxis2(p_pBodyB->GetBody().m_OrientationMatrix * (jointAxis * p_pBodyA->GetBody().m_OrientationMatrix));

			float lowLimit			=	pJoint->m_LowJointLimit;
			float highLimit			=	pJoint->m_HighJointLimit;
			
			if (p_pNode->m_pConstraints.Size() == 0)
			{
				p_pNode->m_ConstraintLimits.first	= Maths::DegToRad(lowLimit);
				p_pNode->m_ConstraintLimits.second	= Maths::DegToRad(highLimit);
			}

			SharedPhysics::C_RigidBody& pRigidA = p_pBodyA->GetBody();
			SharedPhysics::C_RigidBody& pRigidB = p_pBodyB->GetBody();

			
			const Maths::HVec3 worldCar = pRigidA.m_Position + position* pRigidA.m_OrientationMatrix;
			const Maths::HVec3 anchorPoint2 = (worldCar - pRigidB.m_Position)*pRigidB.m_OrientationMatrix.Inverse();

			const Maths::HVec3 worldBody2 = pRigidB.m_Position + anchorPoint2*pRigidB.m_OrientationMatrix;

			if (lowLimit >= highLimit)
				LOGERROR(g_VehiclePartLog, "Constraint limit issue: lowLimit(%f) >= highLimit(%f)", lowLimit, highLimit);

			Gravity::RevoluteJointLimitDescriptor desc;
			desc.DefineAnchorPoints(position, anchorPoint2);
			desc.DefineJointAxis(jointAxis, jointAxis2);
			desc.DefineJointLimit(lowLimit, highLimit);
			pConstraint = desc.CreateConstraint(*p_pGraphManager, p_pBodyA, p_pBodyB);
			pConstraint->SetBodyAMassScale(0.f);
			p_pNode->m_ConstraintAxis = jointAxis;


		}
		break;
	}

	return pConstraint;
}

/*
===============
NAME: BuildDynamicMaterials

NOTE: Builds dynamic materials
===============
*/
void C_PartList::BuildDynamicMaterials()
{
	for (int iComponent = 0; iComponent < m_Parts.Size(); iComponent++)
	{
		for (int iNode = 0; iNode < m_Parts[iComponent].m_Nodes.Size(); iNode++)
		{
			C_Part* pNode = &m_Parts[iComponent].m_Nodes[iNode];
	
			if (pNode->m_pMaterialParameters)
				pNode->m_pMaterialParameters->Build();
		}
	}
}


/*
===============
NAME: UpdateDynamicMaterials

NOTE: Updates all dynamic materials for the parts
===============
*/
void C_PartList::UpdateDynamicMaterials(XtVehicle* p_pVehicle, XtMaterialDefaults* p_pMaterialDefaults)
{
	for (int iComponent = 0; iComponent < m_Parts.Size(); iComponent++)
	{
		for (int iNode = 0; iNode < m_Parts[iComponent].m_Nodes.Size(); iNode++)
		{
			UpdateDynamicMaterial(p_pVehicle, p_pMaterialDefaults, (E_PartCode)iComponent, (E_PartSubCode)iNode);
		}
	}
}

/*
===============
NAME: UpdateDynamicMaterial

NOTE: Updates the dynamic materials for a specific part
===============
*/
void C_PartList::UpdateDynamicMaterial(XtVehicle* p_pVehicle, XtMaterialDefaults* p_pMaterialDefaults,
											E_PartCode p_PartCode, E_PartSubCode p_SubCode)
{
	C_Part* pNode = &m_Parts[p_PartCode].m_Nodes[p_SubCode];

	XtMaterialBase* pXtMaterial = C_MaterialManager::Instance().GetXtMaterial(	p_pVehicle, p_pMaterialDefaults,
																				p_PartCode, p_SubCode);
			
	if (pNode->m_pMaterialParameters)
	{
		if (!pXtMaterial)
		{
			//Delete game-side material
			pNode->m_pMaterialParameters->Destroy();
			SAFE_DELETE_PTR(pNode->m_pMaterialParameters);

			return;
		}

		if (pXtMaterial->m_Type != pNode->m_MaterialParametersType)
		{
			//Delete game-side material					
			pNode->m_pMaterialParameters->Destroy();
			SAFE_DELETE_PTR(pNode->m_pMaterialParameters);

			//Create new one
			pNode->m_pMaterialParameters = C_MaterialManager::Instance().CreateMaterial(p_pVehicle, p_pMaterialDefaults, 
																						pNode->m_pModelInstance, 
																						p_PartCode, p_SubCode);

			if (pNode->m_pMaterialParameters)
			{
				pNode->m_MaterialParametersType = pNode->m_pMaterialParameters->m_Type;
				pNode->m_MaterialParametersUpdate = true;
				pNode->m_pMaterialParameters->Build();
			}

			return;
		}
		
		C_MaterialManager::Instance().TranslateMaterial(pXtMaterial, pNode->m_pMaterialParameters);

		pNode->m_pMaterialParameters->Build();
	}
	else if (pXtMaterial)
	{
		//Create new material
		pNode->m_pMaterialParameters = C_MaterialManager::Instance().CreateMaterial(p_pVehicle, p_pMaterialDefaults, 
																					pNode->m_pModelInstance, 
																					p_PartCode, p_SubCode);
		
		if (pNode->m_pMaterialParameters)
		{
			pNode->m_MaterialParametersType = pNode->m_pMaterialParameters->m_Type;
			pNode->m_MaterialParametersUpdate = true;
			pNode->m_pMaterialParameters->Build();
		}
	}
}

/*
===============
NAME: CreateDamageBehaviours

NOTE: CreateDamageBehaviours
===============
*/
void C_PartList::CreateDamageBehaviours() 
{
	for (int iNode = 0; iNode < m_ActiveParts.Size(); iNode++)
	{
		C_Part* pNode = m_ActiveParts[iNode].m_pNode;
		
		if (!pNode)
			continue;

		assert(pNode->m_pDamageBehaviour == NULL);

		switch (pNode->m_PartCode)
		{
		case E_PartCode_HeadlightCover:
		case E_PartCode_FogLightCover:
		case E_PartCode_RearLightCover:
			{
				pNode->m_pDamageBehaviour = new Damage::C_BehaviourGlassSmash();
				pNode->m_DamageBehaviourType = pNode->m_pDamageBehaviour->GetType();
				assert(pNode->m_pDamageBehaviour);
				pNode->m_pDamageBehaviour->SetNodePtr(pNode);
			}
			break;
		case E_PartCode_Window:
			{
				if (pNode->m_PartSubCode != E_PartSubCode_Back && pNode->m_PartSubCode != E_PartSubCode_Front)
				{
					pNode->m_pDamageBehaviour = new Damage::C_BehaviourGlassSmash();
					pNode->m_DamageBehaviourType = pNode->m_pDamageBehaviour->GetType();
					assert(pNode->m_pDamageBehaviour);
					pNode->m_pDamageBehaviour->SetNodePtr(pNode);
				}
			}
			break;
		case E_PartCode_HeadLight:
		case E_PartCode_RearLight:
		case E_PartCode_RearAndBrakeLight:
		case E_PartCode_FogLight:
		case E_PartCode_BrakeLight:
		case E_PartCode_ReverseLight:
		case E_PartCode_RunningLight:
		case E_PartCode_SignalLight:
			{
				pNode->m_pDamageBehaviour = new Damage::C_BehaviourLight();
				pNode->m_DamageBehaviourType = pNode->m_pDamageBehaviour->GetType();
				assert(pNode->m_pDamageBehaviour);
				pNode->m_pDamageBehaviour->SetNodePtr(pNode);
			}
			break;
		case E_PartCode_LightCluster:
			{
				pNode->m_pDamageBehaviour = new Damage::C_BehaviourLightCluster();
				pNode->m_DamageBehaviourType = pNode->m_pDamageBehaviour->GetType();
				assert(pNode->m_pDamageBehaviour);
				pNode->m_pDamageBehaviour->SetNodePtr(pNode);
				Damage::C_BehaviourLightCluster* pBehaviour = (Damage::C_BehaviourLightCluster*)pNode->m_pDamageBehaviour;
				if (pNode->m_PartSubCode == E_PartSubCode_FrontLeft)
				{
					pBehaviour->AddChild(&m_Parts[E_PartCode_HeadLight].m_Nodes[E_PartSubCode_FrontLeft]);
					pBehaviour->AddChild(&m_Parts[E_PartCode_HeadLight].m_Nodes[E_PartSubCode_Left]);
					pBehaviour->AddChild(&m_Parts[E_PartCode_FogLight].m_Nodes[E_PartSubCode_FrontLeft]);
					pBehaviour->AddChild(&m_Parts[E_PartCode_FogLight].m_Nodes[E_PartSubCode_Left]);
					pBehaviour->AddChild(&m_Parts[E_PartCode_SignalLight].m_Nodes[E_PartSubCode_FrontLeft]);
					pBehaviour->AddChild(&m_Parts[E_PartCode_SignalLight].m_Nodes[E_PartSubCode_Left]);
					pBehaviour->AddChild(&m_Parts[E_PartCode_RunningLight].m_Nodes[E_PartSubCode_FrontLeft]);
					pBehaviour->AddChild(&m_Parts[E_PartCode_RunningLight].m_Nodes[E_PartSubCode_Left]);
				}
				else if (pNode->m_PartSubCode == E_PartSubCode_FrontRight)
				{
					pBehaviour->AddChild(&m_Parts[E_PartCode_HeadLight].m_Nodes[E_PartSubCode_FrontRight]);
					pBehaviour->AddChild(&m_Parts[E_PartCode_HeadLight].m_Nodes[E_PartSubCode_Right]);
					pBehaviour->AddChild(&m_Parts[E_PartCode_FogLight].m_Nodes[E_PartSubCode_FrontRight]);
					pBehaviour->AddChild(&m_Parts[E_PartCode_FogLight].m_Nodes[E_PartSubCode_Right]);
					pBehaviour->AddChild(&m_Parts[E_PartCode_SignalLight].m_Nodes[E_PartSubCode_FrontRight]);
					pBehaviour->AddChild(&m_Parts[E_PartCode_SignalLight].m_Nodes[E_PartSubCode_Right]);
					pBehaviour->AddChild(&m_Parts[E_PartCode_RunningLight].m_Nodes[E_PartSubCode_FrontRight]);
					pBehaviour->AddChild(&m_Parts[E_PartCode_RunningLight].m_Nodes[E_PartSubCode_Right]);
				}
				else if (pNode->m_PartSubCode == E_PartSubCode_BackLeft)
				{
					pBehaviour->AddChild(&m_Parts[E_PartCode_RearLight].m_Nodes[E_PartSubCode_BackLeft]);
					pBehaviour->AddChild(&m_Parts[E_PartCode_RearLight].m_Nodes[E_PartSubCode_Left]);
					pBehaviour->AddChild(&m_Parts[E_PartCode_FogLight].m_Nodes[E_PartSubCode_BackLeft]);
					pBehaviour->AddChild(&m_Parts[E_PartCode_SignalLight].m_Nodes[E_PartSubCode_BackLeft]);
					pBehaviour->AddChild(&m_Parts[E_PartCode_RunningLight].m_Nodes[E_PartSubCode_BackLeft]);
					pBehaviour->AddChild(&m_Parts[E_PartCode_ReverseLight].m_Nodes[E_PartSubCode_BackLeft]);
					pBehaviour->AddChild(&m_Parts[E_PartCode_ReverseLight].m_Nodes[E_PartSubCode_Left]);
				}
				else if (pNode->m_PartSubCode == E_PartSubCode_BackRight)
				{
					pBehaviour->AddChild(&m_Parts[E_PartCode_RearLight].m_Nodes[E_PartSubCode_BackRight]);
					pBehaviour->AddChild(&m_Parts[E_PartCode_RearLight].m_Nodes[E_PartSubCode_Right]);
					pBehaviour->AddChild(&m_Parts[E_PartCode_FogLight].m_Nodes[E_PartSubCode_BackRight]);
					pBehaviour->AddChild(&m_Parts[E_PartCode_SignalLight].m_Nodes[E_PartSubCode_BackRight]);
					pBehaviour->AddChild(&m_Parts[E_PartCode_RunningLight].m_Nodes[E_PartSubCode_BackRight]);
					pBehaviour->AddChild(&m_Parts[E_PartCode_ReverseLight].m_Nodes[E_PartSubCode_BackRight]);
					pBehaviour->AddChild(&m_Parts[E_PartCode_ReverseLight].m_Nodes[E_PartSubCode_Right]);
				}
			}
			break;
		}
	}
}		

Debug::Bool g_debugVehicleAxisHelper("Vehicle.DebugRender.DrawHelperAxis", false, Debug::Persist());
Debug::Float g_debugAxisHelperHeight("Vehicle.DebugRender.AxisHelperHeight", 1.5f, Debug::Range(-1.0f, 3.0f), Debug::Step(0.001f));
Debug::Bool g_debugVehicleDrawMeshSpawners("Vehicle.DebugRender.DrawMeshSpawners", false, Debug::Persist());
Debug::Bool g_debugVehicleDrawConstraints("Vehicle.DebugRender.DrawConstraints", false, Debug::Persist());

/*
===============
NAME: UpdateDynamicMaterial

NOTE: Updates the dynamic materials for a specific part
===============
*/
void C_PartList::DebugDraw(Maths::HMat43& p_Matrix)
{
	DbUtil::I_DebugRender *pDebugRenderer = DbUtil::DefaultDebugRenderSys()->Deferred();

	for (int i = 0; i < m_ActiveParts.Size(); i++)
	{
		if (!m_ActiveParts[i].m_pNode)
			continue;

		if (g_debugVehicleDrawConstraints)
		{
			for (int j = 0; j < m_ActiveParts[i].m_pNode->m_pConstraints.Size(); j++)
				m_ActiveParts[i].m_pNode->m_pConstraints[j]->DebugRender(pDebugRenderer);			
		}

		if (g_debugVehicleDrawMeshSpawners && m_ActiveParts[i].m_pNode->m_pSpawnMesh)
			m_ActiveParts[i].m_pNode->m_pSpawnMesh->Display(p_Matrix);
	}

	if (g_debugVehicleAxisHelper)
	{
		Maths::HMat43 mat = p_Matrix;
		Maths::HVec3 vPos(mat.AW().X(), mat.AW().Y() + (float)g_debugAxisHelperHeight, mat.AW().Z());
		mat.AW(vPos);
		pDebugRenderer->Draw3dMatrix(mat);
	}
}

/*
===============
NAME: FreezeParts

NOTE: Freeze/resume parts
===============
*/
void C_PartList::FreezeParts(bool p_bFreeze, const Maths::HMat43& p_ParentMatrix)
{
	for (S32 iPart = 0; iPart < m_ConstrainedParts.Size(); iPart++)
	{
		C_Part* pPart = m_ConstrainedParts[iPart];
		if (!pPart)
			continue;

		pPart->Freeze(p_bFreeze, p_ParentMatrix);
	}
}

Debug::Float g_debugMeshSpawnReduction("Vehicle.Damage.Particles.MeshSpawnReduction", 0.15f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

/*
===============
NAME: CreateSpawnMesh

NOTE: Creates spawn mesh for particles 
===============
*/
void C_PartList::CreateSpawnMesh(C_Part* p_pPart)
{
	if (!p_pPart || p_pPart->m_pSpawnMesh || !p_pPart->m_pElement)
		return;

	if (p_pPart->m_PartCode == E_PartCode_Window || 
		p_pPart->m_PartCode == E_PartCode_HeadlightCover || 
		p_pPart->m_PartCode == E_PartCode_FogLightCover ||
		p_pPart->m_PartCode == E_PartCode_RearLightCover ||		
		p_pPart->m_PartCode == E_PartCode_Tyre ||
		(p_pPart->m_PartCode >= E_PartCode_HeadLight && p_pPart->m_PartCode <= E_PartCode_RearAndBrakeLight))
	{
		p_pPart->m_pSpawnMesh = new DynamicEffect::Particle::C_SpawnMesh;
		BIZ_ASSERT(p_pPart->m_pSpawnMesh);
		if (!p_pPart->m_pSpawnMesh)
			return;

	/*	Maths::HVec3 vCentre	= p_pPart->m_RestMatrix.AW() + p_pPart->m_pElement->GetLocalBounds().Centre();
		Maths::HVec3 vMin		= p_pPart->m_RestMatrix.AW() + p_pPart->m_pElement->GetLocalBounds().Min();
		Maths::HVec3 vMax		= p_pPart->m_RestMatrix.AW() + p_pPart->m_pElement->GetLocalBounds().Max();*/

		Maths::HVec3 vCentre	= p_pPart->m_pElement->GetLocalBounds().Centre();
		Maths::HVec3 vMin		= p_pPart->m_pElement->GetLocalBounds().Min();
		Maths::HVec3 vMax		= p_pPart->m_pElement->GetLocalBounds().Max();

		const Maths::HVec3 zero(Maths::Constants<Maths::HVec3>::Zero());

		Maths::HVec3 vP0(zero);
		Maths::HVec3 vP1(zero);
		Maths::HVec3 vP2(zero);
		Maths::HVec3 vP3(zero);

	/*	Maths::HVec3 vP0(0.0f, 0.0f, 0.0f);
		Maths::HVec3 vP1(0.0f, 0.0f, 0.0f);
		Maths::HVec3 vP2(0.0f, 0.0f, 0.0f);
		Maths::HVec3 vP3(0.0f, 0.0f, 0.0f);*/

		bool bFront = false;
		bool bBack	= false;
		bool bLeft	= false;
		bool bRight = false;
		bool bTop	= false;

		if (p_pPart->m_PartCode == E_PartCode_Window)
		{
			if (p_pPart->m_PartSubCode == E_PartSubCode_Front)
				bFront = true;
			else if (p_pPart->m_PartSubCode == E_PartSubCode_Back)
				bBack = true;
			else if (p_pPart->m_PartSubCode == E_PartSubCode_Left || p_pPart->m_PartSubCode == E_PartSubCode_FrontLeft || p_pPart->m_PartSubCode == E_PartSubCode_BackLeft)
				bLeft = true;
			else if (p_pPart->m_PartSubCode == E_PartSubCode_Right || p_pPart->m_PartSubCode == E_PartSubCode_FrontRight || p_pPart->m_PartSubCode == E_PartSubCode_BackRight)
				bRight = true;
			else if (p_pPart->m_PartSubCode == E_PartSubCode_Top)
				bTop = true;
		}
		else if (p_pPart->m_PartCode == E_PartCode_HeadlightCover || p_pPart->m_PartCode == E_PartCode_FogLightCover)
		{
			if (p_pPart->m_PartSubCode == E_PartSubCode_Left || p_pPart->m_PartSubCode == E_PartSubCode_FrontLeft || 
				p_pPart->m_PartSubCode == E_PartSubCode_Right || p_pPart->m_PartSubCode == E_PartSubCode_FrontRight)
				bFront = true;
			else if (p_pPart->m_PartSubCode == E_PartSubCode_BackLeft || p_pPart->m_PartSubCode == E_PartSubCode_BackRight)
				bBack = true;
		}
		else if (p_pPart->m_PartCode == E_PartCode_RearLightCover)
		{
			if (p_pPart->m_PartSubCode == E_PartSubCode_Left || p_pPart->m_PartSubCode == E_PartSubCode_BackLeft || 
				p_pPart->m_PartSubCode == E_PartSubCode_Right || p_pPart->m_PartSubCode == E_PartSubCode_BackRight)
				bBack = true;
		}
		else if (p_pPart->m_PartCode == E_PartCode_Tyre)
		{
			vCentre	= vCentre + p_pPart->m_RestMatrix.AW();
			vMin	= vMin + p_pPart->m_RestMatrix.AW();
			vMax	= vMax + p_pPart->m_RestMatrix.AW();

			if (p_pPart->m_PartSubCode == E_PartSubCode_FrontLeft || p_pPart->m_PartSubCode == E_PartSubCode_BackLeft)
				bLeft = true;
			else if (p_pPart->m_PartSubCode == E_PartSubCode_FrontRight || p_pPart->m_PartSubCode == E_PartSubCode_BackRight)
				bRight = true;
		}
		else if (p_pPart->m_PartCode == E_PartCode_HeadLight || p_pPart->m_PartCode == E_PartCode_FogLight)
		{
			bFront = true;
		}
		else if (p_pPart->m_PartCode == E_PartCode_RearLight || p_pPart->m_PartCode == E_PartCode_BrakeLight ||
				 p_pPart->m_PartCode == E_PartCode_ReverseLight || p_pPart->m_PartCode == E_PartCode_RearAndBrakeLight)
		{
			bBack = true;
		}
		else if (p_pPart->m_PartCode == E_PartCode_RunningLight || p_pPart->m_PartCode == E_PartCode_SignalLight)
		{
			if (p_pPart->m_PartSubCode == E_PartSubCode_Left || p_pPart->m_PartSubCode == E_PartSubCode_FrontLeft || p_pPart->m_PartSubCode == E_PartSubCode_BackLeft)
				bLeft = true;
			else if (p_pPart->m_PartSubCode == E_PartSubCode_Right || p_pPart->m_PartSubCode == E_PartSubCode_FrontRight || p_pPart->m_PartSubCode == E_PartSubCode_BackRight)
				bRight = true;
		}

		if (bFront)
		{
			vP0 = Maths::HVec3(vMax.X(), vMax.Y(), vMin.Z());
			vP1 = Maths::HVec3(vMax.X(), vMin.Y(), vMax.Z());
			vP2 = Maths::HVec3(vMin.X(), vMin.Y(), vMax.Z());
			vP3 = Maths::HVec3(vMin.X(), vMax.Y(), vMin.Z());

		}
		else if (bBack)
		{
			vP0 = Maths::HVec3(vMin.X(), vMax.Y(), vMax.Z());
			vP1 = Maths::HVec3(vMin.X(), vMin.Y(), vMin.Z());
			vP2 = Maths::HVec3(vMax.X(), vMin.Y(), vMin.Z());
			vP3 = Maths::HVec3(vMax.X(), vMax.Y(), vMax.Z());

		}
		else if (bLeft)
		{
			vP0 = Maths::HVec3(vMin.X(), vMax.Y(), vMax.Z());
			vP1 = Maths::HVec3(vMin.X(), vMin.Y(), vMax.Z());
			vP2 = Maths::HVec3(vMin.X(), vMin.Y(), vMin.Z());
			vP3 = Maths::HVec3(vMin.X(), vMax.Y(), vMin.Z());
		}
		else if (bRight)
		{
			vP0 = Maths::HVec3(vMax.X(), vMax.Y(), vMin.Z());
			vP1 = Maths::HVec3(vMax.X(), vMin.Y(), vMin.Z());
			vP2 = Maths::HVec3(vMax.X(), vMin.Y(), vMax.Z());
			vP3 = Maths::HVec3(vMax.X(), vMax.Y(), vMax.Z());
		}
		else if (bTop)
		{
			vP0 = Maths::HVec3(vMin.X(), vMax.Y(), vMax.Z());
			vP1 = Maths::HVec3(vMin.X(), vMax.Y(), vMin.Z());
			vP2 = Maths::HVec3(vMax.X(), vMax.Y(), vMin.Z());
			vP3 = Maths::HVec3(vMax.X(), vMax.Y(), vMax.Z());
		}
		else
		{
			SAFE_DELETE_PTR(p_pPart->m_pSpawnMesh)
			LOGINFO(g_VehiclePartLog, "Didn't find a valid code, deleting m_pSpawnMesh - partCode(%d), partSubCode(%d)", p_pPart->m_PartCode, p_pPart->m_PartSubCode);
			return;
		}

		float fReduction = (float)g_debugMeshSpawnReduction;

		vP0 = vP0 - (vP0 - vCentre) * fReduction;
		vP1 = vP1 - (vP1 - vCentre) * fReduction;
		vP2 = vP2 - (vP2 - vCentre) * fReduction;
		vP3 = vP3 - (vP3 - vCentre) * fReduction;

		Maths::HVec3 vN = ((vP3 - vP0) ^ (vP1 - vP0));
		vN.Normalise();

		p_pPart->m_pSpawnMesh->AddVertex(vP0, vN);
		p_pPart->m_pSpawnMesh->AddVertex(vP1, vN);
		p_pPart->m_pSpawnMesh->AddVertex(vP2, vN);
		p_pPart->m_pSpawnMesh->AddVertex(vP3, vN);
		p_pPart->m_pSpawnMesh->AddTriangle(0, 1, 2);
		p_pPart->m_pSpawnMesh->AddTriangle(0, 2, 3);	
		p_pPart->m_pSpawnMesh->CalculateArea();
	}
}

/*
===============
NAME: CreateSpawnSpline

NOTE: Creates spawn spline for particles 
===============
*/
//void C_PartList::CreateSpawnMesh(C_Part* p_pPart)
//{
//
//	Curve::C_ActorCurve3D* m_pSpawnSpline = new Curve::C_ActorCurve3D();
//
//	Curve::C_HermiteSplineFlags flags;
//	flags.calcKnotU = Curve::E_ProcessU_ArcLengthLinear;
//	flags.calcTangents = true;
//	Curve::C_Hermite3D* pSplineEvaluator = new Curve::C_Hermite3D(flags);
//
//	pSplineEvaluator->Evaluate(
//	m_pSpawnSpline->SetCurve(
//}

/*
===============
NAME: PrintPhysicsParts

NOTE: Outputs parts with physics meshes
===============
*/
void C_PartList::PrintPhysicsParts()
{
	bool bPartsFound = false;
	for (S32 iPart = 0; iPart < m_ActiveParts.Size(); iPart++)
	{
		if (!m_ActiveParts[iPart].m_pNode || !m_ActiveParts[iPart].m_pNode->m_pPhysicsBody)
			continue;

		bPartsFound = true;

		C_Part* pPart = m_ActiveParts[iPart].m_pNode;
		Util::C_String partName("None");
		Util::C_String partSubName("None");
	
		for (S32 iPartName = 0; iPartName < E_PartCode_Max - 1; iPartName++)
		{
			if (s_PartNames[iPartName].m_Code == pPart->m_PartCode)
				partName = s_PartNames[iPartName].m_Name;
		}
		
		for (S32 iSubName = 0; iSubName < NUM_PARTSUBCODE_STRINGS; iSubName++)
		{
			if (s_LocationNames[iSubName].m_Code == pPart->m_PartSubCode)
				partSubName = s_LocationNames[iSubName].m_Name;
		}
		LOGINFO(g_VehiclePartLog, "Physics Body: %s_%s", partName.Chars(), partSubName.Chars());
	}

	if (!bPartsFound)
		LOGINFO(g_VehiclePartLog, "Vehicle has no parts with collision mesh!");
}

/* 
===============================================================
					class C_VehicleWindow

NOTES: 
===============================================================
*/

Debug::Bool g_debugWindowWindUp("Vehicle.Windows.Up", false);
Debug::Bool g_debugWindowWindDown("Vehicle.Windows.Down", false);
Debug::Bool g_debugWindowDebugPos("Vehicle.Windows.Override", false);

Debug::Float g_debugWindowPos("Vehicle.Windows.Pos", 0.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

/*
===============
NAME: C_VehicleWindow

NOTE: C_VehicleWindow
===============
*/
C_VehicleWindow::C_VehicleWindow()
{
	m_Position				= 0.0f;
	m_TargetPosition		= 0.0f;
	m_Speed					= 0.0f;
	m_FinalRotation			= 0.0f;

	m_FinalPosition			= Maths::Constants<Maths::HVec3>::Zero();

	m_pPart					= NULL;

	m_Overriden				= false;
}

/*
===============
NAME: Initialise

NOTE: Initialises window
===============
*/
void C_VehicleWindow::Initialise(XtWindow* p_pXtWindow, C_Part* p_pPart)
{
	if (!p_pXtWindow || !p_pPart)
		return;
	
	if (m_pPart != p_pPart)
		m_RestMatrix		= p_pPart->m_RestMatrix;

	m_pPart = p_pPart;

	m_FinalPosition			= Maths::HVec3(p_pXtWindow->m_FinalPosition.vx, p_pXtWindow->m_FinalPosition.vy, p_pXtWindow->m_FinalPosition.vz);
	m_FinalRotation			= Maths::DegToRad(p_pXtWindow->m_FinalRotation);
	m_Speed					= p_pXtWindow->m_Speed;	

	m_Position				= 0.0f;
	m_TargetPosition		= 0.0f;
}

/*
===============
NAME: SetPosition

NOTE: Sets window position
===============
*/
void C_VehicleWindow::SetPosition(float p_fPosition)
{
	m_Position			= Maths::Clamp(p_fPosition, 0.0f, 1.0f);
	m_TargetPosition	= m_Position;
	m_Overriden			= true;
	if (m_pPart)
		m_pPart->SetPermanentVisibility(1);
}

/*
===============
NAME: WindUp

NOTE: Winds window up
===============
*/
void C_VehicleWindow::WindUp()
{
	m_TargetPosition	= 0.0f;
}

/*
===============
NAME: WindDown

NOTE: Winds window down
===============
*/
void C_VehicleWindow::WindDown()
{
	m_TargetPosition	= 1.0f;
}

/*
===============
NAME: Updates

NOTE: Updates window movement
===============
*/
void C_VehicleWindow::Update(float p_fTimeStep)
{
	if (!m_pPart)
		return;

	bool updateMatrix = false;
	if (m_Position < m_TargetPosition)
	{
		m_Position += m_Speed * p_fTimeStep;
		m_Position = Maths::Min(m_TargetPosition, m_Position);
		updateMatrix = true;
	}
	else if (m_Position > m_TargetPosition)
	{
		m_Position -= m_Speed * p_fTimeStep;
		m_Position = Maths::Max(m_TargetPosition, m_Position);
		updateMatrix = true;
	}

#ifndef CFG_GOLD
	if (g_debugWindowWindUp)
	{
		WindUp();
		g_debugWindowWindUp = false;
	}
	if (g_debugWindowWindDown)
	{
		WindDown();
		g_debugWindowWindDown = false;
	}
	if (g_debugWindowDebugPos)
	{
		m_Position = (float)g_debugWindowPos;
		updateMatrix = true;
	}
#endif

	if (updateMatrix || m_Overriden)
	{
		Maths::C_Quaternion sourceRot(m_RestMatrix.Rotation());
		Maths::C_Quaternion targetRot(m_RestMatrix.AZ(), m_FinalRotation);
		Maths::C_Quaternion quat = sourceRot.Slerp(targetRot, m_Position);
		Maths::HVec3 pos = Maths::Lerp(m_RestMatrix.AW(), m_FinalPosition, m_Position);
		m_pPart->m_RestMatrix = Maths::HMat43(Maths::HMat33(quat), pos);
		m_Overriden = false;
	}
}

} //namespace Vehicle
