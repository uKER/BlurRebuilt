
#include "Stdafx.h"

#include "ui/Illustrator/UIAdobeGroup.h"
#include "ui/Illustrator/UIAdobeShape.h"

namespace UI
{

C_AdobeGroup::C_AdobeGroup()
:	C_AdobeElement( Ipf::E_ElementType_Group ),
	m_NumberOfShapes( 0 ),
	m_NumberOfGroups( 0 )
{
}


C_AdobeGroup::~C_AdobeGroup()
{
	DeleteElements();
}


void C_AdobeGroup::RemoveElements()
{
	m_ElementList.Clear();
	m_NumberOfShapes = 0;
	m_NumberOfGroups = 0;
}


void C_AdobeGroup::DeleteElements()
{
	for( S32 i = 0; i < m_ElementList.Size(); i++ )
	{
		SAFE_DELETE_PTR( m_ElementList[i] );
	}
	RemoveElements(); // Reset the data.
}


void C_AdobeGroup::AddElement( C_AdobeElement* pElement )
{
	assert( pElement );

	m_ElementList.PushBack( pElement );

	if ( pElement->GetElementType() == Ipf::E_ElementType_Shape )
		m_NumberOfShapes++;
	if ( pElement->GetElementType() == Ipf::E_ElementType_Group )
		m_NumberOfGroups++;
}


void C_AdobeGroup::SetCapacity(S32 capacity)
{
	m_ElementList.SetCapacity( capacity );
}

const C_AdobeElement* C_AdobeGroup::GetElementPtr( const S32 index ) const
{
	assert( index >= 0 && index < m_ElementList.Size() );

	return m_ElementList[index];
}


const C_AdobeElement* C_AdobeGroup::GetElementPtrByName( const char* name ) const 
{
	for ( S32 i = 0; i < m_ElementList.Size(); i++ )
	{
		if ( m_ElementList[i]->GetName() == name )
			m_ElementList[i];
	}
	return NULL;
}


const C_AdobeGroup* C_AdobeGroup::GetGroup( const S32 group ) const
{
	if ( m_NumberOfGroups <= 0 || group > m_NumberOfGroups )
		return NULL;

	S32 count = 0;
	for ( S32 i = 0; i < m_ElementList.Size(); i++ )
	{
		if ( m_ElementList[i]->GetElementType() == Ipf::E_ElementType_Group )
		{
			if ( group == count )
				return (C_AdobeGroup*)m_ElementList[i];
			count++;
		}
	}
	return NULL;
}

const C_AdobeGroup* C_AdobeGroup::GetGroupByName( const char* name ) const
{
	if ( m_NumberOfGroups <= 0 )
		return NULL;

	for ( S32 i = 0; i < m_ElementList.Size(); i++ )
	{
		if ( m_ElementList[i]->GetElementType() == Ipf::E_ElementType_Group )
		{
			if ( m_ElementList[i]->GetName().Lower() == name )
				return (C_AdobeGroup*)m_ElementList[i];
		}
	}
	return NULL;
}

const C_AdobeShape* C_AdobeGroup::GetShape( const S32 shape ) const
{
	if ( m_NumberOfShapes <= 0 || shape > m_NumberOfShapes )
		return NULL;

	S32 count = 0;
	for ( S32 i = 0; i < m_ElementList.Size(); i++ )
	{
		if ( m_ElementList[i]->GetElementType() == Ipf::E_ElementType_Shape )
		{
			if ( shape == count )
				return (C_AdobeShape*)m_ElementList[i];
			count++;
		}
	}
	return NULL;
}


}// namespace UI
