
#include "stdafx.h"

#include <resource/Resource.h>

namespace Resource
{

struct C_TypeRegistry::C_TypeInfo
{
	Util::C_String	m_TypeName;
	F_Create		m_pCreate;
};

C_TypeRegistry* C_TypeRegistry::GetInstance()
{
	return _GetInstance();
}

C_TypeRegistry*& C_TypeRegistry::_GetInstance()
{
	static C_TypeRegistry* g_pRegistry = NULL;
	if( !g_pRegistry )
	{
		g_pRegistry = new C_TypeRegistry;
	}

	return g_pRegistry;
}

void C_TypeRegistry::DeleteInstance()
{
	//Type registry instance pointer is held within GetInstanceMember as static. 
	//SAFE_DELETE_PTR is macro so take reference to avoid calling GetInstance() multiple times. 
	C_TypeRegistry*& g_pTypeRegistry = C_TypeRegistry::_GetInstance();
	SAFE_DELETE_PTR( g_pTypeRegistry );
}

C_ResourceTypeId C_TypeRegistry::RegisterResourceType( const Util::C_String& resourceTypeName, const F_Create pCreate)
{
	assert( GetResourceTypeId( resourceTypeName ) == -1 ); // duplicate type name detected, fix it!
	assert( pCreate != NULL );

	const S32 resourceTypeCount = m_TypeInfoList.Size();

	//CORE_PRINT( "Registered resource type[%d] = '%s'\n", resourceTypeCount, resourceTypeName.Chars() );

	m_TypeInfoList.Resize( resourceTypeCount + 1 );

	C_TypeInfo& typeInfo = m_TypeInfoList[resourceTypeCount];
	typeInfo.m_TypeName = resourceTypeName;
	typeInfo.m_pCreate = pCreate;

	return resourceTypeCount;
}

C_ResourceTypeId C_TypeRegistry::GetResourceTypeId( const Util::C_String& resourceTypeName )
{
	const S32 resourceTypeCount = m_TypeInfoList.Size();
	for(S32 resourceTypeNo=0; resourceTypeNo<resourceTypeCount; resourceTypeNo++)
	{
		if( m_TypeInfoList[resourceTypeNo].m_TypeName == resourceTypeName )
			return resourceTypeNo;
	}

	return -1;
}

Util::C_String C_TypeRegistry::GetResourceTypeName( const C_ResourceTypeId typeId )
{
	if( typeId < m_TypeInfoList.Size() )
	{
		return m_TypeInfoList[typeId].m_TypeName;
	}

	return "<Unknown>";
}

C_ResourceBase* C_TypeRegistry::CreateResource( const C_ResourceTypeId typeId )
{
	if( typeId < m_TypeInfoList.Size() )
	{
		return m_TypeInfoList[typeId].m_pCreate();
	}

	return NULL;
}

void C_TypeRegistry::DestroyResource( C_ResourceBase* pResourceBase )
{
	if( pResourceBase != NULL )
	{
		delete pResourceBase;
	}
}

} // namespace Resource
