#ifndef BASEPHYSICALOBJECTDEF_H
#define BASEPHYSICALOBJECTDEF_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/vector.h>
#include <resource_utils/ResourceUtilsDeclare.h>
#include <shared_Actors/ActorDef/BasePhysicalObjectEnums.h>
#include <shared_actors/PivotDefs.h>
#include <xt/XtGame.h>


namespace Actor
{
	class C_Actor;
}

namespace ActorMsg
{
	class C_XtActorMessageBase;
}

namespace Actors
{ 
	struct C_ConstraintTypeBase; 
}

namespace GameResource 
{ 
	class C_Model; 
	class C_ModelData;
	class C_ModelResource;
}

namespace Gravity 
{ 
	class C_ImportShape; 
}

namespace Messaging
{
	class C_MessageSubscriberSystem;
}

namespace Components
{
	class C_AnimationPlayerCreator;
}


//////////////////////////////////////////////////////////////////////////
//
namespace BasePhysicalObject
{

struct C_ElementObjectRecord;
struct C_ElementDef;

typedef Btl::Vector<const C_ElementDef*> TElementDefList;

/// Manages the runtime caching of data from model resources into element definitions.
class C_ModelCacheSpec
{
public:
	C_ModelCacheSpec();
	~C_ModelCacheSpec();

	U32				SetupModelData(const C_ElementObjectRecord& elemObjRecord, const GameResource::C_ModelData* pModelData); ///< Call to cache the data.

	static void		SetMessageSubscriberSystem(Messaging::C_MessageSubscriberSystem* pMessageSubscriberSystem); ///< Sets the subscriber system used for notifications.

	static void		NotifyInvalidated(const XtRecord* pRecord); ///< Broadcasts C_ModelCacheInvalidatedMsg.

private:
	const void*		m_pModelDataUsedToSetup; ///< cache the pointer used to setup data, so we can avoid repeating the work
	U32				m_ModelErrors; ///< total number of errors encountered

	static Messaging::C_MessageSubscriberSystem* s_pMessageSubscriberSystem; ///< subscriber system that we broadcast notifications to

};


//////////////////////////////////////////////////////////////////////////
// Damage threshold stuff:
//

struct C_DamageThresholdItem
{
	F32	m_DamageAmount;
	XtArray<ActorMsg::C_XtActorMessageBase*> m_Messages;
};

struct C_DamageThresholdArray
{
	XtArray<C_DamageThresholdItem> m_Items;

	// Non-Xt stuff
	bool Empty() const {return m_Items.count==0;}
	void ProcessThresholds(const Actor::C_Actor* actor, F32 oldDmgTot, F32 newDmgTot) const;
};



struct C_ElementObject;

// Element damage level
struct C_ElementDamageLevelDef
{
	XtSz8							m_ModelElementName;
	F32								m_Strength;
	F32								m_DamageAmountThreshold;
	F32								m_Mass;
	XtArray<C_ElementObject*>		m_ShatterReplacements;
	XtRangeI8						m_ReplacementSpawnPercent;
	XtFlags<U32, ElementDamageLevelFlags> m_BaseFlags;

	// non Xt stuff
	void SetModelElementIndex(S32 index) const {m_ModelElementIndex=index;}
	S32 ModelElementIndex() const;
	mutable const Gravity::C_ImportShape* m_pShape;
private:
	mutable S32						m_ModelElementIndex;
};


// object element
struct C_ElementDef
{
	C_ElementDef() : m_ElementObject(NULL) {}

	XtSz8							m_Name;
	XtHandle<C_PivotDef>			m_PivotDef;
	bool							m_PivotParented;
	bool							m_DontIncInflictedDamageVal;
	XtArray<C_ElementDamageLevelDef*> m_DamageLevels;
	XtArray<C_ElementObject*>		m_Children;

	// non Xt stuff
	virtual U32						SetupModelData(const GameResource::C_ModelData* pModelData) const;
	virtual void					AddToResourceBag(ResourceUtils::I_ResourceBag& bag) const {}

	mutable Maths::HMat43			m_PivotMatrix;
	mutable bool					m_Pivoted;
	mutable const C_ElementObject*	m_ElementObject;
};


// This is an 'object' node in the hierarchy.
struct C_ElementObject
{
	virtual ~C_ElementObject() {}

	XtArray<C_ElementDef*> m_ElementDefHierarchy;
	C_DamageThresholdArray m_DamageThresholdActions;
	Actors::C_ConstraintTypeBase* m_Constraint;
	XtSz8 m_ConstraintModelElementName;
	XtVec3 m_CentreOfMassOffset;

	// Non-Xt stuff
	S32 ElementCount() const;
	const C_ElementObject* FindChildDef(S32 modelElementIndex, S32& elemIndex) const;
	mutable C_ModelCacheSpec m_ModelCacheSpec;

	// Type stuff.
	enum TTypeId
	{
		E_Invalid,
		E_PhysicalObject,
		E_Vehicle,
		E_Boat,

		KTypeCount
	};
	virtual TTypeId TypeId() const {return E_Invalid;}

	const char* TypeName() const
	{
		return TypeId()==E_PhysicalObject ? "PhysObj" : (TypeId()==E_Vehicle ? "Vehicle" : (TypeId()==E_Boat ? "Boat" : "Invalid"));
	}
};


//////////////////////////////////////////////////////////////////////////
// Base physical object def.
//
struct C_Def : public C_ElementObject
{
	C_Def() : m_AnimationPlayerCreate(NULL) {}

	XtSz8								m_Name;
	F32									m_DrawDistance;
	F32									m_LODDistance;
	XtFlags<U32, Flags>					m_BaseFlags;
	F32									m_BlobShadowRadius;
	F32									m_BlobShadowIntensity;
	U32									m_BlobShadowTexture;
	E_BodyType							m_BodyType;
	F32									m_ContactImpulseThreshold;
	F32									m_StaticActivationThreshold;
	F32									m_ImpactDamageThresholdScale;
	F32									m_BuoyancyCoeff;
	F32									m_BuoyancyMaxCoeff;
	F32									m_BuoyancyLinearDragCoeff;
	F32									m_BuoyancyAngularDragCoeff;
	Components::C_AnimationPlayerCreator* m_AnimationPlayerCreate;

	// Non Xt stuff
	U32									m_NetUid;
	virtual TTypeId TypeId() const {return BasePhysicalObject::C_Def::E_PhysicalObject;}
};


// Object 'container' record for node graph.
struct C_ElementObjectRecord
{
	const C_ElementObject* FindObjectDef(S32 modelElementIndex) const;

	C_ElementObject* m_RootObject;	// Ref to root object def.
	XtHandle<ResourceUtils::C_GameModel> m_Model;

	// Where the actual defs are stored:
	XtArray<C_ElementObject*> m_Objects;
	XtArray<C_ElementDef*> m_Elements;
	XtArray<C_ElementDamageLevelDef*> m_DamageLevels;
};


} // namespace BasePhysicalObject


#endif
