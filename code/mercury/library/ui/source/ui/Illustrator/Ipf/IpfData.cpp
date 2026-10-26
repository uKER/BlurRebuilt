
#include "stdafx.h"
#include "Ui/Illustrator/Ipf/IpfData.h"

#include "Ui/Illustrator/UIAdobePolygon.h"
#include "UI/Illustrator/UIAdobeGroup.h"

using namespace UI;

namespace Ipf
{

C_IpfData::C_IpfData()
:	m_pCurrentLayer( NULL ),
	m_pCurrentGroup( NULL ),
	m_CurrentLevel( 1 ),
	m_NumGroups( 0 ),
	m_NumShapes( 0 )
{
}


C_IpfData::~C_IpfData()
{
	FlushData();
	FlushGradients();
	Destroy();
}


void C_IpfData::FlushData()
{
	for ( S32 layer = 0; layer < m_LayerList.Size(); layer++ )
	{
		m_LayerList[layer]->DeleteElements();
	}
	m_LayerList.Clear();

	m_NumGroups = 0;
	m_NumShapes = 0;
}


void C_IpfData::FlushGradients()
{
	for ( S32 gradient = 0; gradient < m_GradientList.Size(); gradient++ )
	{
		SAFE_DELETE_PTR( m_GradientList[ gradient ] );
	}
	m_GradientList.Clear();
}


void C_IpfData::Destroy()
{
	m_LayerList.Destroy();
	m_GradientList.Destroy();
}


void C_IpfData::AddLayer( const char* name )
{
	C_AdobeGroup* pLayer = new C_AdobeGroup();
	pLayer->Setup( 0, name );

	m_LayerList.PushBack( pLayer );
	m_pCurrentLayer = pLayer;
	m_pCurrentGroup = NULL;
	m_CurrentLevel = 1;
}


void C_IpfData::AddGroup( const char* name )
{
	if ( !m_pCurrentLayer )
		return;

	if ( m_pCurrentGroup )
	{
		// Add group to currently opened group..
		C_AdobeGroup* pGroup = new C_AdobeGroup();
		pGroup->Setup( m_CurrentLevel, name );
		pGroup->AttachParent( m_pCurrentGroup, m_pCurrentGroup->GetName() );

		m_pCurrentGroup->AddElement( pGroup );
		m_pCurrentGroup = pGroup;
		m_CurrentLevel++;
	}
	else
	{
		// Add group to the layer.
		C_AdobeGroup* pGroup = new C_AdobeGroup();
		pGroup->Setup( m_CurrentLevel, name );
		pGroup->AttachParent( m_pCurrentLayer, m_pCurrentLayer->GetName() );

		m_pCurrentLayer->AddElement( pGroup );
		m_pCurrentGroup = pGroup;
		m_CurrentLevel++;
	}
	m_NumGroups++;
}


void C_IpfData::CloseGroup()
{
	// Cant close a layer.
	if ( !m_pCurrentGroup )
		return;

	// If we are not at the top level.
	if ( m_pCurrentGroup->GetLevel() <= 1 )
	{
		m_pCurrentGroup = NULL;
	}
	else
	{
		// Back up 1 level.
		m_pCurrentGroup = const_cast<C_AdobeGroup*>( m_pCurrentGroup->GetParent() );
	}
}


bool C_IpfData::HasDataToWrite()
{
	if ( m_GradientList.Size() )
		return true;

	// Check the layers arnt empty.
	for ( S32 i = 0; i < m_LayerList.Size(); i++ )
	{
		if ( m_LayerList[ i ]->GetNumOfElements() )
			return true;
	}
	return false;
}


void C_IpfData::AddData( C_AdobePolygon* pPolygon )
{
	if ( !m_pCurrentLayer )
		return;

	if( m_pCurrentGroup )
	{
		// Add to opened group.
		m_pCurrentGroup->AddElement( pPolygon );
		pPolygon->Setup( m_CurrentLevel );
		pPolygon->AttachParent( m_pCurrentGroup, m_pCurrentGroup->GetName() );
	}
	else
	{
		// Add to layer.
		m_pCurrentLayer->AddElement( pPolygon );
		pPolygon->Setup( m_CurrentLevel );
		pPolygon->AttachParent( m_pCurrentLayer, m_pCurrentLayer->GetName() );
	}
	m_NumShapes++;
}


void C_IpfData::AddGradient( C_AdobeGradient* pGradient )
{
	assert( pGradient );

	m_GradientList.PushBack( pGradient );
}

}; // namespace Ipf
