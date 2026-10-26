
#ifndef _BASEDAMAGEABLEELEMENTMANAGERCOMPONENT_H
#define _BASEDAMAGEABLEELEMENTMANAGERCOMPONENT_H

#include <shared_actors/Components/Interfaces/I_ElementManager.h>
#include <shared_actors/ActorDef/BasePhysicalObjectDef.h>
#include <btl/Containers/multimap.h>


namespace GameResource
{
	class C_ModelElement;
}

namespace ActorDamage
{
	class C_DamageData;
	class C_DamageInfo;
}

namespace BasePhysicalObject
{
	struct C_PivotInfo;
}

class C_ElementMgrNotificationSpawnChildMsg;


namespace Components
{

class C_BaseDamageableElementManagerCreateInfo;


///////////////////////////////////////////////////////////////////////////////////////////////////
// Damageable element manager component class.
//
class C_BaseDamageableElementManager : public I_DamageableElementManager
{
public:

	// Class storing the current state of each element.
	class C_ElementState
	{
	public:
		C_ElementState() : m_IsActive(false), m_DamageLevel(0), m_Strength(0.0f), m_Flags(0), m_ParentElementIndex(-1), m_ModelElementIndex(-1) {}	// Container requires default ctor.
		C_ElementState(S32 damageLevel, const BasePhysicalObject::C_ElementDamageLevelDef& dmgLevelDef) {Init(damageLevel, dmgLevelDef);}

		void Init(S32 damageLevel, const BasePhysicalObject::C_ElementDamageLevelDef& dmgLevelDef);

		void Serialise(Sys::C_ContextualStream& stream) const;
		void Deserialise(Sys::C_ContextualStream& stream);

	public:
		bool						m_IsActive;
		S32  						m_DamageLevel;
		F32  						m_Strength;
		U32  						m_Flags;
		Maths::HMat43				m_Matrix;
		Maths::HVec3				m_PivotRotationAngles;
		Maths::HVec3				m_ScriptedPivotRotation;
		F32							m_Time;
		S32							m_ParentElementIndex;
		//Util::C_String				m_ModelElementName;
		S32							m_ModelElementIndex;
		Actor::C_ActorHandle		m_UserHandle;
		Btl::Vector<Actor::C_ActorHandle> m_Constraints;	// Constraints to child objects attached to this element.
	};

	typedef Btl::Vector<C_ElementState> TElementStateList;

public:
	C_BaseDamageableElementManager();
	~C_BaseDamageableElementManager();

public:
	const C_BaseDamageableElementManagerCreateInfo* GetCreationData(void) const;

	S32 ActiveElementCount() const {return m_ActiveElementCount;}
	S32 ElementIndexFromActiveElementIndex(S32 n) const;
	S32 ElementCount() const {return m_ElementStates.Size();}
	F32 ElementStrength(S32 elementIndex) const {return ElementState(elementIndex).m_Strength;}
	const char* ElementName(S32 elementIndex) const;
	S32 ElementDamageLevelCount(S32 elementIndex) const;
	S32 ElementDamageLevel(S32 elementIndex) const {return ElementState(elementIndex).m_DamageLevel;}
	const Maths::HMat43& ElementMatrix(S32 elementIndex) const  {return ElementState(elementIndex).m_Matrix;}
	const Maths::HMat43 ModelElementImportMatrix(S32 modelIndex) const;
	S32 ElementIndex(const Util::C_String& elementName) const;
	bool IsElementIndestructible(S32 elementIndex) const {return ElementState(elementIndex).m_Flags & BasePhysicalObject::FlagElementDamageLevelIndestructible;}
	bool GetElementCollisionLocalPos(S32 elementIndex, Maths::HVec3& localPos) const;
	S32 ElementIndexFromFlatPrimitiveIndex(S32 primIndex) const;
	S32 PrimitiveIndexFromElementIndex(S32 elementIndex) const;
	const BasePhysicalObject::C_ElementDamageLevelDef& ElementDamageLevelDef(S32 elementIndex, S32 dmgLevel) const;
	const C_ElementState& ElementState(S32 elementIndex) const { assert(elementIndex>=0); return m_ElementStates[elementIndex]; }
	C_ElementState& ElementState(S32 elementIndex) { assert(elementIndex>=0); return m_ElementStates[elementIndex]; }
	bool ElementIsActive(S32 elementIndex) const {assert(elementIndex>=0); return ElementState(elementIndex).m_IsActive;}

	const BasePhysicalObject::C_ElementDamageLevelDef& ElementDamageLevelDef(S32 dynObjElementIndex) const { return ElementDamageLevelDef(dynObjElementIndex, ElementState(dynObjElementIndex).m_DamageLevel); }
	const Gravity::C_ImportShape* CollisionShape(S32 dynObjElementIndex) const;
	const Gravity::C_ImportShape* ActiveCollisionShape(S32 elementIndex) const;

	bool ContainsPivotedElement() const {return m_ContainsPivot;} 

	F32 TotalInflictedDamage() const {return m_TotalInflictedDamage;}

	void Serialise(Sys::C_ContextualStream& stream) const;
	void Deserialise(Sys::C_ContextualStream& stream);

public: // From I_ActorComponent
	static Component::C_ComponentTypeId		ClassType() {return E_DamageableElementManager;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return E_DamageableElementManager;}

	bool OnMessage(const Messaging::C_MessageBase* pMessage);

	const S32 GetElementCount() const {return ActiveElementCount();}
	Components::C_ElementState GetElement(S32 index) const;
	const BasePhysicalObject::TElementDefList& FlatElementDefs() const {return m_FlatElements;}

	void SerialiseForSave(Sys::C_ContextualStream& stream) const {I_DamageableElementManager::SerialiseForSave(stream); Serialise(stream);}
	void DeserialiseForSave(Sys::C_ContextualStream& stream) {I_DamageableElementManager::DeserialiseForSave(stream); Deserialise(stream);}
	void SerialiseForSync(Sys::C_ContextualStream& stream) const {I_DamageableElementManager::SerialiseForSync(stream); Serialise(stream);}
	void DeserialiseForSync(Sys::C_ContextualStream& stream) {I_DamageableElementManager::DeserialiseForSync(stream); Deserialise(stream);}

#ifndef CFG_GOLD
	void DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const;
#endif

protected:
	// from Component::I_GameComponent
	bool InitialiseImpl();
	bool CreateImpl();
	void InternalDependenciesCreatedImpl();

	void InitElementHierarchy(const BasePhysicalObject::C_ElementObject& elemObj, F32 damagePercent, S32 parentElementIndex, S32& errorCount);

	void UpdateElementMatrices();
	void UpdatePivots(const BasePhysicalObject::C_PivotInfo &pivotInfo);
	void UpdatePivot(const S32 elementIndex, const Maths::HVec3&rotation);
	void UpdateScriptedPivot(const S32 elementIndex, const Maths::HVec3&rotation, F32 time);
	void UpdatePhysicsShapes();

	const GameResource::C_Model* ModelResource() const;
	const GameResource::C_ModelElement* ModelElement(S32 dynObjElementIndex) const;
	const GameResource::C_ModelElement* FindModelElement(const Util::C_String& elementName) const;
	const GameResource::C_ModelElement* FindModelElement(S32 modelElementIndex) const;
	void EnableElement(S32 index, bool enable);
	void ReduceElementStrength(S32 elementIndex, F32 amount) {assert(amount >= 0.0f); ElementState(elementIndex).m_Strength -= amount;}
	void ChangeElementDamageLevel(S32 elementIndex, S32 damageLevel);
	void RepairElement(int elementIndex, F32 strength);
	void SetElementIndestructible(S32 elementIndex, bool set);
	virtual void DamageElement(S32 elementIndex, const ActorDamage::C_DamageData *pDamageData, const ActorDamage::C_DamageInfo& damageInfo);
	void BreakElement(S32 elementIndex, const ActorDamage::C_DamageData* pDamageData, const ActorDamage::C_DamageInfo& damageInfo, bool respectIndestructible=true);
	virtual void OnElementBreak(S32 elementIndex, const ActorDamage::C_DamageData* pDamageData, const ActorDamage::C_DamageInfo& damageInfo) {}
	void DestroyElement(S32 elementIndex, const ActorDamage::C_DamageData* pDamageData, const ActorDamage::C_DamageInfo& damageInfo);
	virtual void OnElementDestroy(S32 elementIndex, const ActorDamage::C_DamageData* pDamageData, const ActorDamage::C_DamageInfo& damageInfo) {}
	virtual void SpawnElementBreakoff(S32 elementIndex, const ActorDamage::C_DamageData* pDamageData,  const ActorDamage::C_DamageInfo& damageInfo);
	void DetachElementChildren(S32 parentElementIndex);
	void DetachChild(S32 elementIndex, const ActorDamage::C_DamageData* damageData=NULL, bool inheritCollideGroup=false);
	virtual void SpawnChild(const C_ElementMgrNotificationSpawnChildMsg& msg);
	Maths::HVec3 WrapAddRotation(const Maths::HVec3& base, const Maths::HVec3 &add);
	void IncInflictedDamageVal(F32 aVal);

protected:
	S8								m_ActiveElementCount;
	Btl::Vector<C_ElementState>		m_ElementStates;		// Current state of each element.
	bool							m_ContainsPivot;
	BasePhysicalObject::TElementDefList m_FlatElements;		// Flattened element hierarchy.
	F32								m_TotalInflictedDamage;
	F32								m_DamagePerSecVal;
	bool							m_SendInflictedDamageNotifications;
	bool							m_DetachChildren;
};

} // namespace Components

#endif
