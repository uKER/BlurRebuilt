
#ifndef _BASEDAMAGEABLEELEMENTMANAGERCOMPONENT_CREATE_H
#define _BASEDAMAGEABLEELEMENTMANAGERCOMPONENT_CREATE_H

#include <shared_actors/Components/Impl/BaseElementManagerComponents.h>


struct BasePhysicalObject::C_ElementObject;
namespace ResourceUtils
{
	struct C_GameModel;
}
class C_ModelCacheSpec;


namespace Components
{

///////////////////////////////////////////////////////////////////////////////////////////////////
// Damageable element mgr component creation info.
//
class C_BaseDamageableElementManagerCreateInfo : public I_GameComponentCreator
{
public:
	C_BaseDamageableElementManagerCreateInfo();

public: // From I_GameComponentCreator
	static Component::C_ComponentTypeId		ClassType() {return E_DamageableElementManager;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return E_DamageableElementManager;}

	XtHandle<ResourceUtils::C_GameModel>	GetModelDef() const;
	BasePhysicalObject::C_ModelCacheSpec*	GetModelCacheSpec() const;
	XtId									GetElementDefsRecordId() const;

// 	virtual void							Serialise(Sys::C_ContextualStream& stream) const;
// 	virtual void							Deserialise(Sys::C_ContextualStream& stream);

public:
	XtHandle<ResourceUtils::C_GameModel> m_Model;
	XtHandle<BasePhysicalObject::C_ElementObjectRecord> m_ElementObject;

	F32 m_DamagePercent;

	// Non-Xt stuff

	// For spawning child objects:
	//Util::C_String m_RootElementName;	// New element root for child object.
	S32 m_RootModelElementIndex;
	Btl::Vector<C_BaseDamageableElementManager::C_ElementState> m_ElementStateSetup;	// Set when spawning a child object.
};

} // namespace Components

#endif
