
#include "Stdafx.h"
#include "ui/Illustrator/UIAdobeScene.h"

#include <system/misc/stringutil.h>

#include "ui/Illustrator/UIAdobeColour.h"
#include "ui/Illustrator/UIAdobeGroup.h"
#include "ui/Illustrator/UIAdobePath.h"
#include "ui/Illustrator/UIAdobeShape.h"
#include "ui/Graphics/Polygons/UIPolygonClasses.h"

#include "ui/UIDefines.h"
#include "ui/Graphics/Simple2D.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

#define DEFAULT_GROUP_CAPACITY 10

namespace UI
{


C_AdobeScene::C_AdobeScene()
:	m_GradientIndexOffset( m_GradientIndexOffset_Invalid ),
	m_pCurrentGroup( NULL ),
	m_pCurrentLayer( NULL ),
	m_TotalNumberOfShapes( 0 ),
	m_TotalNumberOfGroups( 0 )
{
}


C_AdobeScene::C_AdobeScene( const char* pName )
:	m_GradientIndexOffset( m_GradientIndexOffset_Invalid ),
	m_pCurrentGroup( NULL ),
	m_pCurrentLayer( NULL ),
	m_TotalNumberOfShapes( 0 ),
	m_TotalNumberOfGroups( 0 )
{
	strcpy_s( m_SceneName, m_MaxSceneNameLen, pName );
}

C_AdobeScene::~C_AdobeScene()
{
	RemoveLayers();
	RemoveGradientDictionary();
}


void C_AdobeScene::CreateGradientStrips( )
{
	for ( S32 gradient = 0; gradient < m_GradientDictionary.Size(); gradient++ )
	{
		const U8 gradientIndex = gSimple2D.GetGradient().PopIndex();
		if(	gradient == 0 )
			m_GradientIndexOffset = gradientIndex;

		gSimple2D.GetGradient().Build( gradientIndex, *m_GradientDictionary[ gradient ] );
	}
}

const U8 C_AdobeScene::GetGradientIndex( const U8 dictionaryIndex ) const
{ 
	//assert(m_GradientIndexOffset!=m_GradientIndexOffset_Invalid); 
	
	return m_GradientIndexOffset + dictionaryIndex; 
}


void C_AdobeScene::AddLayer( const char* name, S32 capacity )
{
	// Layers all have a level of 0, and no parent.
	C_AdobeGroup* pNewLayer = new C_AdobeGroup( );
	pNewLayer->Setup( 0, name );
	pNewLayer->SetCapacity( capacity );

	m_SceneLayerList.PushBack( pNewLayer );

	// Set as current group so all new shapes are added to this group!!
	m_pCurrentLayer = pNewLayer;
	m_pCurrentGroup = NULL;
}


void C_AdobeScene::AddGroup( S32 level, const char* name)
{
	// Add it to an opened group.
	if ( m_pCurrentGroup )
	{
		C_AdobeGroup* pNewGroup = new C_AdobeGroup();
		pNewGroup->Setup( level, name );
		pNewGroup->AttachParent( m_pCurrentGroup, m_pCurrentGroup->GetName() );
		pNewGroup->SetCapacity( DEFAULT_GROUP_CAPACITY );

		// Link the groups up.
		m_pCurrentGroup->AddElement( pNewGroup );
		m_pCurrentGroup = pNewGroup;
	}
	// Add it to the current layer.
	else
	{
		C_AdobeGroup* pNewGroup = new C_AdobeGroup();
		pNewGroup->Setup( level, name );
		pNewGroup->AttachParent( m_pCurrentLayer, m_pCurrentLayer->GetName() );
		pNewGroup->SetCapacity( DEFAULT_GROUP_CAPACITY );

		// Store the layer 
		m_pCurrentLayer->AddElement( pNewGroup );
		m_pCurrentGroup = pNewGroup;
	}
	m_TotalNumberOfGroups++;
}

void C_AdobeScene::CloseGroup()
{
	// Cant close a layer.
	if ( !m_pCurrentGroup )
		return;

	// If we are not at the top level. Layers are 0.
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

void C_AdobeScene::AddShape( C_AdobeShape* pShape )
{
	assert( pShape );
	assert( m_pCurrentLayer );

	if( m_pCurrentGroup )
	{
		m_pCurrentGroup->AddElement( pShape );
		pShape->AttachParent( m_pCurrentGroup, m_pCurrentGroup->GetName() );
	}
	else
	{
		m_pCurrentLayer->AddElement( pShape );
		pShape->AttachParent( m_pCurrentLayer, m_pCurrentLayer->GetName() );
	}

	m_SceneBounds.UpdateExtents( pShape->BoundingBox() );
	m_TotalNumberOfShapes++;
}


void C_AdobeScene::RemoveLayers()
{
	for ( S32 layer = 0; layer < m_SceneLayerList.Size(); layer++ )
	{
		SAFE_DELETE_PTR( m_SceneLayerList[layer] );
	}
	m_SceneLayerList.Clear();
	m_pCurrentLayer = NULL;
	m_pCurrentGroup = NULL;
}


void C_AdobeScene::RemoveGradientDictionary()
{
	for ( S32 gradient = 0; gradient < m_GradientDictionary.Size(); gradient++ )
	{
		SAFE_DELETE_PTR( m_GradientDictionary[ gradient ] );
	}
	m_GradientDictionary.Clear();
}


const bool C_AdobeScene::TraverseGroup( AdobeRenderList& renderList, Maths::C_AxisBox& sceneBounds, const C_AdobeGroup* pGroup, const char* name, const bool bUseGroupBounds ) const
{
	BIZ_ASSERT( pGroup );

	for ( S32 element = 0; element < pGroup->GetNumOfElements(); element++ )
	{
		const C_AdobeElement* pElement = pGroup->GetElementPtr( element );
		if ( !pElement )
			continue;

		// Traverse the group.
		if ( pElement->GetElementType() == Ipf::E_ElementType_Group )
		{
			C_AdobeGroup* pElementGroup = (C_AdobeGroup*)pElement;

			if ( pElementGroup->GetName() == name )
			{
				AddGroupChildren( renderList, sceneBounds, pElementGroup );
			}
			else
			if( TraverseGroup( renderList, sceneBounds, pElementGroup, name, bUseGroupBounds ) )
				return true;
		}
		else
		if ( pElement->GetElementType() == Ipf::E_ElementType_Shape )
		{
			C_AdobeShape* pElementShape = (C_AdobeShape*)pElement;

			if ( bUseGroupBounds )
				sceneBounds.UpdateExtents( pElementShape->BoundingBox() );

			if ( pElementShape->GetName() == name )
			{
				if ( !bUseGroupBounds )
					sceneBounds.UpdateExtents( pElementShape->BoundingBox() );

				renderList.PushBack( pElementShape );

				return true;
			}
		}
	}
	return false;
}


const bool C_AdobeScene::BuildRenderList( AdobeRenderList& vRenderList, Maths::C_AxisBox& vBoundingBox, const char* pName, const bool bUseGroupBounds ) const
{
	// Reset this.
	vRenderList.Clear();
	vBoundingBox = Maths::C_AxisBox();

	// Go through each layer and pull out the group.
	for ( S32 layer = 0; layer < m_SceneLayerList.Size(); layer++ )
	{
		const C_AdobeGroup* pLayer = m_SceneLayerList[ layer ];

		// Is this layer what we want to pull out?
		if ( !strlen( pName ) || pLayer->GetName() == pName )
		{
			AddGroupChildren( vRenderList, vBoundingBox, pLayer );
		}
		else
		if ( TraverseGroup( vRenderList, vBoundingBox, pLayer, pName, bUseGroupBounds ) )
		{
			return true;
		}
	}
	return ( vRenderList.Size() ) ? true : false;
}


void C_AdobeScene::AddGroupChildren( AdobeRenderList& renderList, Maths::C_AxisBox& sceneBounds, const C_AdobeGroup* pGroup ) const
{
	if ( !pGroup )
	{
		assert( pGroup );
		return;
	}

	for ( S32 element = 0; element < pGroup->GetNumOfElements(); element++ )
	{
		const C_AdobeElement* pElement = pGroup->GetElementPtr( element );
		if ( !pElement )
			continue;

		// Traverse the group.
		if ( pElement->GetElementType() == Ipf::E_ElementType_Group )
		{
			C_AdobeGroup* pElementGroup = (C_AdobeGroup*)pElement;

			AddGroupChildren( renderList, sceneBounds, pElementGroup );
		}
		else
		if ( pElement->GetElementType() == Ipf::E_ElementType_Shape )
		{
			C_AdobeShape* pElementShape = (C_AdobeShape*)pElement;
			
			renderList.PushBack( pElementShape );

			// Calculate new max and min based on this shape size.
			sceneBounds.UpdateExtents( pElementShape->BoundingBox() );
		}
	}
}


const C_AdobeGroup* C_AdobeScene::GetLayer( S32 index ) const
{
	if ( index < 0 || index >= m_SceneLayerList.Size() )
		return NULL;
	else
		return m_SceneLayerList[index]; 
}


} // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on) 
#endif
