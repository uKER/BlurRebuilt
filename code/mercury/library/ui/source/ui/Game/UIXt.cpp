#include "stdafx.h"

#include "ui/UiInterface.h"

#include "ui/Game/UILocaliser.h"
#include "ui/Game/UIXt.h"
#include "ui/Game/UIMaths.h"
#include "ui/Game/Timeline.h"
#include "ui/Game/Gadgets/ButtonMessages.h"
#include "ui/Game/Gadgets/ButtonController.h"

#include "ui/Game/Gadgets/BaseUIList.h"
#include "ui/Game/Gadgets/BaseUIVariableSelect.h"
#include "ui/Game/Gadgets/BaseUIShape.h"
#include "ui/Game/Gadgets/Buttons.h"
#include "ui/Game/Gadgets/UIStaticGraphic.h"
#include "ui/Game/Gadgets/UIStaticLetter.h"
#include "ui/Game/Gadgets/UIStaticText.h"
#include "ui/Game/Gadgets/UIStaticScene.h"
#include "ui/Game/Gadgets/UIStaticShape.h"
#include "ui/Game/Gadgets/UIPath.h"
#include "ui/Game/Gadgets/UIVerticalMenu.h"
#include "ui/Game/Gadgets/UIVariableSpinner.h"
#include "ui/Game/Gadgets/UIBox.h"
#include "ui/Game/Gadgets/UICamera.h"
#include "ui/Game/Gadgets/UICredits.h"
#include "ui/Game/Gadgets/UILegal.h"
#include "ui/Game/Gadgets/UIHorizontalMenu.h"
#include "ui/Game/Gadgets/UISlider.h"
#include "ui/Game/Gadgets/UIParticleEmitter.h"
#include "ui/Game/Gadgets/UIItemSpinner.h"
#include "ui/Game/Gadgets/UIScrollBar.h"
#include "ui/Game/Gadgets/UIFriendsList.h"
#include "ui/Game/Gadgets/UIAdobeBox.h"
#include "ui/Game/Gadgets/UIAdobeTrail.h"
#include "ui/Game/Gadgets/UIAdobeAnimation.h"
#include "ui/Game/Gadgets/UIKeyboard.h"
#include "ui/Game/Gadgets/UINodeList.h"
#include "ui/Game/Gadgets/UIGraph.h"
#include "ui/Game/Gadgets/UIProgressBar.h"
#include "ui/Game/Gadgets/UITable.h"

#include "ui/Graphics/UISpriteTextureParam.h"
#include "ui/Graphics/Font/Font.h"
#include "ui/Graphics/Font/FontStyle.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

// Sorcery definitions
XT_TYPE( UI::C_UIXtHermiteKeyFrame, "UIHermiteKeyFrame" )
	XT_FIELD( m_fValue, "value" )
	XT_FIELD( m_fTangentIn, "inTangent" )
	XT_FIELD( m_fTangentOut, "outTangent" )
	XT_FIELD( m_fTime, "time" )
XT_END_TYPE

XT_TYPE( UI::C_UIXtHermiteCurve, "UIHermiteCurve" )
	XT_FIELD( m_Name, "Name" )
	XT_FIELD( m_KeyFrames, "Curve" )
XT_END_TYPE

XT_TYPE( UI::Padding, "UIPadding" )
	XT_FIELD( top,		"top" )
	XT_FIELD( bottom,	"bottom" )
	XT_FIELD( left,		"left" )
	XT_FIELD( right,	"right" )
XT_END_TYPE

XT_TYPE( UI::E_Justify, "UIJustify" )
	XT_OPTION( UI::E_Justify_TopLeft, "TopLeft" )
	XT_OPTION( UI::E_Justify_TopCentre, "TopCentre" )
	XT_OPTION( UI::E_Justify_TopRight, "TopRight" )
	XT_OPTION( UI::E_Justify_MiddleLeft, "MiddleLeft" )
	XT_OPTION( UI::E_Justify_MiddleCentre, "MiddleCentre" )
	XT_OPTION( UI::E_Justify_MiddleRight, "MiddleRight" )
	XT_OPTION( UI::E_Justify_BottomLeft, "BottomLeft" )
	XT_OPTION( UI::E_Justify_BottomCentre, "BottomCentre" )
	XT_OPTION( UI::E_Justify_BottomRight, "BottomRight" )
	XT_OPTION( UI::E_Justify_None, "None" )
XT_END_TYPE

XT_TYPE( UI::UiXt::C_ColourStyle, "UISpriteColours" )
	XT_CALLBACK( UI::UiXt::C_ColourStyle::SorceryCallback )
	XT_FIELD( m_Name, "name" )
	XT_FIELD( m_TopLeft, "top_left" )
	XT_FIELD( m_TopRight, "top_right" )
	XT_FIELD( m_BottomLeft, "bottom_left" )
	XT_FIELD( m_BottomRight, "bottom_right" )
XT_END_TYPE

XT_TYPE( UI::UiXt::C_Screen, "UIScreen" )
	XT_CALLBACK( UI::UiXt::C_Screen::SorceryCallback )
	XT_FIELD( m_Name, "Name" )
	XT_FIELD( m_FontStyles, "FontStyles" )
	XT_FIELD( m_ColourStyles, "ColourStyles" )
	XT_FIELD( m_Elements, "Elements" )
	XT_FIELD( m_OnNextCmd, "OnNext" )
	XT_FIELD( m_OnBackCmd, "OnBack" )
	XT_FIELD( m_GuiTableName, "GuiTableName" )
XT_END_TYPE

XT_TYPE( UI::E_ScrollBarType, "UIScrollBarType" )
	XT_OPTION( UI::E_ScrollBarType_Vertical, "Vertical" )
	XT_OPTION( UI::E_ScrollBarType_Horizontal, "Horizontal" )
XT_END_TYPE

XT_LIBRARY( UI )
	XT_REGISTER_TYPE( UI::C_UIXtHermiteKeyFrame )
	XT_REGISTER_TYPE( UI::C_UIXtHermiteCurve )
	XT_REGISTER_TYPE( UI::Padding )
	// Gadget containers 
	XT_REGISTER_TYPE( UI::C_GadgetBaseData::E_Flag )
	XT_REGISTER_TYPE( UI::C_GadgetBaseData::C_VectorBase )
	XT_REGISTER_TYPE( UI::C_GadgetBaseData::C_Vector3Position )
	XT_REGISTER_TYPE( UI::C_GadgetBaseData::C_Vector3Direction )
	XT_REGISTER_TYPE( UI::C_GadgetBaseData::C_Vector3Rotation )
	XT_REGISTER_TYPE( UI::C_GadgetBaseData::C_Vector3Scale )
	XT_REGISTER_TYPE( UI::C_GadgetBaseData::C_Vector3Size )
	XT_REGISTER_TYPE( UI::C_TextureParam::C_Vector2UV )
	XT_REGISTER_TYPE( UI::C_TextureParam )
	// Gadget stuff
#ifdef PLATFORM_PC
	XT_REGISTER_TYPE( UI::C_GadgetBaseData::C_MouseData )
#endif
	// Gadget components
	XT_REGISTER_TYPE( UI::C_GadgetBaseData )
	XT_REGISTER_TYPE( UI::C_GadgetBase )
	XT_REGISTER_TYPE( UI::BaseUIList )
	XT_REGISTER_TYPE( UI::BaseUIVariableSelect )
	XT_REGISTER_TYPE( UI::BaseUIVariableSelect::C_Item )
	XT_REGISTER_TYPE( UI::BaseUIShape )
	// Gadget types
	XT_REGISTER_TYPE( UI::UISafeArea )
	XT_REGISTER_TYPE( UI::UIRenderBranch )
	XT_REGISTER_TYPE( UI::UIDummyLerp );
	XT_REGISTER_TYPE( UI::UIRenderState )
	XT_REGISTER_TYPE( UI::UIRenderState::E_AlterFlags )
	XT_REGISTER_TYPE( UI::UIStaticLetter )
	XT_REGISTER_TYPE( UI::UIStaticText )
	XT_REGISTER_TYPE( UI::UIStaticGraphic )
	XT_REGISTER_TYPE( UI::UIStaticScene )
	XT_REGISTER_TYPE( UI::UIStaticShape )
	XT_REGISTER_TYPE( UI::UIPath )
	XT_REGISTER_TYPE( UI::UIStaticBox )
	XT_REGISTER_TYPE( UI::UICamera )
	XT_REGISTER_TYPE( UI::UICredits )
	XT_REGISTER_TYPE( UI::UILegal )
	XT_REGISTER_TYPE( UI::UISlider )
	XT_REGISTER_TYPE( UI::UIItemSpinner )
	XT_REGISTER_TYPE( UI::UIVerticalMenu )
	XT_REGISTER_TYPE( UI::UIVariableSpinner )
	XT_REGISTER_TYPE( UI::UIHorizontalMenu )
	XT_REGISTER_TYPE( UI::UIParticleEmitter )
	XT_REGISTER_TYPE( UI::UIParticleEmitter::E_CycleType )
	XT_REGISTER_TYPE( UI::C_ScrollBarStyle )
	XT_REGISTER_TYPE( UI::E_ScrollBarType )
	XT_REGISTER_TYPE( UI::UIScrollBar )
	XT_REGISTER_TYPE( UI::UIFriendsList )
	XT_REGISTER_TYPE( UI::UIAdobeBox )
	XT_REGISTER_TYPE( UI::UIAdobeTrail )
	XT_REGISTER_TYPE( UI::UIAdobeAnimation )
	XT_REGISTER_TYPE( UI::UIKeyboard )
	XT_REGISTER_TYPE( UI::UINodeList )
	XT_REGISTER_TYPE( UI::UINode )
	XT_REGISTER_TYPE( UI::UINodeList::E_ListStyle )
	XT_REGISTER_TYPE( UI::UINodeList::E_RadialRotation )
	XT_REGISTER_TYPE( UI::UIGraph )
	XT_REGISTER_TYPE( UI::UIProgressBar )
	XT_REGISTER_TYPE( UI::UITable )
	
	// Timeline
	XT_REGISTER_TYPE( UI::C_Timeline )
	XT_REGISTER_TYPE( UI::C_Timeline::E_Type )
	XT_REGISTER_TYPE( UI::C_Timeline::E_RestType )
	XT_REGISTER_TYPE( UI::C_Timeline::C_TimeRange )
	XT_REGISTER_TYPE( UI::C_Timeline::C_Effect )
	XT_REGISTER_TYPE( UI::C_Timeline::C_Event )
	XT_REGISTER_TYPE( UI::C_Timeline::C_Effect::I_Range )
	XT_REGISTER_TYPE( UI::C_Timeline::C_Effect::C_Range< XtF32 > )
	XT_REGISTER_TYPE( UI::C_Timeline::C_Effect::C_Array< XtF32 > )
	XT_REGISTER_TYPE( UI::C_Timeline::C_Effect::C_Array< XtSz8 > )
	XT_REGISTER_TYPE( UI::C_Timeline::C_Effect::C_Array< XtLocId > )
	// System
	XT_REGISTER_TYPE( UI::UiXt::C_ColourStyle )
	XT_REGISTER_TYPE( UI::UiXt::C_Screen )
	// Font
	XT_REGISTER_TYPE( UI::E_XtFontShader )
	XT_REGISTER_TYPE( UI::C_FontStyle )
	XT_REGISTER_TYPE( UI::C_FontStylePasses )
	XT_REGISTER_TYPE( UI::C_FontStyleBaseRenderInfo )
	XT_REGISTER_TYPE( UI::C_FontStylePolygonOutline )
	XT_REGISTER_TYPE( UI::C_FontStylePolygon3D )
	XT_REGISTER_TYPE( UI::C_FontStylePolygonOutline3D )
	XT_REGISTER_TYPE( UI::C_FontStylePolygonGradient )
	XT_REGISTER_TYPE( UI::C_FontStylePolygonBevel3D )
	// Display
	XT_REGISTER_TYPE( Display::E_CullMode )
	XT_REGISTER_TYPE( Display::E_CmpFunc )
	XT_REGISTER_TYPE( Display::E_StencilOp )
	XT_REGISTER_TYPE( Display::E_Bool )
	XT_REGISTER_TYPE( Display::E_ColourWriteEnable )
	XT_REGISTER_TYPE( Display::C_StencilState )	
	// Misc
	XT_REGISTER_TYPE( UI::C_Simple2D::C_StackedRenderState )
	// Types
	XT_REGISTER_TYPE( UI::E_UiBlendMode )
	XT_REGISTER_TYPE( UI::E_Effect )
	XT_REGISTER_TYPE( UI::E_TextureAddressMode )
	XT_REGISTER_TYPE( UI::E_FixedSpaceType )
	XT_REGISTER_TYPE( UI::E_Justify )
	XT_REGISTER_TYPE( UI::UISlider::E_Type )
	XT_REGISTER_TYPE( UI::E_UiMsg )
	XT_REGISTER_TYPE( UI::E_CameraType )
	XT_REGISTER_TYPE( UI::E_UILanguage )
	XT_REGISTER_TYPE( UI::E_UIRegion )
	XT_REGISTER_TYPE( UI::E_StrokeEffect )
XT_END_LIBRARY

namespace UI
{

// global instance to the UI's Sorcery interface
C_UIXt g_UIXt;

// our record from Sorcery
// the UI's sorcery callback context
struct UIXtCallbackContext : public XtCallbackContext
{
	inline UIXtCallbackContext() : XtCallbackContext( 'UI' )	{  }
};

void UiPackOpenCallback(XtPackOpenStatus* status, void* context)
{
	LOGWARN( g_Log, "UiPackOpenCallback: %s", status->message);
}

// load in from Sorcery file
void C_UIXt::LoadDefinitions( const char* pFileName, XtDb* db )
{
	Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open( pFileName );

	UiXt::C_ScreenList::Instance().GetList().Clear();
	UiXt::C_ColourStyleList::Instance().GetList().Clear();
	UiXt::C_FontStyleList::Instance().GetList().Clear();

	if ( pFile )
	{
		U32 length = (U32)pFile->Length();
		U8* pData = new U8[ length ];

		if ( pData )
		{
			if ( pFile->Read( pData, length ) )
			{
				XtSchema* pXtSchema = XtGetApplicationSchema();

				if ( pXtSchema )
				{
					XtPackOpenParams oo;

					oo.callback = UiPackOpenCallback;
					oo.context = 0;

					if( db == NULL )
					{
						m_pXtCC	= new UIXtCallbackContext;
						m_pXtDb = XtDbCreate( pXtSchema, 100, m_pXtCC );	// 100 records max
						m_DbCreate = true;
					}
					else
					{
						m_pXtDb = db;
					}
					m_pXtPack = XtPackOpen( m_pXtDb, pData, length, &oo );

					if( db == NULL )
					{
						XtNetworkStart( m_pXtDb );
					}				
				}				
				// else Sorcery hasn't been initialised - eek!
			}

			delete [] pData;
		}

		delete pFile;
	}
}

// cleanup Sorcery
void C_UIXt::Cleanup()
{
	SAFE_DELETE_PTR( m_pXtCC );

	XtPackClose( m_pXtDb, m_pXtPack );

	if( m_DbCreate )
		XtDbDestroy( m_pXtDb );
}

namespace UiXt
{

// Singleton
C_ColourStyleList		C_ColourStyleList::m_sInstance;

static XtRecordHeader* UiXtGetRecordHeader(const XtRecord* record) // To be replaced later by Mr Breads 'actual' code
{
	return (XtRecordHeader*)((XtU8*)record->object - sizeof(XtRecordHeader));
}

static XtType* UiXtGetTypeByPtr(const XtRecord* record, const void* ptr) // To be replaced later by Mr Breads 'actual' code
{
	if (!ptr || !record || !record->object)
		return 0;

	XtRecordHeader*	header		= UiXtGetRecordHeader(record);
	XtComponent*	components	= (XtComponent*)header->top;

	for(size_t i = 0; i < header->count; i++)
	{
		XtComponent* component = &components[i];

		switch(component->behaviour)
		{
			case kXtCbObject:
				if ((ptr >= component->data) && (ptr < (component->data + (component->type->size * component->count))))
					return component->type;
			break;
		}
	}

	return 0;
}

static bool XtTypeReferencable( XtType* def )
{
	if( !def->event )
		return false;

	XtCallbackContext ctx( 'RRef' );

	def->event( kXtEtUser, NULL, NULL, &ctx );

	return (ctx.tag == 0);
}

static void CloneFromDefImpl( U8* dest, XtType* def, U8 *structPtr, C_Allocator& allocator, C_PointerMap& pointerMap )
{
	if(structPtr == NULL)
	{
		//LOGFATAL( Def::g_Log,  "Bad sorcery data %s\n", def.name);
		return;
	}

	for( XtU32 i = 0; i < def->baseCount; i++ )
	{
		CloneFromDefImpl( dest, def->bases[ i ].type, structPtr + def->bases[ i ].offset, allocator, pointerMap );
	}

	// Check if we can be referenced?
	if( XtTypeReferencable( def ) )
		pointerMap.AddBinding( structPtr, dest );

	for(XtU32 i = 0; i < def->fieldCount; i++)
	{
		XtField &field = def->fields[i];
		switch(field.behaviour)
		{
		case kXtFbPtr:
			if( field.array == 0 )
			{
				void**src_ptr = (void**)(structPtr + field.offset);
				void**dst_ptr = (void**)(dest + field.offset);

				*dst_ptr = *src_ptr;
				pointerMap.AddReference( *src_ptr );
				pointerMap.AddToRemapList( dst_ptr );
			}
			else
			{
				XtArray< void** >* src_array = (XtArray< void** >*)(structPtr + field.offset);
				XtArray< void** >* dst_array = (XtArray< void** >*)(dest + field.offset);

				dst_array->count = 0;
				dst_array->elements = NULL;

				LOGWARN( g_Log, DBSTR "Not yet implemented" );
			}
			break;

		case kXtFbHandle:
			if( field.array == 0 )
			{
				XtHandle< void * >* src_ptr = (XtHandle< void * >*)(structPtr + field.offset);
				XtHandle< void * >* dst_ptr = (XtHandle< void * >*)(dest + field.offset);

				*dst_ptr = *src_ptr;
			}
			else
			{
				LOGWARN( g_Log, DBSTR "Not yet implemented" );
			}
//			LOGWARN( g_Log, DBSTR "Cloning handles is not supported" );
			break;

		case kXtFbAtom:
			// Strings
			if(field.type->atom == kXtSz8)  
			{
				if( field.array == 0 )
				{
					// Single string
					char **in = (char **)(dest + field.offset);
					*in = NULL;

					const char *string = *(const char **)(structPtr + field.offset);
					if( string )
					{
						// Make a copy
						const size_t len = strlen( string );
						
						if( string && len > 0 )
						{
							*in = (char*)allocator.New( len + 1 );
							assert( *in );

							STRCPY_S( *in, len+1, string );
						}
					}
				}
				else
				{
					// Array of strings
					XtArray< const char * >* src_array = (XtArray< const char * >*)(structPtr + field.offset);
					XtArray< const char * >* dst_array = (XtArray< const char * >*)(dest + field.offset);

					if( src_array->count )
					{
						dst_array->elements = (const char * *)allocator.New( field.type->size * src_array->count );
						dst_array->count = src_array->count;

						for(int i = 0; i < src_array->count; i++)
						{
							dst_array->elements[ i ] = NULL;

							const char *string = src_array->elements[ i ];
							if(string)
							{
								// Make a copy
								const size_t len = strlen( string );
								char *p = NULL;
								if( string && len > 0 )
								{
									p = (char*)allocator.New( len+1 );
									STRCPY_S( p, len+1, string );
								}
								dst_array->elements[ i ] = p;
							}
						}
					}
					else
					{
						dst_array->elements = NULL;
						dst_array->count = 0;
					}
				}
			}
			else
			{
				// Other atoms
				if( field.array == 0 )
				{
					// Single atom
					memcpy( dest + field.offset, structPtr + field.offset, field.type->size );
				}
				else
				{
					// Array of atoms
					XtArray< U8 >* src_array = (XtArray< U8 >*)(structPtr + field.offset);
					XtArray< U8 >* dst_array = (XtArray< U8 >*)(dest + field.offset);
					if( src_array->count )
					{
						dst_array->elements = (U8 *)allocator.New( field.type, src_array->count );
						dst_array->count = src_array->count;

						memcpy( dst_array->elements, src_array->elements, field.type->size * src_array->count );
					}
					else
					{
						dst_array->elements = NULL;
						dst_array->count = 0;
					}
				}
			}
			break;

		case kXtFbStruct:
			if( field.array == 0 )
			{
				// Recurse single item
				field.type->construct( dest + field.offset );
				CloneFromDefImpl( dest + field.offset, field.type, (structPtr + field.offset), allocator, pointerMap );
			}
			else
			{
				// Array of items
				XtArray< U8 >* src_array = (XtArray< U8 >*)(structPtr + field.offset);
				XtArray< U8 >* dst_array = (XtArray< U8 >*)(dest + field.offset);
				if( src_array->count && src_array->elements )
				{
					dst_array->elements = (U8 *)allocator.New( field.type, src_array->count );
					dst_array->count = src_array->count;

					// Recurse each array element
					for( XtI32 i = 0; i < dst_array->count; i++ )
					{
						field.type->construct( dst_array->elements + (field.type->size * i) );
						CloneFromDefImpl( dst_array->elements + (field.type->size * i), field.type, src_array->elements + (field.type->size * i), allocator, pointerMap );
					}
				}
				else
				{
					dst_array->elements = NULL;
					dst_array->count = 0;
				}
			}
			break;

		case kXtFbLabel:
		case kXtFbEnum:
		case kXtFbFlags:
			if( field.array == 0 )
			{
				// Copy single item
				memcpy( dest + field.offset, structPtr + field.offset, field.type->size );
			}
			else
			{
				// Array of atoms
				XtArray< U8 >* src_array = (XtArray< U8 >*)(structPtr + field.offset);
				XtArray< U8 >* dst_array = (XtArray< U8 >*)(dest + field.offset);
				if( src_array->count )
				{
					dst_array->elements = (U8 *)allocator.New( field.type->size * src_array->count );
					dst_array->count = src_array->count;

					memcpy( dst_array->elements, src_array->elements, field.type->size * src_array->count );
				}
				else
				{
					dst_array->elements = NULL;
					dst_array->count = 0;
				}
			}

			break;
		}
	}
}
// sorcery callback
void C_ColourStyle::SorceryCallback( XtEventType type, XtRecord* pRecord, C_ColourStyle* pColourStyle, XtCallbackContext* )
{
	switch ( type )
	{
	case kXtEtConstruct:
		C_ScreenList::Instance().SetDirty( C_ScreenList::E_DirtyFlags_Colour ); // Dirty screen list so we get the update

		C_ColourStyleList::Instance().SetDirty();
		C_ColourStyleList::Instance().GetList().Insert( pColourStyle );
		break;

	case kXtEtDestruct:
		C_ColourStyleList::Instance().SetDirty();
		C_ColourStyleList::Instance().GetList().Erase( pColourStyle );
		break;
	}
}

const C_ColourStyle *C_ColourStyleList::FindByName( const char *pName )
{
	if( pName == NULL || pName[ 0 ] == NULL )
		return NULL;

	if(m_List.Size() == 0)
		return NULL;

	for(List::Iterator it = m_List.Begin(); it != m_List.End(); ++it)
	{
		const C_ColourStyle* pStyle = *(it);
		if( pStyle->m_Name && stricmp( pStyle->m_Name, pName ) == 0 )
			return pStyle;
	}
	return NULL;
}

void C_ColourStyleList::FrameUpdate( lua_State* pState )
{
	if(m_bIsDirty == false)
		return;
#if 0
	lua_newtable( pState );

	// Build new colour table
	U32 cnt = 1;
	for(List::Iterator it = m_List.Begin(); it != m_List.End(); ++it)
	{
		C_ColourStyle* pStyle = *(it);

		if( pStyle->m_Name && strlen( pStyle->m_Name  ) > 0 )
		{
			lua_pushstring( pState, pStyle->m_Name  );
		}
		else
		{
			char buffer[16];
			sprintf_s( buffer, 16, "unnamed_%02d", cnt );
			lua_pushstring( pState, buffer );

			cnt++;
		}

		UiXt::C_PointerMap pointerMap;
		UiXt::MakeLuaFromDef( pState, *pStyle, pointerMap, NULL );

		lua_rawset(pState,-3);
	}

	lua_setglobal (pState, "COLS" );
#endif
	m_bIsDirty = false; 
}

// Font style
C_FontStyleList C_FontStyleList::m_sInstance;

void C_FontStyleList::FlushBind( )
{
	if(!m_DefferedBindList.Size())
		return;

	for( S32 i = 0; i < m_DefferedBindList.Size(); i++ )
		m_DefferedBindList[ i ]->Bind();

	m_DefferedBindList.Clear();
	m_bIsDirty = false;
}

void C_FontStyleList::ClearBind()
{
	m_DefferedBindList.Clear();
}

const C_FontStyle *C_FontStyleList::FindByName( const char *pName )
{
	if( pName == NULL || pName[ 0 ] == NULL )
		return NULL;

	if(m_List.Size() == 0)
		return NULL;

	for(List::Iterator it = m_List.Begin(); it != m_List.End(); ++it)
	{
		const C_FontStyle* pStyle = *(it);
		if( pStyle->m_Name && stricmp( pStyle->m_Name, pName ) == 0 )
			return pStyle;
	}
	return NULL;
}

void C_FontStyleList::LoadPolygonFonts( )
{
	const S32 array_size = 64;
	Btl::Vector< Util::C_String256, Btl::VectorParameters_Array< array_size > > filenames;
	Btl::Vector< bool, Btl::VectorParameters_Array< array_size > > is_2d;
	Btl::Vector< bool, Btl::VectorParameters_Array< array_size > > is_outline;
	Btl::Vector< bool, Btl::VectorParameters_Array< array_size > > is_3d;
	Btl::Vector< bool, Btl::VectorParameters_Array< array_size > > is_bevel;
	Btl::Vector< bool, Btl::VectorParameters_Array< array_size > > is_finned;

	S32 count = 0;
	for(List::Iterator it = m_List.Begin(); it != m_List.End(); ++it)
	{
		const C_FontStyle* pStyle = *(it);
		if( pStyle->m_FontName )
		{
			// Search for name (-1) and render pass names (>-1)
			for( S32 k = -1; k < pStyle->m_RenderPassInfo.count; k++ )
			{
				const char *p = NULL;
				if( k >= 0 )
				{
					// No valid render pass
					if( !pStyle->m_RenderPassInfo[ k ] )
						continue;

					// Get render pass name,
					p = pStyle->m_RenderPassInfo[ k ]->m_FontOverideName;
				}
				else
				{
					// Style name
					p = pStyle->m_FontName;
				}

				// Invalid?
				if( !p || !p[0] )
					continue;

				// Already exists? (possibly as bitmap or app loaded polyfont)
				S32 id = gFont.FindFontByName( p, false );
				if( id != -1 )
				{
					//LOGWARN( g_Log, DBSTR " : Font %s already exists", p );
					continue;
				}

				bool do_3d = false;
				bool do_2d = false;
				bool do_outline = false;
				bool do_bevel = false;
				bool do_finned = false;

				Util::C_String filename( p );
				filename = filename.Lower();

				// Finned?
				S32 token = filename.Search( "_finned" );
				if( token != -1 )
				{
					do_finned = true;
					// Truncate
					filename[ token ] = NULL;
				}

				// 3d?
				token = filename.Search( "_3d" );
				if( token != -1 )
				{
					do_3d = true;
					// Truncate
					filename[ token ] = NULL;
				}
				else if ( !do_finned )
				{
					do_2d = true;
				}

				// Outline?
				token = filename.Search( "_outline" );
				if( token != -1 )
				{
					do_outline = true;
					// Truncate
					filename[ token ] = NULL;
				}

				// Bevel?
				token = filename.Search( "_bevel" );
				if( token != -1 )
				{
					do_bevel = true;
					// Truncate
					filename[ token ] = NULL;
				}

				// See if we already exist?
				bool found = false;
				for( S32 j = 0; j < count; j++)
					if( filenames[ j ] == filename )
					{
						if( do_2d )
							is_2d[ j ] = true;
						if( do_3d )
							is_3d[ j ] = true;
						if( do_outline )
							is_outline[ j ] = true;
						if( do_bevel )
							is_bevel[ j ] = true;
						if( do_finned )
							is_finned[ j ] = true;

						found = true;
						break;
					}
				// Add new
				if( !found )
				{
					assert( count < array_size );
					filenames[ count ] = filename;
					is_2d[ count ] = do_2d;
					is_3d[ count ] = do_3d;
					is_outline[ count ] = do_outline;
					is_bevel[ count ] = do_bevel;
					is_finned[ count ] = do_finned;

					count++;
				}
			}
		}
	}

	const Util::C_String256 dir( "Ui\\Data\\pf\\" );
	const Util::C_String256 ext( ".pf" );
	Sys::I_FileSystem *fs = Sys::GetDefaultFileSystem();
	for( S32 i = 0; i < count; i++ )
	{
		Util::C_String256 filename;

		filename += dir;
		filename += filenames[ i ];
		filename += ext;

		if (!fs->FileExists(filename.Chars()))
		{
			LOGDEBUG( g_Log, DBSTR " : Cannot find font file %s", filename.Chars() );
			continue;
		}

		gFont.CreatePolygonFont( filename.Chars(), 
			is_2d[ i ], is_2d[ i ] && is_outline[ i ], 
			is_3d[ i ], is_3d[ i ] && is_outline[ i ], 
			is_bevel[ i ], is_finned[ i ] );
	}

	// Attempt to Bind the fonts now we've loaded them
	for(List::Iterator it = m_List.Begin(); it != m_List.End(); ++it)
	{
		C_FontStyle* pStyle = *(it);
		pStyle->Bind();
	}
}

// Singleton
C_ScreenList			C_ScreenList::m_sInstance;

// Callback
void C_Screen::SorceryCallback( XtEventType type, XtRecord* pRecord, C_Screen* pUIScreen, XtCallbackContext* pContext )
{
	pUIScreen->m_Self = XtHandle< C_Screen >( pRecord );

	switch ( type )
	{
	case kXtEtConstruct:
		{
			// Changing our screen?
			if( C_ScreenList::Instance().IsDirty() == false && pUIScreen )
			{
				for(S32 i = 0; i < C_ScreenControlManager::Instance().Count(); i++ )
				{
					const char *pScreenName = C_ScreenControlManager::Instance().Get(i).GetCurrentScreen();
					if( pScreenName && stricmp( pScreenName, pUIScreen->m_Name ) == 0 )
					{
						C_ScreenList::Instance().SetDirty( (C_ScreenList::E_DirtyFlags)(1 << i) );
						break;
					}

					// PR - allow the check of another screen (I use it for subscreens)
					const char* pScreenName2 = NULL;
					Lua::Call( g_pUILua->GetState(), "GetCurrentScreen", ">s", &pScreenName2 );

					if ( pScreenName2 && !stricmp( pScreenName2, pUIScreen->m_Name ) )
					{
						C_ScreenList::Instance().SetDirty( C_ScreenList::E_DirtyFlags_Screen );
						break;
					}
				}
			}

			C_ScreenList::Instance().GetList().Insert( pUIScreen );
		}
		break;

	case kXtEtDestruct:
		C_ScreenList::Instance().GetList().Erase( pUIScreen );
		break;
	}
}

const C_Screen*	C_ScreenList::FindByName( const char *pName )
{
	if( pName == NULL || pName[ 0 ] == NULL )
		return NULL;

	if(m_List.Size() == 0)
		return NULL;

	for(List::Iterator it = m_List.Begin(); it != m_List.End(); ++it)
	{
		const C_Screen* pScreen = *(it);
		if( pScreen->m_Name && stricmp( pScreen->m_Name, pName ) == 0 )
		{
			return pScreen;
		}
	}
	return NULL;
}

const C_GadgetBase* C_Screen::FindGadgetByName( const char* name ) const
{
	for( XtI32 i = 0; i < m_Elements.count; i++)
		if( m_Elements[ i ] && m_Elements[ i ]->m_Name )
			if( strcmp( name, m_Elements[ i ]->m_Name ) == 0 )
				return m_Elements[ i ];

	return NULL;
}

const U32 C_Screen::AddElementByName( const char* name, const XtRecord* root, const S32 context, const bool cloneHeirachy ) const
{
	C_ButtonId id;

	UiXt::C_PointerMap pointerMap;

	ButtonArray gadgetList;

	bool bInsertRoot = false;
	XtI32 iRootIndex = -1;

	// Find root
	C_GadgetBase *pFound = NULL;
	for( XtI32 i = 0; i < m_Elements.count && !pFound; i++)
	{
		if( m_Elements[ i ] && strcmp( m_Elements[ i ]->m_Name, name ) == 0 )
		{
			XtType* pType = UiXtGetTypeByPtr( root, m_Elements[ i ] );
			if( !pType )
				return C_ButtonId().Number();

			pFound = m_Elements[ i ];
			iRootIndex = i;
			bInsertRoot = true;
			break;
		}
	}
	if( !pFound )
		return C_ButtonId().Number();

	if( cloneHeirachy )
	{
		// Add all children to found
		for( XtI32 i = 0; i < m_Elements.count; i++)
		{
			if( !m_Elements[ i ] )
				continue;

			bool parentToFound = false;
			for( C_GadgetBase *p = m_Elements[ i ]->Parent(); p; p = p->Parent() )
				if( p == pFound )
				{
					parentToFound = true;
					break;
				}

			if( !parentToFound )
				continue;

			// The root before this item?
			if( bInsertRoot && i > iRootIndex )
			{				
				gadgetList.PushBack( m_Elements[ iRootIndex ] );
				iRootIndex = gadgetList.Size() - 1; // Replace
				bInsertRoot = false;
			}

			gadgetList.PushBack( m_Elements[ i ] );
		}
	}
	// Root comes last?
	if( bInsertRoot )
	{
		gadgetList.PushBack( m_Elements[ iRootIndex ] );
		iRootIndex = gadgetList.Size() - 1; // Replace
		bInsertRoot = false;
	}

	// Create & setup
	for( S32 i = 0; i < gadgetList.Size(); i++ )
	{
		C_GadgetBase *p = gButtons.AddButton( (E_UIButtonType)gadgetList[ i ]->GetType(), context );
		if( !p )
			continue;

		XtType* pType = UiXtGetTypeByPtr( root, gadgetList[ i ] );
		if( !pType )
			continue;

		CloneFromDefImpl( (U8 *)p, pType, (U8 *)gadgetList[ i ], p->m_XtAllocator, pointerMap );
		p->Setup( NULL );

		gadgetList[ i ] = p;
	}
	// Store root node
	C_GadgetBase *pRoot = gadgetList[ iRootIndex ];
	if( !pRoot )
		return C_ButtonId().Number();

	// Remap
	pointerMap.CreateReferencedData( root, pRoot->m_XtAllocator );
	pointerMap.ListRemap();
	pointerMap.ClearBindings();

	for( S32 i = 0; i < gadgetList.Size(); i++ )
	{
		C_GadgetBase *p = gadgetList[i];
		p->SetParent(p->Parent());
	}

	// Bind Timeline & PostInit if we're called outside of the Init pass
	if( C_ScreenControlManager::Instance().Get(context).InitState() == false )
	{
		for( S32 i = 0; i < gadgetList.Size(); i++ )
		{
			C_GadgetBase *p = gadgetList[ i ];
			if( !p )
				continue;

			p->BindTimeline();
			p->PostInit();

			p->RemoveSubscriber( E_UiMsg_PostInit );
			p->RemoveSubscriber( E_UiMsg_BindTimelines );
		}
	}
	
	// Return root node id
	return pRoot->m_ID.Number();
}

void C_Screen::AddElements( lua_State* pState, const XtRecord* root, const S32 context, const char* pTableName, C_PointerMap& pointerMap ) const
{
	bool add_items = false;
	bool add_table = false;
	if( pState )
	{
		if ( pTableName && pTableName[ 0 ] )
			lua_getglobal( pState, pTableName );
		else
			lua_getglobal( pState, "SCUI" );

		add_items = lua_type( pState, -1 ) == LUA_TTABLE;

		lua_pushstring(pState, "name_to_id");
		add_table = lua_type( pState, -1 ) != LUA_TTABLE;

		// No table - let's create one
		if( add_table )
			lua_newtable( pState );
	}
	
	for( XtI32 i = 0; i < m_Elements.count; i++)
	{
		if( m_Elements[ i ] && !m_Elements[ i ]->ExportToLua() )
		{
			XtType* pType = UiXtGetTypeByPtr( root, m_Elements[ i ] );
			if( !pType )
				continue;

			C_GadgetBase *p = gButtons.AddButton( (E_UIButtonType)m_Elements[ i ]->GetType(), context );
			if( !p )
				continue;

			CloneFromDefImpl( (U8 *)p, pType, (U8 *)m_Elements[ i ], p->m_XtAllocator, pointerMap );

			p->Setup( NULL );

			if( add_items && p->m_Name )
			{
				Lua::AddDataMember( pState, p->m_Name, p->m_ID.Number() );
			}				
		}
	}

	// Finish table
	if( add_table )
		lua_rawset(pState,-3);

	if( add_items )
		lua_pop(pState,1);
}

void C_Screen::MakeLuaTable( lua_State* pState, const S32 context, const bool bForceNewTable, C_PointerMap& pointerMap ) const
{
	bool bAddTable = bForceNewTable;

	if ( bAddTable )
	{
		lua_newtable( pState );
	}
	else
	{
		// check for table first
		if ( m_GuiTableName && m_GuiTableName[ 0 ] )
			lua_getglobal( pState, m_GuiTableName );
		else
			lua_getglobal( pState, "SCUI" );

		bAddTable = lua_type( pState, -1 ) != LUA_TTABLE;

		if ( bAddTable )
			lua_newtable( pState );		// start new table
	}

	// Elements
	if( m_Elements.count )
	{
		lua_pushstring( pState, "elements" );
		lua_newtable( pState );
		{
			S32 cnt = 1;
			for( XtI32 i = 0; i < m_Elements.count; i++)
			{
				if( m_Elements[ i ] && m_Elements[ i ]->ExportToLua() )
				{
					// Push the table data
					m_Elements[ i ]->m_ID = C_ButtonId( context, 0 );	// Set the record to current context
					m_Elements[ i ]->StoreXtType();						// Make sure we're got XtType set
					m_Elements[ i ]->LuaSerialise( pState, context, pointerMap );
					Lua::AddDataMember( pState, "type", m_Elements[ i ]->GetType() );
					// Set the table
					lua_rawseti(pState, -2, i + 1);
				}
			}
		}
		lua_rawset(pState,-3);
	}

	// Push commands
	if( m_OnNextCmd && m_OnNextCmd[ 0 ] )
		Lua::AddDataMember( pState, "on_next", m_OnNextCmd );

	if( m_OnBackCmd && m_OnBackCmd[ 0 ] ) 
		Lua::AddDataMember( pState, "on_back", m_OnBackCmd );

	// Local colour styles
	if( m_ColourStyles.count )
	{
		lua_pushstring( pState, "colour_styles" );
		lua_newtable( pState );
		{
			S32 cnt = 1;
			for( XtI32 i = 0; i < m_ColourStyles.count; i++)
			{
				if( m_ColourStyles[ i ].m_Name && strlen( m_ColourStyles[ i ].m_Name ) > 0 )
				{
					lua_pushstring( pState, m_ColourStyles[ i ].m_Name );
				}
				else
				{
					char buffer[16];
					sprintf_s( buffer, 16, "unnamed_%02d", cnt );
					lua_pushstring( pState, buffer );

					cnt++;
				}
				UiXt::MakeLuaFromDef( pState, m_ColourStyles[ i ], pointerMap, NULL );

				lua_rawset(pState,-3);
			}
		}
		lua_rawset(pState,-3);
	}
/*
	// Local font styles
	lua_pushstring( pState, "font_styles" );
	lua_newtable( pState );

	lua_rawset(pState,-3);
*/
	if ( bAddTable )
	{
		if ( m_GuiTableName && m_GuiTableName[ 0 ] )
			lua_setglobal( pState, m_GuiTableName );
		else
			lua_setglobal( pState, "SCUI" );
	}
	else
		lua_pop( pState, 1 );
}

const C_FontStyle * C_Screen::FindFontStyleByName( const char *pName ) const
{
	if( pName == NULL || pName[ 0 ] == NULL || m_FontStyles.elements == NULL )
		return NULL;

	for(XtI32 i = 0; i < m_FontStyles.count; i++)
	{
		if( m_FontStyles[ i ].m_Name != NULL && stricmp( m_FontStyles[ i ].m_Name, pName ) == 0 )
			return &m_FontStyles[ i ];
	}

	return NULL;
}

const C_ColourStyle * C_Screen::FindColourStyleByName( const char *pName ) const
{
	if( pName == NULL || pName[ 0 ] == NULL || m_ColourStyles.elements == NULL )
		return NULL;

	for(XtI32 i = 0; i < m_ColourStyles.count; i++)
	{
		if( m_ColourStyles[ i ].m_Name != NULL && stricmp( m_ColourStyles[ i ].m_Name, pName ) == 0 )
			return &m_ColourStyles[ i ];
	}

	return NULL;
}

// Global functions
template <typename Type>
static void AddFlags( lua_State* pState, const U8 *structPtr, XtType* def, int offset )
{
	Type flags = *(Type *)(structPtr + offset);

	for(XtU32 i=0; i < def->fieldCount; i++)
	{
		XtField *flug = &def->fields[i];

		Lua::AddDataMember(pState,flug->name,(flags & flug->value) != 0);
	}
}

template <typename Type>
static void AddFlags( lua_State* pState, const U8 *structPtr, XtField& field)
{
	AddFlags<Type>(pState, structPtr, (XtType*)field.value, field.offset);
}

template <typename Type>
static void AddNumberField( lua_State* pState, const U8 *structPtr, const XtField &field )
{
	if( field.array == 0)
	{
		Type *number = (Type *)(structPtr + field.offset);

		Lua::AddDataMember( pState, field.name, *number );
	}
	else
	{
		XtArray< Type >* array = (XtArray< Type >*)(structPtr + field.offset);
		if( array->count )
		{
			lua_pushstring(pState,field.name);
			lua_newtable(pState);

			for(S32 index = 0; index < array->count; index ++)
			{
				lua_pushnumber(pState, (lua_Number)array->elements[ index ] );
				lua_rawseti(pState, -2, index + 1);
			}
			lua_rawset(pState, -3);
		}
	}
}

void AddStringField( lua_State* pState, const U8 *structPtr, const XtField &field )
{
	if( field.array == 0 )
	{
		const char *string = *(const char **)(structPtr + field.offset);

		Lua::AddDataMember(pState,field.name,string);
	}
	else
	{
		XtArray< const char * >* array = (XtArray< const char * >*)(structPtr + field.offset);
		if( array->count )
		{
			lua_pushstring(pState,field.name);
			lua_newtable(pState);

			for(S32 index = 0; index < array->count; index ++)
			{
				lua_pushstring(pState, array->elements[ index ] );
				lua_rawseti(pState, -2, index + 1);
			}
			lua_rawset(pState, -3);
		}
	}
}

void AddBoolField( lua_State* pState, const U8 *structPtr, const XtField &field )
{
	if( field.array == 0)
	{
		bool *number = (bool *)(structPtr + field.offset);

		Lua::AddDataMember( pState, field.name, *number );
	}
	else
	{
		XtArray< bool >* array = (XtArray< bool >*)(structPtr + field.offset);
		if( array->count )
		{
			lua_pushstring(pState,field.name);
			lua_newtable(pState);

			for(S32 index = 0; index < array->count; index ++)
			{
				lua_pushboolean(pState, array->elements[ index ] );
				lua_rawseti(pState, -2, index + 1);
			}
			lua_rawset(pState, -3);
		}
	}
}


void MakeLuaFromDefImpl(lua_State *pState,XtType* def,const U8 *structPtr, C_PointerMap& pointerMap, const XtRecord* root )
{
	if(structPtr == NULL)
	{
		//LOGFATAL( Def::g_Log,  "Bad sorcery data %s\n", def.name);
		return;
	}

	if (def->behaviour == kXtTbStruct)
	{
		if( def->baseCount > 0 )
		{
			assert( def->baseCount == 1 ); // Not sure this will work for more than one base
			MakeLuaFromDefImpl( pState, def->bases[ 0 ].type, structPtr + def->bases[ 0 ].offset, pointerMap, root );
		}
		else
		{
			lua_newtable(pState);
		}

		// Check if we can be referenced?
		if( XtTypeReferencable( def ) )
		{
			// Store who constructed 'me'
			Lua::AddDataMember( pState, "__src", (void*)structPtr );

			pointerMap.AddBinding( (const void *)structPtr, NULL );
		}

		for(XtU32 i = 0; i < def->fieldCount; i++)
		{
			XtField &field = def->fields[i];

			switch(field.behaviour)
			{
			case kXtFbPtr:
				if( root )
				{
					if( field.array == 0 )
					{
						// Store pointer source
						void**data = (void**)(structPtr + field.offset);

						if( *data )
						{
							Lua::AddDataMember( pState, field.name, *data );

							pointerMap.AddReference( *data );
						}
					}
					else
					{
						XtArray<void *>* array = (XtArray<void *>*)(structPtr + field.offset);
						if( array->count )
						{
							lua_pushstring(pState,field.name);
							lua_newtable(pState);

							// array->length - num entries
							// array->data - data
							for(S32 index = 0; index < array->count; index ++)
							{
								lua_pushlightuserdata(pState, (void*)array->elements[ index ] );
								lua_rawseti(pState, -2, index + 1);

								pointerMap.AddReference( array->elements[ index ] );
							}
							lua_rawset(pState, -3);
						}
					}
				}
				break;

			case kXtFbStruct:
				if( field.array == 0 )
				{
					lua_pushstring(pState,field.name);

					U8 *data = (U8 *)(structPtr + field.offset);

					MakeLuaFromDefImpl( pState, field.type, data, pointerMap, root );

					lua_rawset(pState,-3);
				}
				else
				{
					
					XtArray<U8>* array = (XtArray<U8>*)(structPtr + field.offset);
					if( array->count )
					{
						lua_pushstring(pState,field.name);
						lua_newtable(pState);

						// array->length - num entries
						// array->data - data
						for(S32 index = 0; index < array->count; index ++)
						{
							U8 * data = (U8*)array->elements + (field.type->size * index);
							
							MakeLuaFromDefImpl( pState, field.type, data, pointerMap, root );

							lua_rawseti(pState, -2, index + 1);
						}
						lua_rawset(pState, -3);
					}
				}
				break;

			case kXtFbAtom:	// atomic type
			{
				switch(field.type->atom)
				{
				// Number types
				case kXtU8: AddNumberField< U8 >( pState, structPtr, field ); break;
				case kXtI8: AddNumberField< S8 >( pState, structPtr, field ); break;
				case kXtI16: AddNumberField< S16 >( pState, structPtr, field ); break;
				case kXtU16: AddNumberField< U16 >( pState, structPtr, field ); break;
				case kXtI32: AddNumberField< S32 >( pState, structPtr, field ); break;
				case kXtU32: AddNumberField< U32 >( pState, structPtr, field ); break;
				case kXtF32: AddNumberField< F32 >( pState, structPtr, field ); break;
				case kXtF64: AddNumberField< F64 >( pState, structPtr, field ); break;
				case kXtLocId: AddNumberField< U32 >( pState, structPtr, field ); break;
				// Non number types
				case kXtSz8:	AddStringField( pState, structPtr, field ); break;
				case kXtBool:	AddBoolField( pState, structPtr, field ); break;
				// Unsupported
				default:
					LOGWARN( g_Log, DBSTR "Type %d not supported", field.type->atom );
					break;
				}
			}
			break;
			
			// enum option
			case kXtFbLabel:
			{
				Lua::AddDataMember(pState,field.name,(S32)field.value);
			}
			break;

			case kXtFbHandle:
				if( field.array == 0 )
				{
					XtHandle< void * >* src_ptr = (XtHandle< void * >*)(structPtr + field.offset);

					lua_pushstring( pState, "_handle" );
					lua_newtable( pState );
						Lua::AddDataMember( pState, "_record", (void*)src_ptr->Record() );
						Lua::AddDataMember( pState, "_salt", src_ptr->Salt() );
						Lua::AddDataMember( pState, "_offset", src_ptr->Offset() );
					lua_rawset(pState, -3);
				}
				else
				{
					LOGWARN( g_Log, DBSTR "Exporting arrays of handles is not supported" );
				}
				break;
			case kXtFbFlags:
			{
				lua_pushstring(pState,field.name);
					MakeLuaFromDefImpl(pState, (XtType*)field.value, structPtr + field.offset, pointerMap, root);
				lua_rawset(pState,-3);
			}
			break;

			case kXtFbEnum:	// enum -> number
			{
				S32 *number = (S32 *)(structPtr + field.offset);

				Lua::AddDataMember(pState,field.name,*number);
				// outputs as string 
	//			Lua::AddDataMember(pState,field.name,field.type->fields[*number].name);		// add range check for number
			}
			break;

			default:
				break;
			}
		}
	}
	else 
	if (def->behaviour == kXtTbFlags)
	{
		// Ewe
		lua_newtable(pState);

		switch(def->atom)
		{
		case kXtU8: AddFlags< U8 >( pState, structPtr, def, 0 ); break;
		case kXtI8: AddFlags< S8 >( pState, structPtr, def, 0 ); break;
		case kXtI16: AddFlags< S16 >( pState, structPtr, def, 0 ); break;
		case kXtU16: AddFlags< U16 >( pState, structPtr, def, 0 ); break;
		case kXtI32: AddFlags< S32 >( pState, structPtr, def, 0 ); break;
		case kXtU32: AddFlags< U32 >( pState, structPtr, def, 0 ); break;
		default:
			LOGWARN( g_Log, DBSTR "Type %d not flags not supported", def->atom );
			break;
		}
	}
}

template <typename Type>
static void GetFlags( lua_State* pState, U8 *structPtr, const XtType* def, int offset )
{
	Type* flags = (Type *)(structPtr + offset);

	for(XtU32 i=0; i < def->fieldCount; i++)
	{
		XtField *flug = &def->fields[i];

		bool set;
		Lua::GetDataMember(pState,flug->name,set);

		if( set )
		{
			*flags |= flug->value;
		}
	}
}

template <typename Type>
static void GetNumberField( lua_State* pState, const U8 *structPtr, const XtField &field, C_Allocator& allocator )
{
	if( field.array == 0 )
	{
		if(Lua::DataMemberType(pState,field.name) != LUA_TNUMBER)
			return;

		Type number;
		Lua::GetDataMember(pState,field.name,number);

		*(Type *)(structPtr + field.offset) = ( Type )number;
	}
	else
	{
		lua_pushstring(pState,field.name);
		if( lua_type(pState, -2 ) == LUA_TTABLE)
		{
			lua_gettable(pState, -2);

			int size = luaL_getn(pState,-1);
			if( size )
			{
				XtArray< Type >* array = (XtArray< Type >*)(structPtr + field.offset);
				assert( !array->elements );

				array->elements = (Type*)allocator.New( sizeof( Type ) * size );
				array->count = size;

				for(int i = 0; i < size; i++)
				{
					lua_rawgeti(pState,-1,i+1); // Push on index [i]
					
					array->elements[ i ] = ( Type )lua_tonumber( pState, -1 );

					lua_pop(pState, 1); // pop off[i]
				}
			}
		}
		lua_pop( pState, 1 ); // Pop off array table name
	}
}

void GetBoolField( lua_State* pState, const U8 *structPtr, const XtField &field, C_Allocator& allocator )
{
	if( field.array == 0 )
	{
		if(Lua::DataMemberType(pState,field.name) != LUA_TBOOLEAN)
			return;

		bool number;
		Lua::GetDataMember(pState,field.name,number);

		*(bool *)(structPtr + field.offset) = number;
	}
	else
	{
		lua_pushstring(pState,field.name);
		if( lua_type(pState, -2 ) == LUA_TTABLE)
		{
			lua_gettable(pState, -2);

			int size = luaL_getn(pState,-1);
			if( size )
			{
				XtArray< bool >* array = (XtArray< bool >*)(structPtr + field.offset);
				assert( !array->elements );

				array->elements = (bool*)allocator.New( sizeof( bool ) * size );
				array->count = size;

				for(int i = 0; i < size; i++)
				{
					lua_rawgeti(pState,-1,i+1); // Push on index [i]
					
					array->elements[ i ] = (lua_toboolean( pState, -1 ) != 0);

					lua_pop(pState, 1); // pop off[i]
				}
			}
		}
		lua_pop( pState, 1 ); // Pop off array table name
	}
}
void GetStringField( lua_State* pState, const U8 *structPtr, const XtField &field, C_Allocator& allocator )
{
	if( field.array == 0 )
	{
		if(Lua::DataMemberType(pState,field.name) == LUA_TSTRING)
		{
			const char *string;
			Lua::GetDataMember(pState,field.name,string);

			// Make a copy
			const size_t len = strlen( string );
			char **in = (char **)(structPtr + field.offset);
			if( string && len > 0 )
			{
				*in = (char*)allocator.New( len + 1 );
				assert( *in );

				STRCPY_S( *in, len+1, string );
			}
			else
			{
				*in = NULL;
			}
		}
	}
	else
	{
		lua_pushstring(pState,field.name);
		if( lua_type(pState, -2 ) == LUA_TTABLE)
		{
			lua_gettable(pState, -2);

			int size = luaL_getn(pState,-1);
			if( size )
			{
				XtArray< const char * >* array = (XtArray< const char * >*)(structPtr + field.offset);
				assert( !array->elements );

				array->elements = (const char * *)allocator.New( field.type->size * size );
				array->count = size;

				for(int i = 0; i < size; i++)
				{
					lua_rawgeti(pState,-1,i+1); // Push on index [i]
					if( lua_type(pState, -1) == LUA_TSTRING )
					{
						const char *string = lua_tostring( pState, -1 );

						if(string)
						{
							// Make a copy
							const size_t len = strlen( string );
							char *p = NULL;
							if( string && len > 0 )
							{
								p = (char*)allocator.New( len+1 );
								STRCPY_S( p, len+1, string );
							}
							array->elements[ i ] = p;
						}
						else
						{
							array->elements[ i ] = NULL;
						}
					}
					lua_pop(pState, 1); // pop off[i]
				}
			}
		}
		lua_pop( pState, 1 ); // Pop off array table name
	}
}

void GetLuaFromDefImpl(lua_State *pState,XtType* def,U8 *structPtr, C_Allocator& allocator, C_PointerMap& pointerMap, const XtRecord* root )
{
	if(structPtr == NULL || def == NULL )
	{
		//LOGFATAL( Def::g_Log,  "Bad sorcery data %s\n", def.name);
		return;
	}

	// Get my fields
	if (def->behaviour == kXtTbStruct)
	{	
		// Get bases
		for( XtU32 i = 0; i < def->baseCount; i++ )
		{
			GetLuaFromDefImpl( pState, def->bases[ i ].type, structPtr + def->bases[ i ].offset, allocator, pointerMap, root );
		}
		// Construct my type
		// def->construct( structPtr, 1);

		// Check if we can be referenced?
		if( XtTypeReferencable( def ) )
		{
			// Get where this data was constructed from
			void *data;
			Lua::GetDataMember( pState, "__src", data );

			// Create a mapping reference
			pointerMap.AddBinding( data, structPtr );
		}
		for(XtU32 i = 0; i < def->fieldCount; i++)
		{
			XtField &field = def->fields[i];

			switch(field.behaviour)
			{
			case kXtFbAtom:	// atomic type
				switch(field.type->atom)
				{
				// Number types
				case kXtU8: GetNumberField< U8 >( pState, structPtr, field, allocator ); break;
				case kXtI8: GetNumberField< S8 >( pState, structPtr, field, allocator ); break;
				case kXtI16: GetNumberField< S16 >( pState, structPtr, field, allocator ); break;
				case kXtU16: GetNumberField< U16 >( pState, structPtr, field, allocator ); break;
				case kXtI32: GetNumberField< S32 >( pState, structPtr, field, allocator ); break;
				case kXtU32: GetNumberField< U32 >( pState, structPtr, field, allocator ); break;
				case kXtF32: GetNumberField< F32 >( pState, structPtr, field, allocator ); break;
				case kXtF64: GetNumberField< F64 >( pState, structPtr, field, allocator ); break;
				case kXtLocId : GetNumberField< U32 >( pState, structPtr, field, allocator ); break;
				// Non number types
				case kXtSz8: GetStringField( pState, structPtr, field, allocator ); break;
				case kXtBool: GetBoolField( pState, structPtr, field, allocator ); break;
				// Not supported
				default:
					LOGWARN( g_Log, DBSTR "Type %d not supported", field.type->atom );
					break;
				}
				break;

			// enum option
			case kXtFbLabel:
				if(Lua::DataMemberType(pState,field.name) == LUA_TNUMBER)
				{
					S32 number;
					Lua::GetDataMember(pState,field.name,number);

					field.value = number;
				}
				break;

			case kXtFbHandle:
				if( field.array == 0 )
				{
					XtHandle< void * >* src_ptr = (XtHandle< void * >*)(structPtr + field.offset);

					if ( Lua::DataMemberType( pState, "_handle" ) == LUA_TTABLE )
					{
						lua_pushstring( pState, "_handle" );
						lua_gettable( pState, -2 );

						void* record = NULL;
						XtU16 salt = 0;
						XtU16 offset = 0;

						Lua::GetDataMember( pState, "_record", record );
						Lua::GetDataMember( pState, "_salt", salt );
						Lua::GetDataMember( pState, "_offset", offset );

						*src_ptr = XtHandle<void*>((XtRecord*)record, salt, offset);

						lua_pop( pState, 1 );	// pop table off
					}
				}
				else
				{
					LOGWARN( g_Log, DBSTR "Reading arrays of handles is not supported" );
				}
				break;

			case kXtFbFlags:
				if(Lua::DataMemberType(pState,field.name) == LUA_TTABLE)
				{
					lua_pushstring(pState,field.name);
					lua_gettable(pState, -2);
						GetLuaFromDefImpl(pState, (XtType*)field.value, structPtr + field.offset, allocator, pointerMap, root);
					lua_pop(pState,1);
				}
				break;

			case kXtFbEnum:	// enum -> number (not string type)
				if(Lua::DataMemberType(pState,field.name) == LUA_TNUMBER)
				{
					S32 number;							
					Lua::GetDataMember(pState,field.name,number);

					*(S32 *)(structPtr + field.offset) = number;
				}
				break;

			case kXtFbPtr:
				if( root )
				{
					// Stored pointer source
					if( field.array == 0 )
					{
						if( Lua::DataMemberType(pState,field.name) == LUA_TLIGHTUSERDATA )
						{
							void **data = (void **)(structPtr + field.offset);
							
							// Get source lookup
							Lua::GetDataMember( pState, field.name, *data );
							// Store and resolve later
							pointerMap.AddToRemapList( data );
						}
					}
					else
					{
						// Array of stored src pointers
						lua_pushstring(pState,field.name);
						if( lua_type(pState, -2 ) == LUA_TTABLE )
						{
							lua_gettable(pState, -2);

							int size = luaL_getn(pState,-1);
							if( size )
							{
								XtArray< void* >* array = (XtArray< void* >*)(structPtr + field.offset);
								assert( !array->elements );

								array->elements = (void**)allocator.New( sizeof( void* ) * size );
								array->count = size;

								for(int i = 0; i < size; i++)
								{
									lua_rawgeti(pState,-1,i+1); // Push on index [i]
									
									array->elements[ i ] = ( void* )lua_touserdata( pState, -1 );

									// Object not yet create, store and resolve later
									pointerMap.AddToRemapList( &array->elements[ i ] );
		
									lua_pop(pState, 1); // pop off[i]
								}
							}

						}
						lua_pop( pState, 1 ); // Pop off array table name
					}
				}
				break;
			case kXtFbStruct:
				if( Lua::DataMemberType(pState,field.name) == LUA_TTABLE )
				{
					if (field.array == 0)
					{
						lua_pushstring(pState,field.name);
						if( lua_type(pState, -2 ) == LUA_TTABLE)
						{
							lua_gettable(pState, -2);

							U8 *data = (U8 *)(structPtr + field.offset);

							field.type->construct( data );

							GetLuaFromDefImpl( pState, field.type, data, allocator, pointerMap );
						}

						lua_pop(pState,1);
					}
					else
					{
						lua_pushstring(pState,field.name);
						if( lua_type(pState, -2 ) == LUA_TTABLE)
						{
							lua_gettable(pState, -2);

							int size = luaL_getn(pState,-1);
							if( size )
							{
								XtArray<U8>* array = (XtArray<U8>*)(structPtr + field.offset);
								if( array->elements )
								{
									allocator.Delete( array->elements );
								}

								array->elements = (U8 *)allocator.New( field.type, size );

								for(int i = 0; i < size; i++)
									field.type->construct( ((U8*)array->elements) + (i * field.type->size) );

								array->count = size;
								for(int i = 0; i < size; i++)
								{
									lua_rawgeti(pState,-1,i+1); // Push on index [i]

									GetLuaFromDefImpl( pState, field.type, array->elements + (field.type->size * i), allocator, pointerMap, root );

									lua_pop(pState, 1); // pop off[i]
								}
							}
						}
						lua_pop( pState, 1 );
					}
				}
				break;
			}
		}
	}
	else
	if (def->behaviour == kXtTbFlags)
	{
		switch(def->atom)
		{
		case kXtU8: GetFlags< U8 >( pState, structPtr, def, 0 ); break;
		case kXtI8: GetFlags< S8 >( pState, structPtr, def, 0 ); break;
		case kXtI16: GetFlags< S16 >( pState, structPtr, def, 0 ); break;
		case kXtU16: GetFlags< U16 >( pState, structPtr, def, 0 ); break;
		case kXtI32: GetFlags< S32 >( pState, structPtr, def, 0 ); break;
		case kXtU32: GetFlags< U32 >( pState, structPtr, def, 0 ); break;
		default:
			LOGWARN( g_Log, DBSTR "Type %d not flags not supported", def->atom );
			break;
		}
	}
}

void* FindMemberAddrImpl(XtType* def, const U8 *structPtr, const char *name, C_MemberAddrLink* link )
{
	void *ret = NULL;

	if( !def || !name || !link || strlen( name ) == 0 ) 
		return ret;

	if( def->baseCount > 0 )
	{
		ret = FindMemberAddrImpl( def->bases[ 0 ].type, structPtr + def->bases[ 0 ].offset, name, link );
	}

	if( ret )
		return ret;
	
	link->m_pComponentName = name;
	link->m_pDef = def;
	link->m_pComponent = (void*)structPtr;

	S32 token = -1;
	const S32 len = 128;
	char search_string[ len+1 ];
	memset( search_string, 0, len+1 );

	for(S32 i = 0; name[ i ] && token == -1 && i < len; i++)
	{
		search_string[ i ] = name[ i ];
		if( name[ i ] == '.' )
		{
			search_string[ i ] = NULL;
			token = i;
			break;
		}
	}
	const bool the_end = ( token == -1 );

	for(XtU32 i = 0; i < def->fieldCount; i++)
	{
		XtField &field = def->fields[i];

		if( stricmp( search_string, field.name ) != 0 )
			continue;

		// This is as far as we go
		if( the_end )
		{
			if( field.behaviour == kXtFbAtom ) 
			{
				BIZ_ASSERT(link->m_pNext == NULL);
				link->m_pNext = new C_MemberAddrLink();

				link->m_pNext->m_pComponentName = name;
				link->m_pNext->m_pDef = field.type;
				link->m_pNext->m_pComponent = (void*)(structPtr + field.offset);

				return (void*)(structPtr + field.offset);
			}

			// Unsupported type
			return NULL;
		}
		
		switch(field.behaviour)
		{
		case kXtFbStruct:
			{
				BIZ_ASSERT(link->m_pNext == NULL);
				link->m_pNext = new C_MemberAddrLink();

				if (field.array == 0)
				{
					U8 *data = (U8 *)(structPtr + field.offset);
					return FindMemberAddrImpl( field.type, data, name + token + 1, link->m_pNext );
				}
				else
				{
					int index = atoi( name + token + 1 ); // Note must be >10
					XtArray<U8>* array = (XtArray<U8>*)(structPtr + field.offset); 

					if( index < array->count )
					{
						return FindMemberAddrImpl( field.type, array->elements + (field.type->size * index), name + token + 3, link->m_pNext );
					}
				}
			}
			break;
		}
	}

	return ret;
}

template <typename Type>
void* FindMemberAddr( const Type &def, const char * name, XtAtomType& result_type )
{
	return FindMemberAddrImpl( XtGetType<Type>(), (const U8 *)&def, name, result_type );
}

const bool CreateLuaTableFromEnumType( lua_State *pState, const char *type_name, const char *table_name )
{
	if(!XtGetApplicationSchema())
	{
		LOGERROR( g_Log, DBSTR "Schema == NULL" );
		return false;
	}
	assert( type_name );

	XtType* pType = XtFindTypeByName( XtGetApplicationSchema(), type_name );

	if( !pType )
	{
		LOGERROR( g_Log, DBSTR "Type %s not found", type_name );
		return false;
	}
	
	if( pType->behaviour != kXtTbEnum )
	{
		LOGERROR( g_Log, DBSTR "Type %s not an enum", type_name );
		return false;
	}

	lua_pushstring(pState, table_name ? table_name : type_name );
	lua_newtable(pState);

	for( XtU32 i = 0; i < pType->fieldCount; i++)
	{
		XtField &field = pType->fields[i];
		Lua::AddDataMember( pState, field.name, (S32)field.value );	// [Blur2PC][x64] XtIntPtr is int on x86, the S32 overload
	}

	lua_rawset(pState,-3);

	return true;
}

const char* GetEnumNameFromType( const char* pTypeName, const S32 sEnumValue )
{
	const char* pDefault = "null";

	if ( !pTypeName || !pTypeName[ 0 ] || !XtGetApplicationSchema() )
		return pDefault;

	XtType* pType = XtFindTypeByName( XtGetApplicationSchema(), pTypeName );

	if ( !pType || ( pType->behaviour != kXtTbEnum ) )
		return pDefault;
	
	for ( XtU32 i = 0; i < pType->fieldCount; i++)
		if ( pType->fields[ i ].value == sEnumValue )
			return pType->fields[ i ].name;

	return pDefault;
}

C_PointerMap::C_PointerMap( )
{
	m_AddressPairs.Reserve(16);
	m_ResolveList.Reserve(16);
	m_RefList.Reserve(16);
}

C_PointerMap::~C_PointerMap( )
{
	ClearBindings();
}

// Pointer remapping
void C_PointerMap::AddBinding( const void* _src, void* _dst )
{
	// Check to see if we've already been added but without a destination
	if( _dst )
	{
		for( S32 i = 0; i < m_AddressPairs.Size(); i++ )
			if( m_AddressPairs[ i ].dst == NULL && m_AddressPairs[ i ].src == _src )
			{
				m_AddressPairs[ i ] = C_AddressPair( _src, _dst );
				return;
			}
	}

	m_AddressPairs.PushBack( C_AddressPair( _src, _dst ) );
}

void C_PointerMap::ClearBindings( )
{
	if( m_ResolveList.Size() )
	{
		LOGERROR( g_Log, DBSTR "Previous addresses have not been remmaped" );
		m_ResolveList.Clear();
	}

	m_AddressPairs.Clear();	
}

void C_PointerMap::AddReference( const void* _src )
{
	if( _src == NULL )
		return;

	for( S32 i = 0; i < m_RefList.Size(); i++ )
		if( m_RefList[ i ] == _src )
			return;

	m_RefList.PushBack( _src );
}


void C_PointerMap::AddToRemapList( void** _src )
{
	if( *_src == NULL )
		return;

	for( S32 i = 0; i < m_ResolveList.Size(); i++ )
		if( m_ResolveList[ i ] == _src )
			return;

	m_ResolveList.PushBack( _src );
}

void C_PointerMap::CreateReferencedData( const XtRecord* root, C_Allocator& allocator )
{
	Btl::Vector< const void * > expand;

	do
	{
		expand.Clear();

		// Build list of referenced data that hasn't alreay been created
		for( S32 j = 0; j < m_RefList.Size(); j++ )
			if( !HasBinding( m_RefList[ j ] ) )
			{
				XtType* pType = UiXtGetTypeByPtr( root, m_RefList[ j ] );
				if( !pType )
				{
					LOGERROR( g_Log, DBSTR "Warning : XtType not found @(0x08x) something has gone wrong", m_RefList[ j ] );
					m_RefList[ j ] = NULL;
					continue;
				}
				if( !XtTypeReferencable( pType ) )
				{
					LOGERROR( g_Log, DBSTR "Warning : XtType - %s is not referencable (is that a word?)", pType->name );
					m_RefList[ j ] = NULL;
					continue;
				}

				expand.PushBack( m_RefList[ j ] );
			}

		// Clone rerenced objects
		for( S32 j = 0; j < expand.Size(); j++ )
		{
			const void *data = expand[ j ];

			XtType* pType = UiXtGetTypeByPtr( root, data );
			if( pType )
			{
				U8* p = (U8*)allocator.New( pType->size );

				pType->construct( p );
				CloneFromDefImpl( p, pType, (U8*)data, allocator, *this );
			}
		}
	} while( expand.Size() > 0 ); // Loop incase newly created objects refenced items which haven't yet been cloned

	m_RefList.Clear();
}

void C_PointerMap::ListRemap( )
{
	if( m_RefList.Size() > 0 )
	{
		LOGERROR( g_Log, DBSTR " : Refences have not been expanded" );
		m_RefList.Clear();
	}

	for( S32 j = 0; j < m_ResolveList.Size(); j++ )
		Remap( m_ResolveList[ j ] );

	// Clean up
	m_ResolveList.Clear();
}

const bool C_PointerMap::HasBinding( const void* _src )
{
	for( S32 i = 0; i < m_AddressPairs.Size(); i++ )
		if( _src == m_AddressPairs[ i ].src )
			return true;
	
	return false;
}

const bool C_PointerMap::Remap( void** _src )
{
	for( S32 i = 0; i < m_AddressPairs.Size(); i++ )
		if( m_AddressPairs[ i ].src == *_src )
		{
			*_src = m_AddressPairs[ i ].dst;
			return true;
		}

	LOGERROR( g_Log, DBSTR " 0x%08x mapping not found\n", *_src );
	*_src = NULL;

	return false;
}

} // namespace UiXt

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on) 
#endif
