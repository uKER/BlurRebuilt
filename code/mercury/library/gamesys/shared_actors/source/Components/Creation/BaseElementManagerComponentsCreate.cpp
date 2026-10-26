
#include "SharedActorsPch.h"
//#include <misc_shared/Context/ContextualStream.h>
#include "shared_actors/Components/Creation/BaseElementManagerComponentsCreate.h"


namespace Components
{

C_BaseDamageableElementManagerCreateInfo::C_BaseDamageableElementManagerCreateInfo()
  : m_DamagePercent(0.0f),
	m_RootModelElementIndex(-1)
{
}

XtHandle<ResourceUtils::C_GameModel> C_BaseDamageableElementManagerCreateInfo::GetModelDef() const
{
	return m_Model;
}


BasePhysicalObject::C_ModelCacheSpec* C_BaseDamageableElementManagerCreateInfo::GetModelCacheSpec() const
{
	return &m_ElementObject->m_RootObject->m_ModelCacheSpec;
}

XtId C_BaseDamageableElementManagerCreateInfo::GetElementDefsRecordId() const
{
	return m_ElementObject.Id();
}


// const XtHandle<BasePhysicalObject::C_ElementObject> C_BaseDamageableElementManagerCreateInfo::GetElementObject() const 
// {
// 	return m_ElementObject->m_RootObject;
// }


// No XtHandle serialisation...
// void C_BaseDamageableElementManagerCreateInfo::Serialise(Sys::C_ContextualStream& stream) const
// {
// 	I_GameComponentCreator::Serialise(stream);
// 	stream << m_ElementObject;
// 	stream << m_Model;
// 	stream << m_RootElementName;
// 	stream << m_DamagePercent;
// }
// 
// void C_BaseDamageableElementManagerCreateInfo::Deserialise(Sys::C_ContextualStream& stream)
// {
// 	I_GameComponentCreator::Deserialise(stream);
// 	stream >> m_ElementObject;
// 	stream >> m_Model;
// 	stream >> m_RootElementName;
// 	stream >> m_DamagePercent;
// }

} //namespace Components
