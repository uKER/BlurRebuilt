
#ifndef UI_UIADOBESCENE_H
#define UI_UIADOBESCENE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/Containers/containers.h>

#define ILLUSTRATOR_SCALE 1

namespace UI
{

// Forward declarations
class C_AdobeGroup;
class C_AdobeShape;
class C_AdobeGradient;

/// @addtogroup Illustrator
/// @{

//-------------------------------------------------------------------
// Class : C_AdobeScene
//-------------------------------------------------------------------
/// Container class that contains all of the elements and gradients that make up a scene.
class C_AdobeScene
{
public:
	/// List to store all of the layers in the scene.
	typedef Btl::Vector< const C_AdobeGroup* >		AdobeGroupList;

	/// List of all the gradients availible in the scene.
	typedef Btl::Vector< const C_AdobeGradient* >	AdobeGradientDictionary;

	/// List to store all of the shapes that are ready for rendering.
	typedef Btl::Vector< const C_AdobeShape* >		AdobeRenderList;

public:
	/// @a
	C_AdobeScene();

	/// Construct by value.
	/// @param name - name of the scene.
	C_AdobeScene( const char* pName );

	/// @a
	~C_AdobeScene();

									/// Adds a new layer to the scene. All new elements are added to this layer.
	void							AddLayer( const char* name, S32 capacity );

									/// Will add a group at the current mount point be it a layer or a group.
	void							AddGroup( S32 level, const char* name );

									/// Close the current group and move up the hierachy 1 level.
	void							CloseGroup();

									/// @a
	void							AddShape( C_AdobeShape* pShape );

									/// @a
	void							CreateGradientStrips( );

									/// @a
	const U8						GetGradientIndex( const U8 dictionaryIndex ) const;

									/// @a
	const bool						BuildRenderList( AdobeRenderList& vRenderList, Maths::C_AxisBox& vBoundingBox, const char* pName = "", const bool bUseGroupBounds = false ) const;

									/// @a
	const S32						GetNumOfLayers() const						{ return m_SceneLayerList.Size(); }

	const C_AdobeGroup*				GetLayer( S32 index ) const;

									/// @a
	const char*						Name() const								{ return m_SceneName; }

									/// @a
	AdobeGradientDictionary&		GetGradientDictionary( )					{ return m_GradientDictionary; }
	
									/// @a
	const C_AdobeGradient&			GetGradient( const U8 dictionaryIndex )		{ return *m_GradientDictionary[ dictionaryIndex ]; }

	Maths::C_AxisBox				GetSceneBounds()							{ return m_SceneBounds; }

	const U32						GetTotalNumberOfShapes() const				{ return m_TotalNumberOfShapes; }

	const U32						GetTotalNumberOfGroups() const				{ return m_TotalNumberOfGroups; }

private:
									/// Remove and delete all of the data in each layer.
	void							RemoveLayers();

									/// Remove and delete all of the data in each gradient.
	void							RemoveGradientDictionary();

	const bool						TraverseGroup(  AdobeRenderList& renderList, Maths::C_AxisBox& sceneBounds, const C_AdobeGroup* pGroup, const char* name, const bool bUseGroupBounds = false ) const;

	void							AddGroupChildren( AdobeRenderList& renderList, Maths::C_AxisBox& sceneBounds, const C_AdobeGroup* pGroup ) const;

private:
	static const U32				m_MaxSceneNameLen = 64;					///< @a
	char							m_SceneName[m_MaxSceneNameLen];			///< Name of this scene.

	static const U8					m_GradientIndexOffset_Invalid = 0xff;	///< @a
	U8								m_GradientIndexOffset;					///< @a
	AdobeGradientDictionary			m_GradientDictionary;					///< Scene gradient dictionary

	C_AdobeGroup*					m_pCurrentGroup;						///< Current group. Last one added.
	C_AdobeGroup*					m_pCurrentLayer;						///< Current layer. Last one added.
	AdobeGroupList					m_SceneLayerList;						///< List containing all the layers in the scene.

	Maths::C_AxisBox				m_SceneBounds;							///< Bounds of the entire scene.
	
	U32								m_TotalNumberOfShapes;					///< Total Number of shapes in the scene.
	U32								m_TotalNumberOfGroups;					///< Total Number of groups in the scene.
};

///@}

} // namespace UI

#endif
