
#ifndef _ELEMENTMANAGERMESSAGES_H
#define _ELEMENTMANAGERMESSAGES_H

#include <misc_shared/uid/FourCC.h>
#include <shared_actors/XtActorMessages.h>
#include <shared_actors/ActorDamage.h>
#include <shared_actors/Components/Impl/BaseElementManagerComponents.h>
#include <shared_actors/PivotDefs.h>
#include <misc_shared/Messaging/MessageValue.h>

typedef Util::C_String32 TElementName;


// Element Manager component messages:

enum E_ElementManagerMsg
{
	E_ElementMgrMsg_HiBits = FOURCC_HI20('ELMG'), 

	E_ElementMgrMsg_EnableElement,
//	E_ElementMgrMsg_ReduceElementStrength,
	E_ElementMgrMsg_ChangeElementDamageLevel,
	E_ElementMgrMsg_DamageElement,
	E_ElementMgrMsg_BreakElement,
	E_ElementMgrMsg_BreakNamedElement,
	E_ElementMgrMsg_DestroyElement,
	E_ElementMgrMsg_DestroyNamedElement,
	E_ElementMgrMsg_SetElementIndestructible,
	E_ElementMgrMsg_SetAllElementsIndestructible,
	E_ElementMgrMsg_UpdateElementMatricies,
	E_ElementMgrMsg_UpdatePivots,
	E_ElementMgrMsg_UpdatePivot,
	E_ElementMgrMsg_UpdateScriptedPivot,
	E_ElementMgrMsg_UpdatePhysicsShapes,
	E_ElementMgrMsg_IncInflictedDamage,
	E_ElementMgrMsg_AddDependentConstraint,
	E_ElementMgrMsg_EnableInflictedDamageNotifications,
	E_ElementMgrMsg_DetachElement,
	// Notifications:
	E_ElementMgrNotification_ElementBroken,
	E_ElementMgrNotification_ElementDestroyed,
	E_ElementMgrNotification_ElementStateChanged,	// Signals shape rebuild required.
	E_ElementMgrNotification_MakeDynamic,
	E_ElementMgrNotification_SpawnChild,
	E_ElementMgrNotification_InflictedDamageValIncreased,
	
	E_ElementMgrMsg_End, 
	E_ElementMgrMsg_Begin = E_ElementMgrMsg_HiBits + 1, 
	E_ElementMgrMsg_Count = E_ElementMgrMsg_End - E_ElementMgrMsg_Begin
};


class C_ElementMgrEnableElementMsg : public Messaging::C_MessageDerived<C_ElementMgrEnableElementMsg, E_ElementMgrMsg_EnableElement>
{
public:
	C_ElementMgrEnableElementMsg(S32 elementIndex, bool enable) : m_ElementIndex(elementIndex), m_Enable(enable) {}
	S32 m_ElementIndex;
	bool m_Enable;
};


// class C_ElementMgrReduceElementStrengthMsg : public Messaging::C_MessageDerived<C_ElementMgrReduceElementStrengthMsg, E_ElementMgrMsg_ReduceElementStrength>
// {
// public:
// 	C_ElementMgrReduceElementStrengthMsg(S32 elementIndex, F32 amount) : m_ElementIndex(elementIndex), m_Amount(amount) {}
// 	S32 m_ElementIndex;
// 	F32 m_Amount;
// };


class C_ElementMgrChangeElementDamageLevelMsg : public Messaging::C_MessageDerived<C_ElementMgrChangeElementDamageLevelMsg, E_ElementMgrMsg_ChangeElementDamageLevel>
{
public:
	C_ElementMgrChangeElementDamageLevelMsg() : m_DamageLevel(-1) {}
	C_ElementMgrChangeElementDamageLevelMsg(const Util::C_String& elementName, S32 damageLevel) : m_ElementName(elementName), m_DamageLevel(damageLevel)
		{BIZ_ASSERT_MSG(elementName.Length()<m_ElementName.Size(), "String too large");}
	TElementName m_ElementName;
	S32 m_DamageLevel;
};


class C_ElementMgrDamageElementMsg : public Messaging::C_MessageDerived<C_ElementMgrDamageElementMsg, E_ElementMgrMsg_DamageElement>
{
public:
	C_ElementMgrDamageElementMsg(S32 elementIndex, const ActorDamage::C_DamageData* damageData, const ActorDamage::C_DamageInfo& damageInfo)
		: m_ElementIndex(elementIndex),
		m_DamageDataSet(damageData!=NULL),
		m_DamageInfo(damageInfo)
	{
		if (damageData)
			m_DamageData = *damageData;
	}

	const ActorDamage::C_DamageData& DamageData() const {assert(m_DamageDataSet); return m_DamageData;}
	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage);
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage);

	void SetDamageAmount(F32 amount) {m_DamageData.m_DamageAmount=amount;}

	S32 m_ElementIndex;
	bool m_DamageDataSet;
	ActorDamage::C_DamageInfo m_DamageInfo;
private:
	ActorDamage::C_DamageData m_DamageData;
	static Messaging::C_MessageSerialiser s_Serialiser;
};


class C_ElementMgrBreakElementMsg : public Messaging::C_MessageDerived<C_ElementMgrBreakElementMsg, E_ElementMgrMsg_BreakElement>
{
public:
	C_ElementMgrBreakElementMsg(){}
	C_ElementMgrBreakElementMsg(S32 elementIndex, const ActorDamage::C_DamageData* damageData, const ActorDamage::C_DamageInfo& damageInfo)
		: m_ElementIndex(elementIndex),
		m_DamageDataSet(damageData!=NULL),
		m_DamageInfo(damageInfo)
	{
		if (damageData)
			m_DamageData = *damageData;
	}

	const ActorDamage::C_DamageData& DamageData() const {assert(m_DamageDataSet); return m_DamageData;}
	void SetDamageData(const ActorDamage::C_DamageData* damageData) {m_DamageData=*damageData;m_DamageDataSet=true;}
	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage);
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage);

	S32 m_ElementIndex;
	bool m_DamageDataSet;
	ActorDamage::C_DamageInfo m_DamageInfo;
private:
	ActorDamage::C_DamageData m_DamageData;
	static Messaging::C_MessageSerialiser s_Serialiser;
};


class C_ElementMgrDestroyElementMsg : public Messaging::C_MessageDerived<C_ElementMgrDestroyElementMsg, E_ElementMgrMsg_DestroyElement>
{
public:
	C_ElementMgrDestroyElementMsg() {}
	C_ElementMgrDestroyElementMsg(S32 elementIndex, const ActorDamage::C_DamageData* damageData, const ActorDamage::C_DamageInfo& damageInfo)
		: m_ElementIndex(elementIndex),
		m_DamageDataSet(damageData!=NULL),
		m_DamageInfo(damageInfo)
	{
		if (damageData)
			m_DamageData = *damageData;
	}

	const ActorDamage::C_DamageData& DamageData() const {assert(m_DamageDataSet); return m_DamageData;}
	void SetDamageData(const ActorDamage::C_DamageData* damageData) {m_DamageData=*damageData;m_DamageDataSet=true;}
	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage);
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage);

	S32 m_ElementIndex;
	bool m_DamageDataSet;
	ActorDamage::C_DamageInfo m_DamageInfo;
private:
	ActorDamage::C_DamageData m_DamageData;
	static Messaging::C_MessageSerialiser s_Serialiser;
};


class C_ElementMgrSetElementIndestructibleMsg : public Messaging::C_MessageDerived<C_ElementMgrSetElementIndestructibleMsg, E_ElementMgrMsg_SetElementIndestructible>
{
public:
	C_ElementMgrSetElementIndestructibleMsg() {}
	C_ElementMgrSetElementIndestructibleMsg(const Util::C_String& elementName, bool set) : m_ElementName(elementName), m_Set(set) {}

	TElementName m_ElementName;
	bool m_Set;
};


typedef Messaging::C_MessageValue<bool, E_ElementMgrMsg_SetAllElementsIndestructible> C_ElementMgrSetAllElementsIndestructibleMsg;


class C_ElementMgrDetachElementMsg : public Messaging::C_MessageDerived<C_ElementMgrSetElementIndestructibleMsg, E_ElementMgrMsg_DetachElement>
{
public:
	C_ElementMgrDetachElementMsg() {}
	C_ElementMgrDetachElementMsg(const Util::C_String& elementName, const Maths::HVec3& impulse=Maths::HVec3()) : m_ElementName(elementName), m_Impulse(impulse)
		{BIZ_ASSERT_MSG(elementName.Length()<m_ElementName.Size(), "String too large");}
	TElementName m_ElementName;
	Maths::HVec3 m_Impulse;
};


typedef Messaging::C_MessageValue<S32, E_ElementMgrNotification_ElementBroken> C_ElementMgrNotificationElementBrokenMsg;

typedef Messaging::C_MessageValue<S32, E_ElementMgrNotification_ElementDestroyed> C_ElementMgrNotificationElementDestroyedMsg;


class C_ElementMgrNotificationSpawnChildMsg : public Messaging::C_MessageDerived<C_ElementMgrNotificationSpawnChildMsg, E_ElementMgrNotification_SpawnChild>
{
public:
	explicit C_ElementMgrNotificationSpawnChildMsg(S32 parentElementIndex, S32 modelElementIndex, const ActorDamage::C_DamageData* damageData=NULL, bool inheritCollideGroup=false)
		: m_ParentElementIndex(parentElementIndex), m_ModelElementIndex(modelElementIndex), m_DamageDataSet(damageData!=NULL), m_InheritCollideGroup(inheritCollideGroup), m_ForceOverrideSet(false)
	{
		if (damageData)
			m_DamageData = *damageData;
	}

	void SetForceOverride(const Maths::HVec3& force) {m_ForceOverrideSet=true; Maths::Store(force, &m_ForceOverride);}
	bool ForceOverrideSet() const {return m_ForceOverrideSet;}
	Maths::HVec3 ForceOverride() const {BIZ_ASSERT(m_ForceOverrideSet); return Maths::Load<Maths::HVec3>(&m_ForceOverride);}

	S32 m_ParentElementIndex;	// PhysObj parent element index.
	S32 m_ModelElementIndex;
	Components::C_BaseDamageableElementManager::TElementStateList m_ElementStateSetup;	// Optional.
	const ActorDamage::C_DamageData* DamageData() const {return m_DamageDataSet ? &m_DamageData : NULL;}
	bool m_InheritCollideGroup;

private:
	bool m_DamageDataSet;
	ActorDamage::C_DamageData m_DamageData;
	bool m_ForceOverrideSet;
	MathsStorage::Float3 m_ForceOverride;
};


class C_ElementMgrNotificationInflictedDamageValIncreased : public Messaging::C_MessageDerived<C_ElementMgrNotificationInflictedDamageValIncreased, E_ElementMgrNotification_InflictedDamageValIncreased>
{
public:
	C_ElementMgrNotificationInflictedDamageValIncreased(F32 oldVal, F32 newVal) : m_OldVal(oldVal), m_NewVal(newVal) {}
	F32 m_OldVal;
	F32 m_NewVal;
};


class C_ElementMgrUpdatePivotsMsg : public Messaging::C_MessageDerived<C_ElementMgrUpdatePivotsMsg, E_ElementMgrMsg_UpdatePivots>
{
public:
	C_ElementMgrUpdatePivotsMsg(const BasePhysicalObject::C_PivotInfo& pivotInfo)
		: m_PivotInfo(pivotInfo) {}

	BasePhysicalObject::C_PivotInfo m_PivotInfo;
};

class C_ElementMgrUpdatePivotMsg : public Messaging::C_MessageDerived<C_ElementMgrUpdatePivotMsg, E_ElementMgrMsg_UpdatePivot>
{
public:
	C_ElementMgrUpdatePivotMsg(const S32 elementIndex, const Maths::HVec3& rotation)
		: m_ElementIndex(elementIndex), m_Rotation(rotation) {}

	S32 m_ElementIndex;
	Maths::HVec3 m_Rotation;
};


typedef Messaging::C_MessageValue<F32, E_ElementMgrMsg_IncInflictedDamage> C_ElementMgrIncInflictedDamageMsg;


class C_ElementMgrAddDependentConstraintMsg : public Messaging::C_MessageDerived<C_ElementMgrAddDependentConstraintMsg, E_ElementMgrMsg_AddDependentConstraint>
{
public:
	C_ElementMgrAddDependentConstraintMsg(S32 elementIndex, const Actor::C_ActorHandle& constraintHandle)
		: m_ElementIndex(elementIndex), m_ConstraintHandle(constraintHandle) {}

	S32 m_ElementIndex;
	Actor::C_ActorHandle m_ConstraintHandle;
};


class C_ElementMgrBreakNamedElementMsg : public Messaging::C_MessageDerived<C_ElementMgrBreakNamedElementMsg, E_ElementMgrMsg_BreakNamedElement>
{
public:
	C_ElementMgrBreakNamedElementMsg() : m_RespectIndestructible(true) {}
	C_ElementMgrBreakNamedElementMsg(const Util::C_String& elemName, bool respectIndestructible)
		: m_ElementName(elemName), m_RespectIndestructible(respectIndestructible)
		{BIZ_ASSERT_MSG(elemName.Length()<m_ElementName.Size(), "String too large");}

	TElementName m_ElementName;
	bool m_RespectIndestructible;
};


class C_ElementMgrDestroyNamedElementMsg : public Messaging::C_MessageDerived<C_ElementMgrDestroyNamedElementMsg, E_ElementMgrMsg_DestroyNamedElement>
{
public:
	C_ElementMgrDestroyNamedElementMsg() : m_RespectIndestructible(true) {}
	C_ElementMgrDestroyNamedElementMsg(const Util::C_String& elemName, bool respectIndestructible)
		: m_ElementName(elemName), m_RespectIndestructible(respectIndestructible)
		{BIZ_ASSERT_MSG(elemName.Length()<m_ElementName.Size(), "String too large");}

	TElementName m_ElementName;
	bool m_RespectIndestructible;
};


class C_ElementMgrUpdateScriptedPivotMsg : public Messaging::C_MessageDerived<C_ElementMgrUpdateScriptedPivotMsg, E_ElementMgrMsg_UpdateScriptedPivot>
{
public:
	C_ElementMgrUpdateScriptedPivotMsg(){}
	C_ElementMgrUpdateScriptedPivotMsg(const Util::C_String& elemName, const Maths::HVec3& rotation, F32 time)
		: m_ElementName(elemName), m_Rotation(rotation), m_Time(time)
		{BIZ_ASSERT_MSG(elemName.Length()<m_ElementName.Size(), "String too large");}

	TElementName m_ElementName;
	Maths::HVec3 m_Rotation;
	F32 m_Time;
};


#endif
