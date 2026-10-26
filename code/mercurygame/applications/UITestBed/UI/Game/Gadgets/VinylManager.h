#ifndef UI_VINYLMANAGER_H
#define UI_VINYLMANAGER_H

#include <ui/Illustrator/UIAdobeShape.h>
#include <ui/Illustrator/UIAdobeSceneManager.h>

#include <ui/Graphics/Simple2D.h>
#include <ui/Graphics/UIColourClasses.h>

#include "VinylList.h"

namespace UI
{

class UIColourPicker;

class C_List
{
public:
	C_List( ) :
		m_ClipX( -1.0f ),
		m_ClipY( -1.0f ),
		m_ClipW( -1.0f ),
		m_ClipH( -1.0f ),
		m_MoveScale( 1.0f )
	{
	}

	VinylList						m_scList;
	int								m_scCurrent;
	Btl::Vector< Maths::HMat43 >	m_MatrixCache;

	void							CacheMatrices( );

	F32								m_ClipX;
	F32								m_ClipY;
	F32								m_ClipW;
	F32								m_ClipH;

	F32								m_DrawCentreX;
	F32								m_DrawCentreY;

	F32								m_MoveScale;

private:
	void							CacheMatrices( const int index, Maths::HMat43& mat );
};

class C_VinylManager
{
public:
	// Enums
	enum E_EditMode
	{
		E_EditMode_None,
		E_EditMode_Translate,
		E_EditMode_Scale,
		E_EditMode_Rotate,
		E_EditMode_Shear,
		E_EditMode_GradientRotate,
		E_EditMode_GradientTranslate,
		E_EditMode_GradientScale,
	};

	enum E_ShouldRender
	{
		E_ShouldRender_None = 0,
		E_ShouldRender_Avatar,
		E_ShouldRender_Render,
		E_ShouldRender_Clipboard,
		E_ShouldRender_ScratchPad,
	};

	enum E_List
	{
		E_List_Clipboard = 0,			// For cut and paste
		E_List_ScratchPad,				// For defrag and stamp
		E_List_Avatar,					// Avatar creator render buffer
		E_List_Group,					// Group icon creator render buffer
		E_List_LiveryTop,				// Livery parts render buffers
		E_List_LiveryLeft,
		E_List_LiveryRight,
		E_List_LiveryFront,
		E_List_LiveryBack,

		E_List_Count
	};

	enum E_SceneType
	{
		E_SceneType_Avatar = 0,			// Player avatar
		E_SceneType_Group,				// Custom group icon
		E_SceneType_Livery,				// Car livery
	};

public:
	C_VinylManager( );
	~C_VinylManager( );

public:
	static C_VinylManager&				Instance()	{ return m_sInstance; }
	const bool							Init( const U32 totalItems );
	void								End( );
										
	void								Copy( );
	void								Cut( );
	void								Stamp( );
	void								Paste( const bool allowEmpty = false );

	void								FlashShape( );
								
	void								Rotate( const F32 rotationInc, const bool nudge );
	void								Translate( const bool moveX, const F32 amount, const bool nudge );
	void								Scale( const bool scaleX, const F32 amount, const bool nudge );
	void								Shear( const bool shearX, const F32 amount, const bool nudge );
	void								Alpha( const F32 alphaInc, const bool nudge );
	void								ColourRotate( const F32 rotationInc, const bool nudge );
	void								ColourTranslate( const F32 rotationInc, const bool nudge );
	void								ColourScale( const F32 rotationInc, const bool nudge );

	void								InsertNewItem( );

	void								SelectedToRoot();
	void								SelectedResetItem( );
	void								SelectedShapeIndex( const U16 index );
	const U16							SelectedShapeIndex( );
	const C_AdobeShape*					SelectedShape( );

	void								SelectedUseGradient( const bool useGradient );
	const bool							SelectedUseGradient();

	void								SelectedColourIndex( const U32 edge_index, U16 index );
	const U16							SelectedColourIndex( const U32 edge_index );
								
	void								SelectedAlpha( const U32 edge_index, const F32 alpha );
	const F32							SelectedAlpha( const U32 edge_index );

	void								SelectedEnterGroup( );
	void								SelectedExitGroup( );
	void								SelectedExplodeGroup( );
								
	const bool							HighlightSelected( );

	const S32							SetSelection( const S32 dir );

	void								SelectedHighlightToggle( );
	const bool							SelectedHightlight( );

	void								EditMode( const E_EditMode editMode );
	const E_EditMode					EditMode( ) const;
	
	void								SceneType( const E_SceneType t );
	const E_SceneType					SceneType() const;

	void								CurrentList( const E_List l )			{ m_CurrentList = l; }
	const E_List						CurrentList() const						{ return m_CurrentList; }
	
	// Render list
	C_List&								List()									{ return m_List[ m_CurrentList ]; }
	C_List&								List( const E_List list )				{ return m_List[ list ]; }

	// Vinyl lists
	VinylList&							scList()								{ return m_List[ m_CurrentList		].m_scList;		}
	VinylList&							scList( const E_List list )				{ return m_List[ list				].m_scList;		}

	VinylList&							scClipboardList()						{ return m_List[ E_List_Clipboard	].m_scList;		}
	VinylList&							scScratchPadList()						{ return m_List[ E_List_ScratchPad	].m_scList;		}
	VinylList&							scAvatarList()							{ return m_List[ E_List_Avatar		].m_scList;		}
	VinylList&							scGroupList()							{ return m_List[ E_List_Group		].m_scList;		}
																	
	int&								scCurrent( const E_List list )			{ return m_List[ list				].m_scCurrent;	}
	int&								scCurrent()								{ return m_List[ m_CurrentList		].m_scCurrent;	}

	Maths::HVec2&						CoordRange()							{ return m_CoordRange; }
	void								CoordRange( const Maths::HVec2& cr )	{ m_CoordRange = cr; CalcTransform2D(); }
	Maths::HMat44&						Transform2D()							{ return m_Transform2D; }

	// Render now!
	void								ShouldRender( const E_ShouldRender r )	{ m_ShouldRender = r; }
	const bool							ShouldRender( )							{ return m_ShouldRender != E_ShouldRender_None; }
	void								Render();

	const C_AdobeShape*					ResolveShape( U16 index );
	C_UIColour							ResolveColour( U16 index, F32 alpha );

	C_AdobeScene::AdobeRenderList&		ShapeRenderList()	{ return m_ShapeRenderList; }

	void								ClearScene();

	const bool							LoadAvatar( Sys::BinaryDeserialiserAutoEndian& serial );
	const bool							SaveAvatar( Sys::BinarySerialiserAutoEndian& serial );

	const bool							Save();
	const bool							Load();

	const bool							ReadBuffer( U8* pBuffer, const U32 uDataSize );
	const bool							WriteBuffer( U8* pBuffer, const U32 uDataSize );

	void								RenderList( C_List& list, const int index, const Maths::HVec3& shapeSize, const bool isScene, const C_UIColour colourOveride, const bool doOveride );
	
	void								RenderList( const C_Sprite* pSprite );
	void								RenderShape( const C_Sprite* pSprite );

	void								AssignStripMatrix( const int index, const Maths::HMat43& mat )	{ m_StripMatrixCache[ index ] = mat; }

private:
	const bool							LoadImpl( Sys::BinaryDeserialiserAutoEndian& serial);
	const bool							SaveImpl( Sys::BinarySerialiserAutoEndian& serial );

	void								CalcTransform2D();

	// Render
	Maths::HMat43						GetBoundsMatrix( const C_AdobeShape* pShape, const Maths::HVec3& shapeSize );
	void								RenderShape( const C_AdobeShape* pShape );
	void								RenderFlatShape( const C_AdobeShape* pShape, const Maths::HMat43& m0, const Maths::HVec3& shapeSize, const C_UIColour& fillColour );
	void								RenderGradientShape( const C_AdobeShape* pShape, const Maths::HMat43& m0, const Maths::HMat43& col_mat, const Maths::HVec3& shapeSize, const C_UIColour& fillColour0, const C_UIColour& fillColour1 );

private:
	// Static
	static C_VinylManager				m_sInstance;

	// Const
	static const U32 m_FileVersion = 2;

	// vars
	E_EditMode							m_EditMode;
	E_ShouldRender						m_ShouldRender;
	bool								m_bActive;

	Btl::Vector< C_List, Btl::VectorParameters_Array< E_List_Count > >		m_List;
//	Btl::Vector< VinylList, Btl::VectorParameters_Array< E_List_Count > >	m_scList;
//	Btl::Vector< int, Btl::VectorParameters_Array< E_List_Count > >    		m_scCurrent;
	E_List								m_CurrentList;

	E_SceneType							m_SceneType;

//	Btl::Vector< Maths::HMat43 >		m_SceneMatrixCache;
	Btl::Vector< Maths::HMat43 >		m_StripMatrixCache;
	Maths::HVec2						m_CoordRange;
	Maths::HMat44						m_Transform2D;

	// Scene shape data stuff
	C_RenderState						m_RenderState;
	LayerIndex							m_Panel;
	PanelIndex							m_Layer;
	XtSz8								m_SceneName;
	const C_AdobeHandle*				m_pSceneHandle;
	C_AdobeScene::AdobeRenderList		m_ShapeRenderList;
};

const bool CalcTransform( const C_AdobeShape* pShape, const Maths::HVec3& drawSize, const Maths::HMat43& drawMat, const F32 up, TransformIndex& ti, Maths::HVec3& out, F32& sizeX, F32& sizeY );
void GetTransformHeirachy( VinylList* list, int current, Maths::HMat43& mat );

}; // namespace UI

#endif // UI_VINYLMANAGER_H
