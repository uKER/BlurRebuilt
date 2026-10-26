#include "SharedActorsPch.h"
#include <shared_actors/ActorDef/BasePhysicalObjectDef.h>

#include <actor_system/Actor.h>
#include <actor_system/ActorList.h>
#include <actor_system/ActorDebug.h>
#include <game_resource/ModelData.h>
#include <game_resource/ModelResource.h>
#include <gravity/importer/Importer.h>
#include <misc_shared/Messaging/MessageSubscriberSystem.h>
#include <shared_actors/ActorMessages/ModelCacheMessages.h>
#include <shared_actors/XtActorMessages.h>

using namespace BasePhysicalObject;
namespace Actors { extern Debug::Logger &g_Log; }

#ifdef NIGHTWING_PHYSICS_OPTIMIZE_OFF
#pragma optimize("", off)
#endif

// Base Physical object definition
// -------------------------

// Physical object
XT_TYPE(BasePhysicalObject::Flags, "BasePhysicalObjectFlags")
	XT_FLAG_STORAGE(XtU32)
	XT_FLAG(FlagBlobShadow,								"BlobShadow")
	XT_FLAG(FlagBlobShadowOverrideRadius,				"BlobShadowOverrideRadius")
	XT_FLAG(FlagContinuousCollision,					"ContinuousCollision")
	XT_FLAG(FlagMakeStaticOnSettled,					"MakeStaticOnSettled")
	XT_FLAG(FlagRemoveBodyOnSettled,					"RemoveBodyOnSettled")
	XT_FLAG(FlagCollideWithParent,						"CollideWithParent")
	XT_FLAG(FlagReflectDetachImpulse,					"ReflectDetachImpulse")
XT_END_TYPE


XT_TYPE(ElementDamageLevelFlags, "BasePhysicalObjectElementDamageLevelFlags")
	XT_FLAG_STORAGE(XtU32)
	XT_FLAG(FlagElementDamageLevelIndestructible,		"Indestructible")
	XT_FLAG(FlagElementDamageLevelNoShadow,				"NoShadow")
	XT_FLAG(FlagElementDamageLevelEnablePivot,			"EnablePivot")
	XT_FLAG(FlagElementDamageLevelMakeDynamicOnBreak,	"MakeDynamicOnBreak")
	XT_FLAG(FlagElementDamageLevelDetachOnBreak,		"DetachOnBreak")
XT_END_TYPE


// collision body type enum
XT_TYPE(E_BodyType, "PhysicalObjectBodyType")
	XT_OPTION(E_BodyType_Dynamic,	"Dynamic")
	XT_OPTION(E_BodyType_Static,	"Static")
	XT_OPTION(E_BodyType_Keyframed, "Keyframed")
XT_END_TYPE


XT_TYPE(E_PivotRotationType, "PivotRotationType")
	XT_OPTION(E_PivotRotationType_XAxis, "XAxis")
	XT_OPTION(E_PivotRotationType_YAxis, "YAxis")
	XT_OPTION(E_PivotRotationType_ZAxis, "ZAxis")
	XT_OPTION(E_PivotRotationType_XYAxis, "XYAxis")
	XT_OPTION(E_PivotRotationType_XZAxis, "XZAxis")
	XT_OPTION(E_PivotRotationType_YZAxis, "YZAxis")
	XT_OPTION(E_PivotRotationType_XYZAxis, "XYZAxis")
	XT_OPTION(E_PivotRotationType_ExternalSet, "External")
	XT_OPTION(E_PivotRotationType_Scripted, "Scripted")
	XT_OPTION(E_PivotRotationType_AutoRotation, "AutoRotation")
XT_END_TYPE

XT_TYPE(C_PivotDef, "PivotDef")
	XT_FIELD(m_Name, "Name")
	XT_FIELD(m_PivotType, "PivotType")
	XT_FIELD(m_MinRotation, "MinRotation")
	XT_FIELD(m_MaxRotation, "MaxRotation")
	XT_FIELD(m_VelScale, "VelScale")
	XT_FIELD(m_AccelScale, "AccelScale")
	XT_FIELD(m_GravScale, "GravScale")
	XT_FIELD(m_AutoRotationRPM, "AutoRotationRPM")
XT_END_TYPE


// element damage level
XT_TYPE(C_ElementDamageLevelDef, "BasePhysicalObjectElementDamageLevel")
	XT_FIELD(m_ModelElementName, "ModelElementName")
	XT_FIELD(m_Strength, "Strength")
	XT_FIELD(m_DamageAmountThreshold, "DamageAmountThreshold")
	XT_FIELD(m_Mass, "Mass")
	XT_FIELD(m_BaseFlags, "BaseFlags")
	XT_FIELD(m_ShatterReplacements, "ShatterReplacements")
	XT_FIELD(m_ReplacementSpawnPercent, "ReplacementSpawnPercent")
XT_END_TYPE

// object element
XT_TYPE(C_ElementDef, "BasePhysicalObjectElement")
	XT_FIELD(m_Name, "Name")
	XT_FIELD(m_PivotDef, "PivotDef")
	XT_FIELD(m_PivotParented, "PivotParented")
	XT_FIELD(m_DontIncInflictedDamageVal, "DontIncInflictedDamageVal")
	XT_FIELD(m_DamageLevels, "DamageLevels")
	XT_FIELD(m_Children, "Children")
XT_END_TYPE

// Base for element-owning objects.
XT_TYPE(C_ElementObject, "PhysicalElementObject")
	XT_FIELD(m_ElementDefHierarchy, "Elements")
	XT_FIELD(m_Constraint, "Constraint")
	XT_FIELD(m_ConstraintModelElementName, "ConstraintModelElementName")
	XT_FIELD(m_DamageThresholdActions, "DamageThresholdActions")
	XT_FIELD(m_CentreOfMassOffset, "CentreOfMassOffset")
XT_END_TYPE


// Base physical object def
XT_TYPE(C_Def, "BasePhysicalObject")
	XT_BASE(C_ElementObject)
	XT_FIELD(m_Name, "Name" )
	//XT_FIELD(m_Model, "Model")
	XT_FIELD(m_DrawDistance, "DrawDistance")
	XT_FIELD(m_LODDistance, "LODDistance")
	XT_FIELD(m_BaseFlags, "BaseFlags")
	XT_FIELD(m_BlobShadowRadius, "BlobShadowRadius")
	XT_FIELD(m_BlobShadowIntensity, "BlobShadowIntensity")
	XT_FIELD(m_BlobShadowTexture, "BlobShadowTexture")
	XT_FIELD(m_BodyType, "BodyType")
	XT_FIELD(m_ContactImpulseThreshold, "ContactImpulseThreshold")
	XT_FIELD(m_StaticActivationThreshold, "StaticActivationThreshold")
	XT_FIELD(m_ImpactDamageThresholdScale, "ImpactDamageThresholdScale")
	XT_FIELD(m_BuoyancyCoeff, "BuoyancyScale")
	XT_FIELD(m_BuoyancyMaxCoeff, "BuoyancyMaxScale")
	XT_FIELD(m_BuoyancyLinearDragCoeff, "BuoyancyLinearDragScale")
	XT_FIELD(m_BuoyancyAngularDragCoeff, "BuoyancyAngularDragScale")
	XT_FIELD(m_AnimationPlayerCreate, "AnimationPlayer")
XT_END_TYPE

static void XtElementObjectRecordCallback(XtEventType type, XtRecord* pRecord, C_ElementObjectRecord* pDef, XtCallbackContext*)
{
	if (type == kXtEtConstruct)
	{
		C_ModelCacheSpec::NotifyInvalidated(pRecord);
	}
}

XT_TYPE(C_ElementObjectRecord, "ElementObjectRecord")
	XT_CALLBACK(XtElementObjectRecordCallback)
	XT_FIELD(m_Model, "Model")
	XT_FIELD(m_Objects, "objects")
	XT_FIELD(m_Elements, "objectElements")
	XT_FIELD(m_DamageLevels, "damageLevels")
	XT_FIELD(m_RootObject, "rootObject")
XT_END_TYPE


//////////////////////////////////////////////////////////////////////////
// Damage threshold stuff
//
XT_TYPE(C_DamageThresholdItem, "DamageThresholdItem")
	XT_FIELD(m_DamageAmount, "DamageAmount")
	XT_FIELD(m_Messages, "Messages")
XT_END_TYPE

XT_TYPE(C_DamageThresholdArray, "DamageThresholdArray")
	XT_FIELD(m_Items, "Items")
XT_END_TYPE


XT_LIBRARY(BasePhysicalObjects)
	XT_REGISTER_TYPE(BasePhysicalObject::Flags)
	XT_REGISTER_TYPE(ElementDamageLevelFlags)
	XT_REGISTER_TYPE(C_ElementDamageLevelDef)
	XT_REGISTER_TYPE(C_ElementDef)
	XT_REGISTER_TYPE(C_ElementObject)
	XT_REGISTER_TYPE(E_BodyType)
	XT_REGISTER_TYPE(C_Def)
	XT_REGISTER_TYPE(C_PivotDef)
	XT_REGISTER_TYPE(E_PivotRotationType)
	XT_REGISTER_TYPE(C_ElementObjectRecord)
	// Damage threshold stuff:
	XT_REGISTER_TYPE(C_DamageThresholdItem)
	XT_REGISTER_TYPE(C_DamageThresholdArray)
XT_END_LIBRARY


namespace BasePhysicalObject
{

S32 C_ElementDamageLevelDef::ModelElementIndex() const
{
	return m_ModelElementIndex;
}


void C_DamageThresholdArray::ProcessThresholds(const Actor::C_Actor* actor, F32 oldDmgTot, F32 newDmgTot) const
{
	for (S32 i=0 ; i<m_Items.count ; i++)
	{
		const C_DamageThresholdItem& item = m_Items[i];
		if (oldDmgTot < item.m_DamageAmount && newDmgTot >= item.m_DamageAmount)
		{
			for (S32 i=0 ; i<item.m_Messages.count ; i++)
			{
				if (item.m_Messages[i])
				{
					actor->GetActorList()->SendMessageToActor(actor->GetHandle(), item.m_Messages[i]->GetMessage(actor));
				}
				else
				{
					static S32 count = 0;
					if (count++ < 5)
						LOGWARN(Actors::g_Log, DBSTR"NULL damage threshold message");
				}
			}
		}
	}
}


static void ElemCount(S32& count, const XtArray<C_ElementDef*>& def)
{
	for (S32 e=0 ; e<def.count ; e++)
	{
		count++;
		for (S32 c=0 ; c<def[e]->m_Children.count ; c++)
			ElemCount(count, def[e]->m_Children[c]->m_ElementDefHierarchy);
	}
}


S32 C_ElementObject::ElementCount() const
{
	S32 count=0;
	ElemCount(count, m_ElementDefHierarchy);
	return count;
}


// Walks an element def tree to find a node for the given model element name.
// Param elemIndex - gets set to the index of the found element def in the returned elementObject.
static const C_ElementObject* FindDef(const C_ElementObject* elemObj, S32 modelElementIndex, S32& elemIndex)
{
	for (S32 e=0 ; e<elemObj->m_ElementDefHierarchy.count ; e++)
	{
		// Check elements:
		for (S32 d=0 ; d<elemObj->m_ElementDefHierarchy[e]->m_DamageLevels.count ; d++)
		{
			const C_ElementDamageLevelDef& dmgLevDef = *elemObj->m_ElementDefHierarchy[e]->m_DamageLevels[d];
			if (dmgLevDef.ModelElementIndex() == modelElementIndex)
			{
				elemIndex = e;
				return elemObj;
			}

			// Check in shatter replacements:
			const S32 srCount = dmgLevDef.m_ShatterReplacements.count;
			for (S32 s=0 ; s<srCount ; s++)
			{
				BIZ_ASSERT_MSG(dmgLevDef.m_ShatterReplacements[s] != elemObj, "FindDef has encountered a cyclic link from '%s'", dmgLevDef.m_ModelElementName.string);
				
				if (dmgLevDef.m_ShatterReplacements[s] != elemObj)
				{
					const C_ElementObject* obj = FindDef(dmgLevDef.m_ShatterReplacements[s], modelElementIndex, elemIndex);
					if (obj)
						return obj;
				}
			}
		}

		for (S32 c=0 ; c<elemObj->m_ElementDefHierarchy[e]->m_Children.count ; c++)
		{
			const C_ElementObject* obj = FindDef(elemObj->m_ElementDefHierarchy[e]->m_Children[c], modelElementIndex, elemIndex);
			if (obj)
				return obj;
		}
	}

	return NULL;
}


const C_ElementObject* C_ElementObject::FindChildDef(S32 modelElementIndex, S32& elemIndex) const
{
	return FindDef(this, modelElementIndex, elemIndex);
}


// Caches model element index, shape & pivot info.
U32 C_ElementDef::SetupModelData(const GameResource::C_ModelData* pModelData) const
{
	BIZ_ASSERT(pModelData);
	U32 errors = 0;

	// Cache damage level data:
	for (S32 damageLevel = 0; damageLevel < m_DamageLevels.count; damageLevel++)
	{
		if (!m_DamageLevels[damageLevel])
		{
			LOGWARN(Actor::GetDebugLogSorcery(), DBSTR"Dmg level %d is NULL", damageLevel);
			continue;
		}

		const C_ElementDamageLevelDef& dmgLev = *m_DamageLevels[damageLevel];

		// Cache model index & element shape for dmg level:
		const Util::C_String elemName(dmgLev.m_ModelElementName);
		const GameResource::C_Model* model = pModelData->GetRootModel();
		if (model)
		{
			const GameResource::C_ModelElement* pElement = model->GetElement(elemName);
			if (pElement)
			{
				const S32 index = pElement->GetIndex();
				dmgLev.SetModelElementIndex(index);
				dmgLev.m_pShape = NULL;
				const Gravity::C_ImportShape* importShape = pElement->GetPhysicsImportShape();
				if (importShape && importShape->GetShapeDesc())
				{
					if (importShape->GetShapeDesc()->ValidShapeDescriptor())
					{
						dmgLev.m_pShape = importShape;
					}
					else
					{
						LOGERROR(Actors::g_Log, DBSTR"Elem '%s', dmgLev %d: invalid shape descriptor", elemName.Chars(), damageLevel);
						errors++;
					}
				}

				m_Pivoted = false;
				if (m_PivotDef || m_PivotParented)
 				{
 					m_PivotMatrix = pElement->GetMatrix();
 					m_Pivoted = true;
					if (m_PivotParented)
					{
			 			m_PivotMatrix = m_PivotMatrix * pElement->GetParent()->GetMatrix().InverseRotTrans();
					}
 				}
			}
			else
			{
				LOGERROR(Actor::GetDebugLogSorcery(), "PhysObjElement '%s': Damage level %d: element '%s' NOT found", (const char *)m_Name, damageLevel, (const char *)dmgLev.m_ModelElementName);
				model->Dump();
				errors++;
			}
		}
		else
		{
			LOGERROR(Actor::GetDebugLogSorcery(), DBSTR"Element '%s' is NULL", elemName.Chars());
			errors++;
		}

// 		for (S32 r=0 ; r<dmgLev.m_ShatterReplacements.count ; r++)
// 		{
// 			dmgLev.m_ShatterReplacements[r]->m_ElementDefHierarchy[0]->SetupModelData(pModelData);
// 		}
	}

// 	for (S32 c=0 ; c<m_Children.count ; c++)
// 	{
// 		m_Children[c]->m_ElementDefHierarchy[0]->SetupModelData(pModelData);
// 	}

	return errors;
}


/**
@class BasePhysicalObject::C_ModelCacheSpec.

C_ModelCacheSpec objects should be embedded into any data type that comes from
Xt and contains C_ElementDef data. Before other code can safely use that data 
it must call SetupModelData() to cache data from the model resource into the
C_ElementDef data.

Those objects that embed C_ModelCacheSpec should have callbacks as part of 
their Xt annotations, and in response to construct events they should call
NotifyInvalidated(). If the game has called SetNotifyInvalidatedCallback() then 
this will allow it to inform all the code that use C_ElementDef data to call 
SetupModelData() again.

@code
// my game handles the notification like this...
void MyModelCacheSpedNotifyInvalidatedCallback(const XtRecord* pRecord)
{
	if (pRecord != NULL)
	{
		C_GameWorld* pGameWorld = C_GameWorld::GetInstance();

		if (pGameWorld != NULL)
		{
			Actor::C_ActorList* pActorList = pGameWorld->GetActorList();

			if (pActorList != NULL)
			{
				// in my game, every object that uses C_ElementDef subscribes to this message...
				const C_ModelCacheSpecInvalidatedMsg msg(pRecord->id);
				pActorList->BroadcastMessage(&msg);
			}
		}
	}
}
@endcode
*/

Messaging::C_MessageSubscriberSystem* C_ModelCacheSpec::s_pMessageSubscriberSystem = NULL;

C_ModelCacheSpec::C_ModelCacheSpec()
	: m_pModelDataUsedToSetup(NULL)
	, m_ModelErrors(0) 
{
}

C_ModelCacheSpec::~C_ModelCacheSpec()
{
}

/**
@param defs element defs to setup.
@param pModelData model resource data to use, must not be NULL.
@returns total number of errors encountered.
*/
U32 C_ModelCacheSpec::SetupModelData(const BasePhysicalObject::C_ElementObjectRecord& elemObjRecord, const GameResource::C_ModelData* pModelData)
{
	BIZ_ASSERT(pModelData != NULL);

	if (pModelData != m_pModelDataUsedToSetup)
	{
		m_ModelErrors = 0;
		for (S32 i=elemObjRecord.m_Elements.count-1 ; i>=0 ; i--)
			m_ModelErrors += elemObjRecord.m_Elements[i]->SetupModelData(pModelData);

		//printf("MCS: model data new:0x%x, old:0x%x, errors:%d\n", pModelData, m_pModelDataUsedToSetup, m_ModelErrors);
		m_pModelDataUsedToSetup = pModelData;
	}

	return m_ModelErrors;
}

/**
@param pMessageSubscriberSystem subscriber system that we broadcast notifications to.
*/
void C_ModelCacheSpec::SetMessageSubscriberSystem(Messaging::C_MessageSubscriberSystem* pMessageSubscriberSystem)
{
	s_pMessageSubscriberSystem = pMessageSubscriberSystem;
}

/**
@param pRecord record containing the invalidated data.
*/
void C_ModelCacheSpec::NotifyInvalidated(const XtRecord* pRecord)
{
	if (s_pMessageSubscriberSystem != NULL)
	{
		const C_ModelCacheInvalidatedMsg msg(pRecord ? pRecord->id : 0);
		s_pMessageSubscriberSystem->BroadcastMessage(&msg);
	}
}


// Finds an object def that has the given model element name in it.
const C_ElementObject* C_ElementObjectRecord::FindObjectDef(S32 modelElementIndex) const
{
	const S32 objCount = m_Objects.count;
	for (S32 i=0 ; i<objCount ; i++)
	{
		const BasePhysicalObject::C_ElementObject* obj = m_Objects[i];
		if (obj)
		{
			const XtArray<BasePhysicalObject::C_ElementDef*>& elements = obj->m_ElementDefHierarchy;
			for (S32 e=0 ; e<elements.count ; e++)
			{
				for (S32 d=0 ; d<elements[e]->m_DamageLevels.count ; d++)
				{
					if (elements[e]->m_DamageLevels[d]->ModelElementIndex() == modelElementIndex)
					{
						return obj;
					}
				}
			}
		}
	}

	return NULL;
}


} // namespace BasePhysicalObject

#ifdef NIGHTWING_PHYSICS_OPTIMIZE_OFF
#pragma optimize("", on)
#endif
