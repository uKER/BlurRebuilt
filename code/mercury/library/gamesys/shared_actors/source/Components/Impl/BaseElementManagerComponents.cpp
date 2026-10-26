
#include "SharedActorsPch.h"
#include <shared_actors/Components/Impl/BaseElementManagerComponents.h>
#include <game_resource/ModelData.h>
#include <game_resource/ModelResource.h>
#include <gravity/importer/Importer.h>
#include <gravity/sdk/CompositeDescriptor.h>
#include <resource_utils/GameModel.h>
#include <shared_actors/Components/Impl/PhysicsBodyComponent.h>
#include <system/misc/Random.h>
#include <shared_actors/Components/Creation/BaseElementManagerComponentsCreate.h>
#include <misc_shared/Context/ContextualStream.h>
#include <actor_system/Actor.h>
#include <shared_actors/Components/Interfaces/I_ModelResource.h>
#include "shared_actors/Components/Messages/ElementManagerMessages.h"
#include <actor_system/ActorDebug.h>
#include <actor_system/ActorSerialisation.h>
#include <shared_actors/ActorCreation/ConstraintActorCreate.h>
#include <shared_actors/SharedActorUtils.h>
#include <shared_actors/ActorMessages/ConstraintActorMessages.h>
#include <system/misc/Random.h>
#include <Lod/LodManager.h>

#ifdef SHARED_ACTORS_OPTIMIZE_OFF
#pragma optimize("", off)
#endif

namespace Actors { extern Debug::Logger &g_Log; }

Debug::Float gElemMgrPivotDampedGravityRate("SharedActors.Components.DamageableElementMgr.PivotDampedGravityRate", 0.8f, Debug::Range(0.0f, 2.0f));
Debug::Float gElemMgrDetachImpulseScale("SharedActors.Components.DamageableElementMgr.DetachImpulseScale", 0.5f);
Debug::Bool g_ElemMgrNoConstraints("SharedActors.Components.DamageableElementMgr.NoConstraints", false);

using namespace Maths;


namespace Components
{

void C_BaseDamageableElementManager::C_ElementState::Serialise(Sys::C_ContextualStream& stream) const
{
	stream.WriteAs<bool>(m_IsActive);
	stream << m_DamageLevel;
	stream << m_Flags;
	stream << m_Strength;
	stream << m_Matrix;
	stream << m_PivotRotationAngles;
	stream << m_ParentElementIndex;
	//stream << m_ModelElementName;
	stream << m_ModelElementIndex;
	stream << m_UserHandle;
	SharedActorUtils::SerialiseVector<Actor::C_ActorHandle>(m_Constraints, stream);
}


void C_BaseDamageableElementManager::C_ElementState::Deserialise(Sys::C_ContextualStream& stream)
{
	stream.ReadAs<bool>(m_IsActive);
	stream >> m_DamageLevel;
	stream >> m_Flags;
	stream >> m_Strength;
	stream >> m_Matrix;
	stream >> m_PivotRotationAngles;
	stream >> m_ParentElementIndex;
	//stream >> m_ModelElementName;
	stream >> m_ModelElementIndex;
	stream >> m_UserHandle;
	SharedActorUtils::DeserialiseVector<Actor::C_ActorHandle>(m_Constraints, stream);
}


void C_BaseDamageableElementManager::C_ElementState::Init(S32 damageLevel, const BasePhysicalObject::C_ElementDamageLevelDef& dmgLevelDef)
{
	m_DamageLevel = damageLevel;
	m_Strength = dmgLevelDef.m_Strength;
	m_Flags = dmgLevelDef.m_BaseFlags;
	m_IsActive = true;
	//m_ModelElementName = dmgLevelDef.m_ModelElementName;
	m_ModelElementIndex = dmgLevelDef.ModelElementIndex();
}


static S32 RecursiveElementCount(const BasePhysicalObject::C_ElementObject& def)
{
	const XtArray<BasePhysicalObject::C_ElementDef*>& elementList = def.m_ElementDefHierarchy;
	S32 count = elementList.count;
	for (S32 e=0 ; e<elementList.count ; e++)
	{
		for (S32 c=0 ; c<elementList[e]->m_Children.count ; c++)
			count += RecursiveElementCount(*elementList[e]->m_Children[c]);
	}
	return count;
}



//////////////////////////////////////////////////////////////////////////
//
//
C_BaseDamageableElementManager::C_BaseDamageableElementManager()
{
	m_ActiveElementCount = 0;
	m_ContainsPivot = false;
	m_TotalInflictedDamage = 0.0f;
	m_SendInflictedDamageNotifications = false;
	m_DetachChildren = true;
}


C_BaseDamageableElementManager::~C_BaseDamageableElementManager()
{
}


bool C_BaseDamageableElementManager::InitialiseImpl()
{
	if (!I_DamageableElementManager::InitialiseImpl())
		return false;

	// Anything to do?

	return true;
}


// Creates the flattened PhysObj element list corresponding to each C_ElementDef in the sorcery hierarchy.
static void SetupFlatElements(const BasePhysicalObject::C_ElementObject& rootElemObjDef, BasePhysicalObject::TElementDefList& flatElementList)
{
	for (S32 e=0 ; e<rootElemObjDef.m_ElementDefHierarchy.count ; e++)
	{
		const BasePhysicalObject::C_ElementDef& elementDef = *rootElemObjDef.m_ElementDefHierarchy[e];
		flatElementList.PushBack(&elementDef);
		for (S32 c=0 ; c<elementDef.m_Children.count ; c++)
		{
			const BasePhysicalObject::C_ElementObject* pElementObject = elementDef.m_Children[c];
			if (pElementObject != NULL)
				SetupFlatElements(*pElementObject, flatElementList);
			else
				LOGWARN(Actors::g_Log, DBSTR"element definition '%s' missing child element %d", elementDef.m_Name.Text(), c);
		}
	}	
}


bool C_BaseDamageableElementManager::CreateImpl()
{
	if (!I_DamageableElementManager::CreateImpl())
		return false;

	const C_BaseDamageableElementManagerCreateInfo* pCreate = GetCreationData();

	XtHandle<ResourceUtils::C_GameModel> pModelDef = pCreate->GetModelDef();
	if (pModelDef)
	{
		const I_ModelResource* pModelResource = (const I_ModelResource*)GetActor()->QueryInterface(Components::E_IF_ModelResource);
		if (pModelResource && pModelResource->GetModelData())
			pCreate->GetModelCacheSpec()->SetupModelData(pCreate->m_ElementObject, pModelResource->GetModelData());
	}

	const BasePhysicalObject::C_ElementObject* rootElemObjDef = NULL;
	if (pCreate->m_RootModelElementIndex == -1)
	{
		rootElemObjDef = pCreate->m_ElementObject->m_RootObject;
	}
	else
	{
		S32 temp;
		rootElemObjDef = pCreate->m_ElementObject->m_RootObject->FindChildDef(pCreate->m_RootModelElementIndex, temp);
	}
	if (!rootElemObjDef)
		return false;

	if (m_ElementStates.Empty()) // i.e. Not deserialised
	{
		S32 errorCount = 0;
		InitElementHierarchy(*rootElemObjDef, pCreate->m_DamagePercent, -1, errorCount);
		if (errorCount > 0)
			return false;
	}

	// Set up flat element list:
	BIZ_ASSERT(m_FlatElements.Empty());
	SetupFlatElements(*rootElemObjDef, m_FlatElements);

	if (!m_FlatElements.Empty() && !pCreate->m_ElementStateSetup.Empty())
	{
		BIZ_ASSERT(m_ElementStates.Size() > 0 ? m_ElementStates.Size()==pCreate->m_ElementStateSetup.Size() : true);
		// Copy element states across but keep parent index :-/
		m_ActiveElementCount = 0;
		for (S32 e=0 ; e<m_ElementStates.Size() ; e++)
		{
			C_ElementState es = pCreate->m_ElementStateSetup[e];
			es.m_ParentElementIndex = m_ElementStates[e].m_ParentElementIndex;
			m_ElementStates[e] = es;
			if (es.m_IsActive)
				m_ActiveElementCount++;
		}
	}

	BIZ_ASSERT(m_ActiveElementCount>0 && m_ActiveElementCount<=m_FlatElements.Size() && m_FlatElements.Size()>0 && m_FlatElements.Size()==m_ElementStates.Size());

	return true;
}


void C_BaseDamageableElementManager::InternalDependenciesCreatedImpl()
{
	I_DamageableElementManager::InternalDependenciesCreatedImpl();

	// Detach child objects with constraints specified:
	// Note: detach children here when actor fully constructed, as they query info from parent.
	if (m_DetachChildren && !g_ElemMgrNoConstraints)
	{
		S32 elemIndex=1;	// Skip root.
		while (elemIndex < m_FlatElements.Size())
		{
			const BasePhysicalObject::C_ElementObject* objDef = m_FlatElements[elemIndex]->m_ElementObject;
			if (objDef->m_Constraint)
			{
				DetachChild(elemIndex, NULL, true);
				elemIndex += RecursiveElementCount(*objDef);	// Skip object branch.
			}
			else
			{
				elemIndex++;
			}
		}

		SendMessageToActor(E_ElementMgrNotification_ElementStateChanged);
	}
}


const GameResource::C_Model* C_BaseDamageableElementManager::ModelResource() const
{
	const I_ModelResource* mrc = GetActor()->QueryInterface<I_ModelResource>();
	if (mrc && mrc->GetModelData())
		return mrc->GetModelData()->GetRootModel();

	return NULL;
}


const C_BaseDamageableElementManagerCreateInfo* C_BaseDamageableElementManager::GetCreationData(void) const
{
	return static_cast<const C_BaseDamageableElementManagerCreateInfo*>(GetCreator().GetData());
}


// Gets element render info.
C_ElementState C_BaseDamageableElementManager::GetElement(S32 index) const
{
	BIZ_ASSERT(index < ActiveElementCount());

	S32 activeElementCount = 0;
	for (S32 e=0 ; e<ElementCount() ; e++)
	{
		if (ElementIsActive(e))
		{
			if (index == activeElementCount)
			{
				Components::C_ElementState es;
				es.m_Matrix = ElementState(e).m_Matrix;
				es.m_ModelElement = ElementDamageLevelDef(e).ModelElementIndex();
				return es;
			}
			activeElementCount++;
		}
	}

	BIZ_ASSERT_FAIL(DBSTR"active element count screwed!");
	return Components::C_ElementState();
}


bool C_BaseDamageableElementManager::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	switch (pMessage->Type())
	{
		case E_ElementMgrMsg_EnableElement:
		{
			const C_ElementMgrEnableElementMsg* msg = Messaging::MessageCast<C_ElementMgrEnableElementMsg>(pMessage);
			EnableElement(msg->m_ElementIndex, msg->m_Enable);
			break;
		}

// 		case E_ElementMgrMsg_ReduceElementStrength:
// 		{
// 			const C_ElementMgrReduceElementStrengthMsg* msg = Messaging::MessageCast<C_ElementMgrReduceElementStrengthMsg>(pMessage);
// 			ReduceElementStrength(msg->m_ElementIndex, msg->m_Amount);
// 			break;
// 		}

		case E_ElementMgrMsg_ChangeElementDamageLevel:
		{
			const C_ElementMgrChangeElementDamageLevelMsg* msg = Messaging::MessageCast<C_ElementMgrChangeElementDamageLevelMsg>(pMessage);
			const S32 elemIndex = ElementIndex(msg->m_ElementName);
			if (elemIndex >= 0)
			{
				if (ElementIsActive(elemIndex))
				{
					if (msg->m_DamageLevel >= 0 && msg->m_DamageLevel < ElementDamageLevelCount(elemIndex))
					{
						ChangeElementDamageLevel(elemIndex, msg->m_DamageLevel);
						SendMessageToActor(E_ElementMgrNotification_ElementStateChanged);
					}
					else
					{
						LOGERROR(Actors::g_Log, "%s: change element dmg level: invalid damage level '%d' (valid range:0 to %d)", GetActor()->GetActorName(), msg->m_DamageLevel, ElementDamageLevelCount(elemIndex)-1);
					}
				}
			}
			else
			{
				LOGERROR(Actors::g_Log, "%s: change element dmg level: invalid element '%s'", GetActor()->GetActorName(), msg->m_ElementName.Chars());
			}
			break;
		}

		case E_ElementMgrMsg_DamageElement:
		{
			const C_ElementMgrDamageElementMsg* msg = Messaging::MessageCast<C_ElementMgrDamageElementMsg>(pMessage);
			const ActorDamage::C_DamageData* dmgData = msg->m_DamageDataSet ? &msg->DamageData() : NULL;
			if (msg->m_ElementIndex >= 0 && ElementState(msg->m_ElementIndex).m_IsActive)
			{
				DamageElement(msg->m_ElementIndex, dmgData, msg->m_DamageInfo);
			}
			break;
		}

		case E_ElementMgrMsg_BreakElement:
		{
			const C_ElementMgrBreakElementMsg* msg = Messaging::MessageCast<C_ElementMgrBreakElementMsg>(pMessage);
			const ActorDamage::C_DamageData* dmgData = msg->m_DamageDataSet ? &msg->DamageData() : NULL;
			if (msg->m_ElementIndex >= 0 && ElementState(msg->m_ElementIndex).m_IsActive)
			{
// [MGB] Don't think we want this here:
//				if (!FlatElementDefs()[msg->m_ElementIndex]->m_DontIncInflictedDamageVal)
//					IncInflictedDamageVal(ElementState(msg->m_ElementIndex).m_Strength);
				BreakElement(msg->m_ElementIndex, dmgData, msg->m_DamageInfo);
			}
			break;
		}

		case E_ElementMgrMsg_BreakNamedElement:
		{
			const C_ElementMgrBreakNamedElementMsg* msg = Messaging::MessageCast<C_ElementMgrBreakNamedElementMsg>(pMessage);
			const S32 elemIndex = ElementIndex(msg->m_ElementName);
			if (elemIndex >= 0)
			{
				if (ElementState(elemIndex).m_IsActive)
				{
// [MGB] Don't think we want this here:
//					if (!FlatElementDefs()[elemIndex]->m_DontIncInflictedDamageVal)
//						IncInflictedDamageVal(ElementState(index).m_Strength);
					BreakElement(elemIndex, NULL, ActorDamage::C_DamageInfo(), msg->m_RespectIndestructible);
				}
			}
			else
			{
				LOGERROR(Actors::g_Log, "%s: break named element: invalid element '%s'", GetActor()->GetActorName(), msg->m_ElementName.Chars());
			}
		}	break;

		case E_ElementMgrMsg_DestroyElement:
		{
			const C_ElementMgrDestroyElementMsg* msg = Messaging::MessageCast<C_ElementMgrDestroyElementMsg>(pMessage);
			const ActorDamage::C_DamageData* dmgData = msg->m_DamageDataSet ? &msg->DamageData() : NULL;
			if (msg->m_ElementIndex >= 0 && ElementState(msg->m_ElementIndex).m_IsActive)
			{
				DestroyElement(msg->m_ElementIndex, dmgData, msg->m_DamageInfo);
			}
			break;
		}

		case E_ElementMgrMsg_DestroyNamedElement:
		{
			const C_ElementMgrDestroyNamedElementMsg* msg = Messaging::MessageCast<C_ElementMgrDestroyNamedElementMsg>(pMessage);
			const S32 elemIndex = ElementIndex(msg->m_ElementName);
			if (elemIndex >= 0)
			{
				if (ElementIsActive(elemIndex) && !(IsElementIndestructible(elemIndex) && msg->m_RespectIndestructible))
					DestroyElement(elemIndex, NULL, ActorDamage::C_DamageInfo());
			}
			else
			{
				LOGERROR(Actors::g_Log, "%s: destroy named element: invalid element '%s'", GetActor()->GetActorName(), msg->m_ElementName.Chars());
			}
		}	break;

		case E_ElementMgrMsg_SetElementIndestructible:
		{
			const C_ElementMgrSetElementIndestructibleMsg* msg = Messaging::MessageCast<C_ElementMgrSetElementIndestructibleMsg>(pMessage);
			const S32 elemIndex = ElementIndex(msg->m_ElementName);
			if (elemIndex >= 0)
			{
				if (ElementIsActive(elemIndex))
					SetElementIndestructible(elemIndex, msg->m_Set);
			}
			else
			{
				LOGERROR(Actors::g_Log, "%s: set element indestructible: invalid element '%s'", GetActor()->GetActorName(), msg->m_ElementName.Chars());
			}
			break;
		}

		case E_ElementMgrMsg_SetAllElementsIndestructible:
		{
			const C_ElementMgrSetAllElementsIndestructibleMsg* msg = Messaging::MessageCast<C_ElementMgrSetAllElementsIndestructibleMsg>(pMessage);
			for (S32 i=0 ; i<ElementCount() ; i++)
			{
				if (ElementIsActive(i))
					SetElementIndestructible(i, msg->GetValue());
			}
		}	break;

		case E_ElementMgrMsg_UpdateElementMatricies:
			UpdateElementMatrices();
			break;

		case E_ElementMgrMsg_UpdatePivots:
		{
			const C_ElementMgrUpdatePivotsMsg* msg = Messaging::MessageCast<C_ElementMgrUpdatePivotsMsg>(pMessage);
			UpdatePivots(msg->m_PivotInfo);
			break;
		}

		case E_ElementMgrMsg_UpdatePivot:
		{
			const C_ElementMgrUpdatePivotMsg* msg = Messaging::MessageCast<C_ElementMgrUpdatePivotMsg>(pMessage);
			UpdatePivot(msg->m_ElementIndex, msg->m_Rotation);
			break;
		}

		case E_ElementMgrMsg_UpdateScriptedPivot:
		{
			const C_ElementMgrUpdateScriptedPivotMsg* msg = Messaging::MessageCast<C_ElementMgrUpdateScriptedPivotMsg>(pMessage);
			const S32 index = ElementIndex(msg->m_ElementName);
			if (index >= 0 && ElementState(index).m_IsActive)
				UpdateScriptedPivot(index, msg->m_Rotation, msg->m_Time);
			break;
		}

		case E_ElementMgrMsg_UpdatePhysicsShapes:
			UpdatePhysicsShapes();
			break;

		case E_ElementMgrMsg_IncInflictedDamage:
		{
			const C_ElementMgrIncInflictedDamageMsg* msg = Messaging::MessageCast<C_ElementMgrIncInflictedDamageMsg>(pMessage);
			IncInflictedDamageVal(msg->GetValue());
		}	break;

		case E_ElementMgrMsg_AddDependentConstraint:
		{
			const C_ElementMgrAddDependentConstraintMsg* msg = Messaging::MessageCast<C_ElementMgrAddDependentConstraintMsg>(pMessage);
			BIZ_ASSERT_MSG(msg->m_ElementIndex>=0 && ElementIsActive(msg->m_ElementIndex), "Invalid element index (%d)", msg->m_ElementIndex);
			if (msg->m_ElementIndex >= 0)
				ElementState(msg->m_ElementIndex).m_Constraints.PushBack(msg->m_ConstraintHandle);
		}	break;

		case E_ElementMgrMsg_EnableInflictedDamageNotifications:
			m_SendInflictedDamageNotifications = true;
			break;

		case E_ElementMgrMsg_DetachElement:
			{
				const C_ElementMgrDetachElementMsg* msg = Messaging::MessageCast<C_ElementMgrDetachElementMsg>(pMessage);
				const S32 elemIndex = ElementIndex(msg->m_ElementName);
				if (elemIndex >= 0 && ElementState(elemIndex).m_IsActive)
				{
					ActorDamage::C_DamageData dmgData;
					dmgData.SetHitImpulse(msg->m_Impulse * GetActor()->GetMatrix().Rotation());
					DetachChild(elemIndex, &dmgData);
					SendMessageToActor(E_ElementMgrNotification_ElementStateChanged);
				}
			}			
			break;

		default:
			return I_DamageableElementManager::OnMessage(pMessage);
	}

	return true;
}


void C_BaseDamageableElementManager::IncInflictedDamageVal(F32 aVal)
{
	const F32 oldVal = m_TotalInflictedDamage;
	m_TotalInflictedDamage += aVal;

	if (m_SendInflictedDamageNotifications)
	{
		const C_ElementMgrNotificationInflictedDamageValIncreased msg(oldVal, m_TotalInflictedDamage);
		SendMessageToActor(&msg);
	}
}


#ifndef CFG_GOLD
static Debug::Bool g_DamageableElementMgrDebugRender("ActorDebug.Components.DamageableElementMgr.DebugRender.Enable", false);
static Debug::Bool g_DamageableElementMgrDrawElementMat("ActorDebug.Components.DamageableElementMgr.DebugRender.ElementMatrix", false);
static Debug::Bool g_DamageableElementMgrDrawElementInfo("ActorDebug.Components.DamageableElementMgr.DebugRender.ElementInfo", false);

void C_BaseDamageableElementManager::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
{
	if (!g_DamageableElementMgrDebugRender)
		return;

	// Render model:
	Maths::HMat43 renderMat(GetActor()->GetMatrix());

	for (int e=0 ; e<ActiveElementCount() ; e++)
	{
		const Gravity::C_ImportShape* importShape = ActiveCollisionShape(e);
		if (importShape)
		{
			Gravity::C_ShapeDesc* desc = importShape->GetShapeDesc();
			const Maths::HVec3 worldPos = (ElementState(e).m_Matrix.AW() + desc->GetBoundingBox().centre) * renderMat;//(GetElementDefs()[e]->m_Pivoted ? ElementState(e).m_Matrix.AW() : shape->m_RenderOffset) * renderMat;

			if (g_DamageableElementMgrDrawElementMat)
			{
				Maths::HMat43 elementMat = importShape->GetImportMatrix();
				elementMat = elementMat * renderMat;
				//elementMat.AW(worldPos);
				debugRenderPtr->Draw3dMatrix(elementMat, 0.2f);
			}

			if (g_DamageableElementMgrDrawElementInfo)
			{
				Util::C_String text;
				text.Printf("%d: %s\nDL:%d\nSt:%.02f%s", e, FlatElementDefs()[e]->m_Name.string, ElementState(e).m_DamageLevel, ElementState(e).m_Strength, (ElementState(e).m_Flags&BasePhysicalObject::FlagElementDamageLevelIndestructible)!=0?" (indestr)":"");
				debugRenderPtr->Draw3dText(worldPos, 0.1f, text);
			}
		}
	}

	debugRenderPtr->Draw3dText(renderMat.AW()+Maths::HVec3(0,-1,0), Util::C_String::Format("TotDmg: %.02f", TotalInflictedDamage()));
}
#endif


// Creates the element state list, using rootElemObj as the root object.
void C_BaseDamageableElementManager::InitElementHierarchy(const BasePhysicalObject::C_ElementObject& rootElemObj, F32 damagePercent, S32 parentElementIndex, S32& errorCount)
{
	const GameResource::C_Model* modelRes = ModelResource();

	for (S32 e=0 ; e<rootElemObj.m_ElementDefHierarchy.count ; e++)
	{
		const BasePhysicalObject::C_ElementDef& elementDef = *rootElemObj.m_ElementDefHierarchy[e];

		elementDef.m_ElementObject = &rootElemObj;

		if (elementDef.m_Pivoted)
			m_ContainsPivot = true;

		// Store element state:
		C_ElementState elemState;
		elemState.m_PivotRotationAngles = Maths::HVec3(0.0f,0.0f,0.0f);

		// TODO: reinstate initial damage if required.
//		if (damagePercent == 0.0f)
			elemState.Init(0, *elementDef.m_DamageLevels[0]);
//		else
//			ApplyInitialDamage(elemState, elementDef, damagePercent);

		if (elemState.m_Strength <= 0.0f)
		{
			LOGERROR(Actors::g_Log, DBSTR"%s: element %d, has invalid strength", GetActor()->GetActorName(), e);
			elemState.m_Strength = 1.0f;
		}

		elemState.m_Matrix = modelRes->GetElement(elementDef.m_DamageLevels[0]->ModelElementIndex())->GetMatrix();

		elemState.m_ParentElementIndex = parentElementIndex;
		m_ElementStates.PushBack(elemState);
		m_ActiveElementCount++;

		// Process any children: (depth-first)
		const S32 parentElemIndex = m_ElementStates.Size()-1;
		for (S32 c=0 ; c<elementDef.m_Children.count ; c++)
		{
			const BasePhysicalObject::C_ElementObject* pElementObject = elementDef.m_Children[c];
			if (pElementObject)
			{
				if (pElementObject != &rootElemObj)
				{
					InitElementHierarchy(*pElementObject, damagePercent, parentElemIndex, errorCount);
				}
				else
				{
					errorCount++;
					LOGERROR(Actors::g_Log, DBSTR"%s: cyclic link found at element '%s', child %d", GetActor()->GetActorName(), elementDef.m_Name.Text(), c);
				}
			}
			else
			{
				LOGWARN(Actors::g_Log, DBSTR "%s: element definition '%s' missing child element %d", GetActor()->GetActorName(), elementDef.m_Name.Text(), c);
			}
		}

#ifndef CFG_GOLD
		// Check hierarchy validity:
		for (S32 d=0 ; d<elementDef.m_DamageLevels.count ; d++)
		{
			const S32 srCount = elementDef.m_DamageLevels[d]->m_ShatterReplacements.count;
			for (S32 sr=0 ; sr<srCount ; sr++)
			{
				if (elementDef.m_DamageLevels[d]->m_ShatterReplacements[sr] == &rootElemObj)
				{
					LOGERROR(Actors::g_Log, DBSTR"%s: cyclic link found at element '%s', damage level %d, replacement %d", GetActor()->GetActorName(), elementDef.m_Name.Text(), d, sr);
					errorCount++;
				}
			}
		}
#endif
	}
}


void C_BaseDamageableElementManager::UpdatePhysicsShapes()
{
	const BasePhysicalObject::TElementDefList& elementDefs = FlatElementDefs();
	U32 runningPrimCount=0;
	const S32 elementCount = m_ElementStates.Size();
	for (S32 e=0 ; e<elementCount ; e++)
	{
		if (ElementIsActive(e))
		{
			if (elementDefs[e]->m_Pivoted)
			{
// todoGrav:
// 				const U32 srcPrimitiveCount = CollisionShape(e)->Primitives();
// 
// 				for ( U32 srcPrimitiveIndex=0; srcPrimitiveIndex < srcPrimitiveCount; srcPrimitiveIndex++ )
// 				{
// 					Maths::HMat43 trans(CollisionShape(e)->Transform(srcPrimitiveIndex) * ElementState(e).m_Matrix);
// //?					m_pCurrentShape->Transform(runningPrimCount, trans );
//
//					if (ElementDamageLevelDef(e).m_pShape)
//						runningPrimCount++;
// 				}
			}
			else
			{
				if (ElementDamageLevelDef(e).m_pShape)
					runningPrimCount++;
			}
		}
	}
}


void C_BaseDamageableElementManager::UpdateElementMatrices()
{
	const BasePhysicalObject::TElementDefList& elementDefs = FlatElementDefs();
	const S32 elementCount = m_ElementStates.Size();
	for (S32 e=0 ; e<elementCount ; e++)
	{
		if (ElementIsActive(e))
		{
			C_ElementState& elemState = ElementState(e);
			if (elementDefs[e]->m_Pivoted && elemState.m_ParentElementIndex >= 0)
			{
				Maths::HMat43 rotation(Maths::C_MatrixRotatePHR(elemState.m_PivotRotationAngles));
				elemState.m_Matrix = rotation * elementDefs[e]->m_PivotMatrix;
				if (elementDefs[e]->m_PivotParented)
				{
					elemState.m_Matrix = elemState.m_Matrix * ElementState(elemState.m_ParentElementIndex).m_Matrix;
				}
			}
		}
	}
}


void C_BaseDamageableElementManager::UpdatePivots(const BasePhysicalObject::C_PivotInfo &pivotInfo)
{
	const BasePhysicalObject::TElementDefList& elementDefs = FlatElementDefs();
	F32 time = pivotInfo.m_TimeStep;

	Maths::HVec3 localAccel(pivotInfo.m_LocalAccel);
	Maths::HVec3 localVel(pivotInfo.m_LocalVel);
	F32 forwardVelSqr = localVel.Z() * localVel.Z();

	if (localAccel.Y() > -9.8f && localAccel.Y() < 2.0f)
		localAccel.Y(0.0f);

	const S32 elementCount = m_ElementStates.Size();
	for (S32 e=0 ; e<elementCount ; e++)
	{
		if (ElementIsActive(e) && elementDefs[e]->m_Pivoted && (ElementDamageLevelDef(e).m_BaseFlags & BasePhysicalObject::FlagElementDamageLevelEnablePivot))
		{
			Maths::HVec3 currRotation = ElementState(e).m_PivotRotationAngles;
			if (elementDefs[e]->m_PivotDef)
			{
				switch (elementDefs[e]->m_PivotDef->m_PivotType)
				{
					case BasePhysicalObject::E_PivotRotationType_XAxis:
					{					
						currRotation.X(currRotation.X()-localAccel.Y() * elementDefs[e]->m_PivotDef->m_AccelScale * time);
						currRotation.X(currRotation.X()-gElemMgrPivotDampedGravityRate * elementDefs[e]->m_PivotDef->m_GravScale * time);
					}
					break;
					
					case BasePhysicalObject::E_PivotRotationType_YAxis:
					{
						currRotation.Y( currRotation.Y() + localAccel.X() * elementDefs[e]->m_PivotDef->m_AccelScale * time);
						currRotation.Y( currRotation.Y() - forwardVelSqr * elementDefs[e]->m_PivotDef->m_VelScale * time);
					}
					break;
					
					case BasePhysicalObject::E_PivotRotationType_ZAxis:
					{
						currRotation.Z( currRotation.Z() + localAccel.X() * elementDefs[e]->m_PivotDef->m_AccelScale * time);
					}	
					break;

					case BasePhysicalObject::E_PivotRotationType_Scripted:
						{
							if (ElementState(e).m_Time > time)
							{
								Maths::HVec3 step = time * ElementState(e).m_ScriptedPivotRotation; 
								currRotation = WrapAddRotation(currRotation, step);
								ElementState(e).m_Time -= time;
							}
							else if (ElementState(e).m_Time > 0.01f)
							{
								Maths::HVec3 step = ElementState(e).m_Time * ElementState(e).m_ScriptedPivotRotation; 
								currRotation = WrapAddRotation(currRotation, step);
								ElementState(e).m_Time = 0.0f;
							}

						}
						break;

					case BasePhysicalObject::E_PivotRotationType_AutoRotation:
						{
							const F32 rps = Maths::Pi2/60.0f;
							F32 rotation = rps * elementDefs[e]->m_PivotDef->m_AutoRotationRPM * time;
							Maths::HVec3 step(0.0f, rotation, 0.0f);
							currRotation = WrapAddRotation(currRotation, step);
						}
					break;
					default:
					break;
				}

				const Maths::HVec3 minRotation(Maths::DegToRad(Load(elementDefs[e]->m_PivotDef->m_MinRotation)));
				const Maths::HVec3 maxRotation(Maths::DegToRad(Load(elementDefs[e]->m_PivotDef->m_MaxRotation)));
				currRotation = Maths::Min(currRotation, maxRotation);
				currRotation = Maths::Max(currRotation, minRotation);
				ElementState(e).m_PivotRotationAngles = currRotation;
			}
		}
	}
}


void C_BaseDamageableElementManager::UpdatePivot(const S32 elementIndex, const Maths::HVec3&rotation)
{
	BIZ_ASSERT(elementIndex >=0);
	const BasePhysicalObject::TElementDefList& elementDefs = FlatElementDefs();

	if (!elementDefs[elementIndex]->m_PivotDef)
		return;	// May be pivotParented with no pivot itself.

	if (ElementIsActive(elementIndex) && elementDefs[elementIndex]->m_Pivoted && (ElementDamageLevelDef(elementIndex).m_BaseFlags & BasePhysicalObject::FlagElementDamageLevelEnablePivot))
	{
		if (elementDefs[elementIndex]->m_PivotDef->m_PivotType == BasePhysicalObject::E_PivotRotationType_ExternalSet)
			ElementState(elementIndex).m_PivotRotationAngles = rotation;	
	}
}


Maths::HVec3 C_BaseDamageableElementManager::WrapAddRotation(const Maths::HVec3& base, const Maths::HVec3 &add )
{
	Maths::HVec3 result = base + add;
	F32 x(result.X()),y(result.Y()),z(result.Z());

	if (x > Maths::Pi2)		x-= Maths::Pi2;
	if (x < -Maths::Pi2)	x+= Maths::Pi2;
	if (y > Maths::Pi2)		y-= Maths::Pi2;
	if (y < -Maths::Pi2)	y+= Maths::Pi2;
	if (z > Maths::Pi2)		z-= Maths::Pi2;
	if (z < -Maths::Pi2)	z+= Maths::Pi2;

	return Maths::HVec3(x,y,z);
}


void C_BaseDamageableElementManager::UpdateScriptedPivot(const S32 elementIndex, const Maths::HVec3&rotation, F32 time)
{
	const BasePhysicalObject::TElementDefList& elementDefs = FlatElementDefs();
	BIZ_ASSERT(elementIndex >=0);
	if (ElementIsActive(elementIndex) && elementDefs[elementIndex]->m_Pivoted && (ElementDamageLevelDef(elementIndex).m_BaseFlags & BasePhysicalObject::FlagElementDamageLevelEnablePivot))
	{
		if (elementDefs[elementIndex]->m_PivotDef->m_PivotType == BasePhysicalObject::E_PivotRotationType_Scripted)
		{
			Maths::HVec3 rotrad(Maths::DegToRad(rotation.X()), Maths::DegToRad(rotation.Y()), Maths::DegToRad(rotation.Z()));
			if (time > 0.01f)
			{

				ElementState(elementIndex).m_ScriptedPivotRotation = rotrad / time;	
				ElementState(elementIndex).m_Time = time;
			}
			else
			{
				ElementState(elementIndex).m_PivotRotationAngles = WrapAddRotation(ElementState(elementIndex).m_PivotRotationAngles, rotrad);
			}

		}
	}
}


void C_BaseDamageableElementManager::EnableElement(S32 elementIndex, bool enable)
{
	BIZ_ASSERT(elementIndex >= 0);

	if(enable == false && m_ActiveElementCount <= 1)	// Don't disable last element.
		return;

	if (ElementState(elementIndex).m_Strength <= 0.0f || enable == ElementState(elementIndex).m_IsActive || m_ActiveElementCount < 1)
		return;

	if (CollisionShape(elementIndex))
		SendMessageToActor(E_ElementMgrNotification_ElementStateChanged);

	if(ElementState(elementIndex).m_IsActive != enable)
	{
		m_ActiveElementCount += (enable)? 1 : -1;
		ElementState(elementIndex).m_IsActive = enable;
	}

	BIZ_ASSERT(m_ActiveElementCount >= 0);
}


const char* C_BaseDamageableElementManager::ElementName(S32 elementIndex) const
{
	return FlatElementDefs()[elementIndex]->m_Name.Text();
}


S32 C_BaseDamageableElementManager::ElementDamageLevelCount(S32 elementIndex) const 
{
	return FlatElementDefs()[elementIndex]->m_DamageLevels.count;
}


S32 C_BaseDamageableElementManager::ElementIndex(const Util::C_String& elementName) const
{
	for (S32 e=0 ; e<FlatElementDefs().Size() ; e++)
	{
		if (elementName == Util::C_String(FlatElementDefs()[e]->m_Name))
			return e;
	}

	return -1;
}


// Maps a shape descriptor primitive index to a dynObj element index.
// Returns the dynObj element index, or -1 if not mapped.
S32 C_BaseDamageableElementManager::ElementIndexFromFlatPrimitiveIndex(S32 primIndex) const
{
	if (primIndex < 0)	// Can happen now we got deferred shape swap
		return -1;

	int runningPrimCount = 0;
	
	for (S32 e=0 ; e<m_ElementStates.Size() ; e++)
	{
		if (ElementIsActive(e))
		{
			if (ElementDamageLevelDef(e).m_pShape)
				runningPrimCount++;

			if (runningPrimCount > primIndex)
				return e;
		}
	}

	return -1;
}


S32 C_BaseDamageableElementManager::PrimitiveIndexFromElementIndex(S32 elementIndex) const
{
	BIZ_ASSERT(elementIndex >= 0);
	int runningElementCount = 0;

	for (S32 e=0 ; e<elementIndex ; e++)
	{
		if (ElementIsActive(e))
		{
			if (ElementDamageLevelDef(e).m_pShape)
				runningElementCount++;
		}
	}

	return runningElementCount;
}


const BasePhysicalObject::C_ElementDamageLevelDef& C_BaseDamageableElementManager::ElementDamageLevelDef(S32 dynObjElementIndex, S32 dmgLevel) const 
{ 
	BIZ_ASSERT(dynObjElementIndex>=0); 
	return *FlatElementDefs()[dynObjElementIndex]->m_DamageLevels[dmgLevel]; 
}


const Gravity::C_ImportShape* C_BaseDamageableElementManager::CollisionShape(S32 dynObjElementIndex) const 
{
	const GameResource::C_ModelElement* modelElement = ModelElement(dynObjElementIndex);
	if (modelElement)
		return modelElement->GetPhysicsImportShape();
	return NULL;
}


// Returns the element index of the 'n'th active element.
S32 C_BaseDamageableElementManager::ElementIndexFromActiveElementIndex(S32 n) const
{
	S32 activeElementCount = 0;
	const S32 elementCount = m_ElementStates.Size();
	for (S32 e=0 ; e<elementCount ; e++)
	{
		if (ElementIsActive(e))
		{
			if (n==activeElementCount)
				return e;
			activeElementCount++;
		}
	}

	return -1;
}


const Gravity::C_ImportShape* C_BaseDamageableElementManager::ActiveCollisionShape(S32 activeElementIndex) const
{
	S32 activeElementCount = 0;
	const S32 elementCount = m_ElementStates.Size();
	for (S32 e=0 ; e<elementCount ; e++)
	{
		if (ElementIsActive(e))
		{
			if (activeElementCount==activeElementIndex)
				return CollisionShape(e);
			activeElementCount++;
		}
	}

	LOGWARN(Actors::g_Log, DBSTR"fail for elem index %d!", activeElementIndex);
	return NULL;
}


void C_BaseDamageableElementManager::SetElementIndestructible(S32 elementIndex, bool set)
{
	BIZ_ASSERT(elementIndex>=0);
	BIZ_ASSERT(ElementIsActive(elementIndex));

	if (set)
		ElementState(elementIndex).m_Flags |= BasePhysicalObject::FlagElementDamageLevelIndestructible;
	else
		ElementState(elementIndex).m_Flags &= ~BasePhysicalObject::FlagElementDamageLevelIndestructible;
}


void C_BaseDamageableElementManager::ChangeElementDamageLevel(S32 elementIndex, S32 damageLevel)
{
	BIZ_ASSERT(elementIndex>=0);
	BIZ_ASSERT(ElementIsActive(elementIndex));

	// Store vals:  ugh.
	const Maths::HVec3 rotation = ElementState(elementIndex).m_PivotRotationAngles;
	const Maths::HMat43 mat = ElementState(elementIndex).m_Matrix;
	const S32 parentIndex = ElementState(elementIndex).m_ParentElementIndex;

	ElementState(elementIndex) = C_ElementState(damageLevel, ElementDamageLevelDef(elementIndex, damageLevel));

	// Restore vals:
	ElementState(elementIndex).m_PivotRotationAngles = rotation;
	ElementState(elementIndex).m_Matrix = mat;
	ElementState(elementIndex).m_ParentElementIndex = parentIndex;
}


void C_BaseDamageableElementManager::DamageElement(S32 elementIndex, const ActorDamage::C_DamageData *pDamageData, const ActorDamage::C_DamageInfo& damageInfo)
{
	BIZ_ASSERT(elementIndex>=0);
	BIZ_ASSERT(ElementIsActive(elementIndex));

	if (ActorPhysics::gLogForces)
		LOGINFO(Actors::g_Log, "%s: ElemMgr damage elem force: %s", GetActor()->GetActorName(), ActorPhysics::Stringify(pDamageData->HitImpulse().Get()).Chars());

	C_ElementState& elemState = ElementState(elementIndex);
	const S32 elementDmgLevel = elemState.m_DamageLevel;
	BIZ_ASSERT(elementDmgLevel >= 0);

	// Only apply damage above threshold:
	if (pDamageData->m_DamageAmount < ElementDamageLevelDef(elementIndex).m_DamageAmountThreshold)
		return;

	F32 inflictedDamage = 0.0f;

	if (!IsElementIndestructible(elementIndex))
	{
		const F32 strength = elemState.m_Strength;	// Store.

		elemState.m_Strength -= pDamageData->m_DamageAmount;

  		if (ElementState(elementIndex).m_Strength > 0.0f)
  		{
  			inflictedDamage = pDamageData->m_DamageAmount;
  		}
 		else
		{
			// Simplest way of calculating inflicted damage:
			inflictedDamage = strength;
			for (S32 dl=elemState.m_DamageLevel+1 ; dl<ElementDamageLevelCount(elementIndex) ; dl++)
			{
				if (ElementDamageLevelDef(elementIndex, dl).m_BaseFlags & BasePhysicalObject::FlagElementDamageLevelIndestructible)
					break;
				inflictedDamage += ElementDamageLevelDef(elementIndex, dl).m_Strength;
			}
			inflictedDamage = Maths::Min(inflictedDamage, pDamageData->m_DamageAmount);
 		}
	}

	while (elemState.m_Strength<=0.0f && !IsElementIndestructible(elementIndex) && elemState.m_IsActive)
	{
		const S32 dmgLevel = elemState.m_DamageLevel;
		const S32 dmgLevelCount = ElementDamageLevelCount(elementIndex);

		if (dmgLevel < dmgLevelCount-1)
		{
			BreakElement(elementIndex, pDamageData, damageInfo);
		}
		else if (pDamageData->m_ForceIndestructibleBase)
		{
			break;
		}
		else
		{
			// Element is destroyed.
			DestroyElement(elementIndex, pDamageData, damageInfo);
		}
	}

	if (inflictedDamage > 0.0f && !FlatElementDefs()[elementIndex]->m_DontIncInflictedDamageVal)
		IncInflictedDamageVal(inflictedDamage);
}


void C_BaseDamageableElementManager::BreakElement(S32 elementIndex, const ActorDamage::C_DamageData* pDamageData, const ActorDamage::C_DamageInfo& damageInfo, bool respectIndestructible)
{
	C_ElementState& elemState = ElementState(elementIndex);

	const bool isOnLastDamageLevel = elemState.m_DamageLevel == ElementDamageLevelCount(elementIndex)-1;
	if (isOnLastDamageLevel)
	{
		if ((!IsElementIndestructible(elementIndex) && !pDamageData->m_ForceIndestructibleBase) || !respectIndestructible)
		{
			DestroyElement(elementIndex, pDamageData, damageInfo);
		}
		return;
	}

	if (!ElementIsActive(elementIndex))	// Is this possible here?
		return;

	OnElementBreak(elementIndex, pDamageData, damageInfo);
	SpawnElementBreakoff(elementIndex, pDamageData, damageInfo);

	// Note: uses damage level *before* modification:
	const bool makeDynamic = (ElementDamageLevelDef(elementIndex).m_BaseFlags & BasePhysicalObject::FlagElementDamageLevelMakeDynamicOnBreak) != 0;

	const S32 newDamageLevel = elemState.m_DamageLevel+1;
	const F32 strength = elemState.m_Strength;
	ChangeElementDamageLevel(elementIndex, newDamageLevel);
	elemState.m_Strength = strength;

	if (elemState.m_Strength > 0.0f)
		elemState.m_Strength = 0.0f;

	const BasePhysicalObject::C_ElementDamageLevelDef& dmgLvlDef = ElementDamageLevelDef(elementIndex);

	if (IsElementIndestructible(elementIndex) && respectIndestructible)
	{
		elemState.m_Strength = dmgLvlDef.m_Strength;
	}
	else
	{
		elemState.m_Strength += dmgLvlDef.m_Strength;
	}

	const C_ElementMgrNotificationElementBrokenMsg elementBrokenMsg(elementIndex);
	SendMessageToActor(&elementBrokenMsg);

	if (makeDynamic)
		SendMessageToActor(E_ElementMgrNotification_MakeDynamic);

	// Check for element detach on break:
	const BasePhysicalObject::C_ElementDamageLevelDef& oldDmgLev = ElementDamageLevelDef(elementIndex, newDamageLevel-1);
	if (oldDmgLev.m_BaseFlags & BasePhysicalObject::FlagElementDamageLevelDetachOnBreak)
	{
		const BasePhysicalObject::C_Def* physObjDef = static_cast<const BasePhysicalObject::C_Def*>(FlatElementDefs()[elementIndex]->m_ElementObject);
		const bool inheritCollideGroup = (physObjDef->m_BaseFlags & BasePhysicalObject::FlagCollideWithParent)==0;
		DetachChild(elementIndex, pDamageData, inheritCollideGroup);
	}

	SendMessageToActor(E_ElementMgrNotification_ElementStateChanged);
}


// Returns the element object def that the given element index belongs to in the hierarchy.
// const BasePhysicalObject::C_ElementObject* C_BaseDamageableElementManager::ElementObjectFromElementIndex(S32 elementIndex)
// {
// 	const S32 parentElemIndex = ElementState(elementIndex).m_ParentElementIndex;
// 	const S32 elemModelElementIndex = ElementDamageLevelDef(elementIndex, 0).m_ModelElementIndex;
// 	if (parentElemIndex >= 0)
// 	{
// 		// Find object def for elementIndex:
// 		for (S32 c=0 ; c<FlatElementDefs()[parentElemIndex]->m_Children.count ; c++)
// 		{
// 			const BasePhysicalObject::C_ElementObject& childDef = *FlatElementDefs()[parentElemIndex]->m_Children[c];
// 			// Find element index of child:
// 			const S32 childModelElementIndex = childDef.m_ElementDefHierarchy[0]->m_DamageLevels[0]->m_ModelElementIndex;
// 			if (childModelElementIndex == elemModelElementIndex)
// 				return &childDef;
// 		}
// 	}
// 
// 	return NULL;
// }


void C_BaseDamageableElementManager::DestroyElement(S32 elementIndex, const ActorDamage::C_DamageData* pDamageData, const ActorDamage::C_DamageInfo& damageInfo)
{
	if (m_ActiveElementCount<=0)
	{
		LOGWARN(Actors::g_Log, DBSTR"No active elements");
		return;
	}

	if (ElementIsActive(elementIndex))
	{
		const BasePhysicalObject::TElementDefList& elementDefs = FlatElementDefs();

		OnElementDestroy(elementIndex, pDamageData, damageInfo);
		SpawnElementBreakoff(elementIndex, pDamageData, damageInfo);
		DetachElementChildren(elementIndex);

		// Kill element:
		ElementState(elementIndex).m_Strength = 0.0f;
		ElementState(elementIndex).m_DamageLevel = ElementDamageLevelCount(elementIndex)-1;
		ElementState(elementIndex).m_IsActive = false;

		// Kill any constraints attached to element:
		for (S32 c=ElementState(elementIndex).m_Constraints.Size()-1 ; c>=0 ; c--)
			SendMessageToActor(ElementState(elementIndex).m_Constraints[c], ActorMsg::E_ConstraintMsg_Destroy);
		ElementState(elementIndex).m_Constraints.Clear();

		m_ActiveElementCount--;
		BIZ_ASSERT(m_ActiveElementCount >= 0);

		// Send element broken msg here too.
		const C_ElementMgrNotificationElementBrokenMsg elementBrokenMsg(elementIndex);
		SendMessageToActor(&elementBrokenMsg);

		const C_ElementMgrNotificationElementDestroyedMsg elementDestroyedMsg(elementIndex);
		SendMessageToActor(&elementDestroyedMsg);

		if (m_ActiveElementCount > 0)
			SendMessageToActor(E_ElementMgrNotification_ElementStateChanged);
	}
}

// Spawns any break-offs.
void C_BaseDamageableElementManager::SpawnElementBreakoff(S32 elementIndex, const ActorDamage::C_DamageData* pDamageData,  const ActorDamage::C_DamageInfo& damageInfo)
{
	// Calculate a damage impulse:
	Maths::HVec3 impulse(Maths::Constants<HVec3>::UnitAxisPositiveY());	// Default

	if (pDamageData)
	{
		impulse = pDamageData->HitImpulse().Get();
	}

	const S32 replacementCount = ElementDamageLevelDef(elementIndex).m_ShatterReplacements.count;
	if (replacementCount > 0)
	{
		static Btl::Vector<S32> indices;
		indices.Resize(replacementCount);
		for (S32 r=0 ; r<replacementCount ; r++)
			indices[r] = r;

		// Remove some random indices:
		const XtRangeI8& range = ElementDamageLevelDef(elementIndex).m_ReplacementSpawnPercent;
		if (replacementCount>1 && range.to > 0)
		{
			F32 rangeTo = Maths::Lerp((F32)range.from, (F32)range.to, Lod::C_LodManager::Instance().GetCpuDetail());		
			static Util::C_Random rnd;
			S32 itemCount = indices.Size()-1;
			const F32 frac = rnd.FRand(range.from, rangeTo) * 0.01f;
			const S32 removalCount = S32((1.0f-frac) * replacementCount);
			BIZ_ASSERT(removalCount <= replacementCount);
			for (S32 r=0 ; r<removalCount ; r++)
				indices.EraseByIndexNonOrdered(rnd.Rand(itemCount--));
		}

		for (S32 r=indices.Size()-1 ; r>=0 ; r--)
		{
			const S32 index = indices[r];
			const BasePhysicalObject::C_Def& def = *static_cast<BasePhysicalObject::C_Def*>(ElementDamageLevelDef(elementIndex).m_ShatterReplacements[index]);
			const S32 modelElemIndex = def.m_ElementDefHierarchy[0]->m_DamageLevels[0]->ModelElementIndex();
			const GameResource::C_Model* pModel = ModelResource();
			if (pModel)
			{
				const GameResource::C_ModelElement* pElement = pModel->GetElement(modelElemIndex);
				if (pElement)
				{
					const S32 parentElemIndex = 0;	// Parent elem index not set for shatter replacements - attach to elem 0.
					const C_ElementMgrNotificationSpawnChildMsg spawnChildMsg(parentElemIndex, pElement->GetIndex());
					SpawnChild(spawnChildMsg);
				}			
				else
				{
					BIZ_ASSERT_FAIL("Physical object model '%s' does match Sorcery hierarchy - element '%d' not found!", def.m_Name.Text(), modelElemIndex);
				}
			}
		}
	}
}

void C_BaseDamageableElementManager::SpawnChild(const C_ElementMgrNotificationSpawnChildMsg& msg)
{
	SendMessageToActor(&msg);
}


// Spawns child objects for children underneath the given element index.
// Deactivates all elements below parentElementIndex branch.
void C_BaseDamageableElementManager::DetachElementChildren(S32 parentElementIndex)
{
	// Detach any children:
	const S32 childCount = FlatElementDefs()[parentElementIndex]->m_Children.count;
	for (S32 c=0 ; c<childCount ; c++)
	{
		const BasePhysicalObject::C_ElementObject& childDef = *FlatElementDefs()[parentElementIndex]->m_Children[c];

		// Find element index of child:
		bool detached = false;
		for (S32 def=0 ; def<childDef.m_ElementDefHierarchy.count && !detached ; def++)
		{
			for (S32 e=parentElementIndex+1 ; e<ElementCount() ; e++)
			{
				if (childDef.m_ElementDefHierarchy[def] == FlatElementDefs()[e])
				{
					const BasePhysicalObject::C_Def* physObjDef = static_cast<const BasePhysicalObject::C_Def*>(FlatElementDefs()[e]->m_ElementObject);
					const bool inheritCollideGroup = (physObjDef->m_BaseFlags & BasePhysicalObject::FlagCollideWithParent)==0;
					DetachChild(e, NULL, inheritCollideGroup);
					detached = true;
					break;
				}
			}
		}
	}
}


// Detach a child object node into a separate actor.
// Param elementIndex - the physObj element corresponding to the first element of the node to be detached.
// Note: relies on elements being mapped via depth-first hierarchy traversal.
void C_BaseDamageableElementManager::DetachChild(S32 elementIndex, const ActorDamage::C_DamageData* damageData, bool inheritCollideGroup)
{
	const BasePhysicalObject::C_ElementObject* objDef = FlatElementDefs()[elementIndex]->m_ElementObject;
	if (!objDef)
		return;

	if (!ElementDamageLevelDef(elementIndex).m_pShape)
	{
		// This is mainly to catch erroneous DetachElement messages.
		LOGWARN(Actors::g_Log, DBSTR"element %d has no collision shape", elementIndex);
		return;
	}

	const S32 modelElemIndex = ElementDamageLevelDef(elementIndex,0).ModelElementIndex();
	C_ElementMgrNotificationSpawnChildMsg msg(ElementState(elementIndex).m_ParentElementIndex, modelElemIndex, damageData, inheritCollideGroup);
	// Copy element states for detached branch (including inactive elements):
	S32 activeElemCount = 0;
	const S32 rec = RecursiveElementCount(*objDef);
	for (S32 e=0 ; e<rec ; e++)
	{
		msg.m_ElementStateSetup.PushBack(ElementState(elementIndex+e));
		activeElemCount += ElementIsActive(elementIndex+e) ? 1 : 0;
	}

	if (activeElemCount == 0)
		return;	// No active elements?  No spawny!

	// Calc any force override:
	const BasePhysicalObject::C_Def* physObjDef = static_cast<const BasePhysicalObject::C_Def*>(objDef);
	const bool overrideForce = (damageData && !damageData->IsDamageAreaEffect()) && (physObjDef->m_BaseFlags & BasePhysicalObject::FlagReflectDetachImpulse);
	if (overrideForce)
	{
		const Maths::HVec3 awayVec(ElementState(elementIndex).m_Matrix.AW()-GetActor()->GetMatrix().AW());
		const HF32 len = damageData ? damageData->HitImpulse().Get().Length() : (HF32)1.0f;
		Maths::HVec3 vec = awayVec.Normalised().Normal() * len * gElemMgrDetachImpulseScale;
		msg.SetForceOverride(vec);
	}

	SpawnChild(msg);

	// Deactivate elements for detached branch:
	// Note: this is like destroying the element but not quite.  May want another flag here..?
	for (S32 cc=rec-1 ; cc>=0 ; cc--)
	{
		const S32 elemIndex = elementIndex+cc;
		if (ElementIsActive(elemIndex))
		{
			ElementState(elemIndex).m_IsActive = false;
			m_ActiveElementCount--;
		}
		ElementState(elemIndex).m_Strength = 0.0f;	// = pseudo-destroyed, so we can't enable it later.
		BIZ_ASSERT(m_ActiveElementCount >= 0);
	}

	// Don't send ElementStateChanged msg here so multiple changes can be compounded.
}


const Maths::HMat43 C_BaseDamageableElementManager::ModelElementImportMatrix(S32 modelIndex) const
{
	const GameResource::C_Model* pModel = ModelResource();
	if (pModel)
		return pModel->GetElement(modelIndex)->GetPhysicsImportShape()->GetImportMatrix();
	else
		LOGERROR(Actors::g_Log, DBSTR"No model resource.  Calling from init?");

	return Maths::HMat43();
}


bool C_BaseDamageableElementManager::GetElementCollisionLocalPos(S32 elementIndex, Maths::HVec3& localPos) const
{
	BIZ_ASSERT(elementIndex>=0);

	const Gravity::C_ImportShape* importShape = CollisionShape(elementIndex);
	if (importShape && importShape->GetShapeDesc())
	{
		localPos = importShape->GetShapeDesc()->GetBoundingBox().centre;
		return true;
	}

	LOGWARN(Actors::g_Log, DBSTR"element %d has no collision shape", elementIndex);
	return false;
}


// Returns the model element for a given dynObj element index, or NULL.
const GameResource::C_ModelElement* C_BaseDamageableElementManager::ModelElement(S32 dynObjElementIndex) const
{
	const S32 modelElemIndex = ElementDamageLevelDef(dynObjElementIndex).ModelElementIndex();
	const GameResource::C_Model* pModel = ModelResource();
	if (pModel)
		return pModel->GetElement(modelElemIndex);
	else
		LOGERROR(Actors::g_Log, DBSTR"No model resource.  Calling from init?");

	return NULL;
}


// Finds a named element in this object's model.
const GameResource::C_ModelElement* C_BaseDamageableElementManager::FindModelElement(const Util::C_String& elementName) const
{
	const GameResource::C_Model* pModel = ModelResource();
	if (pModel)
		return pModel->GetElement(elementName);
	else
		LOGERROR(Actors::g_Log, DBSTR"No model resource.  Calling from init?");

	LOGWARN(Actors::g_Log, DBSTR"element '%s' not found.", (const char *)elementName);
	return NULL;
}


// Returns a model element by index.
const GameResource::C_ModelElement* C_BaseDamageableElementManager::FindModelElement(S32 modelElementIndex) const
{
	const GameResource::C_Model* pModel = ModelResource();
	if (pModel)
		return pModel->GetElement(modelElementIndex);
	else
		LOGERROR(Actors::g_Log, DBSTR"No model resource.  Calling from init?");

	LOGWARN(Actors::g_Log, DBSTR"element '%d' not found.", modelElementIndex);
	return NULL;
}


// Repairs an element.
// Note: More work needed if required for all-round dyn obj repair.
void C_BaseDamageableElementManager::RepairElement(int elementIndex, F32 amount)
{
	if (amount==0.0f || !ElementIsActive(elementIndex))
		return;

	if (amount > 0.0f)
	{
		C_ElementState& elemState = ElementState(elementIndex);
		elemState.m_Strength += amount;
		while (elemState.m_Strength > ElementDamageLevelDef(elementIndex).m_Strength  &&  elemState.m_DamageLevel > 0)
		{
			elemState.m_Strength -= ElementDamageLevelDef(elementIndex).m_Strength;
			elemState.m_DamageLevel--;
		}

		if (elemState.m_DamageLevel == 0)
		{
			if (elemState.m_Strength > ElementDamageLevelDef(elementIndex).m_Strength)
				elemState.m_Strength = ElementDamageLevelDef(elementIndex).m_Strength;
		}
		assert(elemState.m_Strength > 0.0f);
	}
	else
	{
		LOGWARN(Actors::g_Log, DBSTR"invalid strength value (%f)", amount);
	}
}


void C_BaseDamageableElementManager::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << m_TotalInflictedDamage;
	stream.WriteAs<bool>(m_SendInflictedDamageNotifications);

	const S32 count = m_ElementStates.Size();
	stream << count;
	for (int s=0 ; s<count ; s++)
		m_ElementStates[s].Serialise(stream);
}


void C_BaseDamageableElementManager::Deserialise(Sys::C_ContextualStream& stream)
{
	BIZ_ASSERT(m_ElementStates.Empty());

	stream >> m_TotalInflictedDamage;
	stream.ReadAs<bool>(m_SendInflictedDamageNotifications);

	S32 count = -1;
	stream >> count;
	for (int e=0 ; e<count ; e++)
	{
		C_ElementState state;

		state.Deserialise(stream);
		if (state.m_IsActive)
			m_ActiveElementCount++;

		m_ElementStates.PushBack(state);
	}

	m_DetachChildren = false;
}


} // namespace Components

#ifdef SHARED_ACTORS_OPTIMIZE_OFF
#pragma optimize("", on)
#endif
