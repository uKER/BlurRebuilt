#include "stdafx.h"

#include "ui/Game/UIDataFinder.h"
#include "ui/Game/UILUA.h"
#include "ui/Game/StringStore.h"
#include "ui/Graphics/Localisation/Loc.h"

// Gadgets
#include "ui/Game/Gadgets/ButtonMessages.h"
#include "ui/Game/Gadgets/BaseUIList.h"
#include "ui/Game/Gadgets/BaseUIVariableSelect.h"
#include "ui/Game/Gadgets/ButtonController.h"
#include "ui/Game/Gadgets/UIStaticText.h"
#include "ui/Game/Gadgets/UIVerticalMenu.h"
#include "ui/Game/Gadgets/UIVariableSpinner.h"
#include "ui/Game/Gadgets/UISlider.h"
#include "ui/Game/Gadgets/UIItemSpinner.h"
#include "ui/Game/Gadgets/UIScrollBar.h"
#include "ui/Game/Gadgets/UIAdobeTrail.h"
#include "ui/Game/Gadgets/UINodeList.h"
#include "ui/Game/Gadgets/UIGraph.h"
#include "ui/Game/Gadgets/UIProgressBar.h"
#include "ui/Game/Gadgets/UIKeyboard.h"
#include "ui/Game/Gadgets/UITable.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

namespace UI
{

static S32 ChangeEffect(lua_State *pState)			{ return 0; }
static S32 AddEffect(lua_State *pState)				{ return 0; }
static S32 AddTimer(lua_State *pState)				{ return 0; }
static S32 UIButtons_SetTimer(lua_State *pState)	{ return 0; }
static S32 UIButtons_GetTimer(lua_State *pState)	{ return 0; }
static S32 SetTimerStart(lua_State *pState)			{ return 0; }
static S32 SetTimerDuration( lua_State* pState )	{ return 0; }

static S32 IsSelected( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "IsSelected( id ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase* pObject = gButtons.Find( (S32)lua_tonumber( pState, 1 ) );
	if ( !pObject )
		return 0;

	lua_pushboolean( pState, pObject->IsSelected() );
	return 1;
}

static S32 SetSelected(lua_State *pState)
{
	if( lua_type(pState,1) != LUA_TNUMBER || lua_type(pState,2) != LUA_TBOOLEAN )
	{
		g_pUILua->StackDump(pState,"SetSelected(num,bool) - incorrect arguments");
		return 0;
	}

	C_GadgetBase *pObject = gButtons.Find((U32)lua_tonumber(pState,1));
	if(!pObject)
		return 0;

	const bool selected = lua_toboolean(pState,2) ? true : false;
	pObject->Select( selected );

	// [optional param] Set the family tree
	if( lua_type( pState, 3 ) == LUA_TBOOLEAN )
	{
		ButtonArray children;
		if( pObject->FindChildren( &children, lua_toboolean( pState, 3 ) != 0 ) )
		{
			for( S32 j = 0; j < children.Size(); j++ )
				children[ j ]->Select( selected );
		}
	}

	return 0;
}


static S32 SetActive(lua_State *pState)
{
	if( lua_type(pState,1) != LUA_TNUMBER || lua_type(pState,2) != LUA_TBOOLEAN )
	{
		g_pUILua->StackDump(pState,"SetActive(num,bool,[bool]) - incorrect arguments");
		return 0;
	}

	C_GadgetBase *pObject = gButtons.Find((U32)lua_tonumber(pState,1));
	if(!pObject)
		return 0;

	const bool activated = lua_toboolean(pState,2) ? true : false;

	bool sendTimelineMessage = true;

	// [optional param] Send gadget_activated timeline message
	if( lua_type( pState, 4 ) == LUA_TBOOLEAN )
		sendTimelineMessage = lua_toboolean(pState,4) ? true : false;

	pObject->Activate( activated, sendTimelineMessage );

	// [optional param] Set the family tree
	if( lua_type( pState, 3 ) == LUA_TBOOLEAN )
	{
		ButtonArray children;
		if( pObject->FindChildren( &children, lua_toboolean( pState, 3 ) != 0 ) )
		{
			// Process all the children
			for( S32 j = 0; j < children.Size(); j++ )
				children[ j ]->Activate( activated, sendTimelineMessage );
		}
	}
	return 0;
}

static S32 IsActive( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "GetActive( id ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase* pObject = gButtons.Find( (S32)lua_tonumber( pState, 1 ) );
	if ( !pObject )
		return 0;

	lua_pushboolean( pState, pObject->IsActivated() );
	return 1;
}

static S32 SetMenuItemActive(lua_State *pState)
{
	if(!lua_isnumber(pState,1) || !lua_isnumber(pState,2) || !lua_isboolean(pState,3))
	{
		g_pUILua->StackDump(pState,"SetMenuItemActive(ID(num),Item(num),bool) - incorrect arguments");
		return 0;
	}

	C_GadgetBase *pObject=gButtons.Find((U32)lua_tonumber(pState,1));
	if(!pObject)
		return 0;

	((BaseUIVariableSelect *)pObject)->SetItemUnlocked( (S32)lua_tonumber(pState,2),lua_toboolean(pState,3) ? true : false );

	return 0;
}

static S32 SetPad(lua_State* pState)
{
	if(lua_type(pState,1)!=LUA_TNUMBER || lua_type(pState,2)!=LUA_TNUMBER)
	{
		g_pUILua->StackDump(pState,"SetPad(number,number) - incorrect arguments");
		return 0;
	}

	C_GadgetBase *pObject = gButtons.Find((U32)lua_tonumber(pState,1));
	if(!pObject)
		return 0;

	if ( pObject->GetType() == E_UIButtonType_NodeList )
		((UINodeList*)pObject)->SetPad((S32)lua_tonumber(pState,2));
	else
		((BaseUIVariableSelect *)pObject)->SetPad((S32)lua_tonumber(pState,2));

	return 0;
}

static S32 SetParent(lua_State* pState)
{
	if(lua_type(pState,1)!=LUA_TNUMBER )
	{
		g_pUILua->StackDump(pState,"SetParent(number,number,number) - incorrect arguments");
		return 0;
	}

	// Get child
	C_GadgetBase *pChild	= gButtons.Find( (S32)lua_tonumber(pState,1) );
	if( !pChild )
	{
		g_pUILua->StackDump(pState,"SetParent() Error : child not found");
		return 0;
	}

	if( lua_type(pState,2) == LUA_TNUMBER && lua_type(pState,3) == LUA_TNUMBER )
	{
		// Get parent
		C_GadgetBase *pParent	= gButtons.Find( (S32)lua_tonumber(pState,2) );
		if( !pParent )
		{
			g_pUILua->StackDump(pState,"SetParent() Error : parent not found");
			return 0;
		}

		E_Justify justify = (E_Justify)(S32)lua_tonumber(pState,3);

		pChild->SetParent(pParent);
		pChild->CheckParentDepth();
		pChild->m_ParentAttach = justify;
	}
	else
	if( lua_type(pState,2) == LUA_TNIL && lua_type(pState,3) == LUA_TNONE )
	{
		pChild->SetParent(0); // 'nil' passed : Clear parent
	}

	return 0;
}

static S32 ChangeMouseClickable(lua_State *pState)
{
#ifdef PLATFORM_PC
	if(!lua_isnumber(pState,1) || !lua_isboolean(pState,2))
	{
		g_pUILua->StackDump(pState,"ChangeMouseClickable(button_ID,enable) - incorrect arguments");
		return 0;
	}

	C_GadgetBase *pObject = gButtons.Find((U32)lua_tonumber(pState,1));
	if(!pObject)
		return 0;

	pObject->m_Mouse.clickable = lua_toboolean(pState,2) ? true : false;
#endif
	return 0;
}

static S32 AddTexture(lua_State *pState)
{
	if(!lua_isnumber(pState,2) || !lua_istable(pState,1))
	{
		g_pUILua->StackDump(pState,"AddTexture(table,num) - incorrect arguments");
		return 0;
	}

	C_GadgetBase *pObject=gButtons.GetCurrentObject();
	if(!pObject)
		return 0;

	const S32 slot=(U32)lua_tonumber(pState,2);
	if( slot >= pObject->m_TextureParams.count )
	{
		LOGERROR( g_Log, DBSTR "Texture slot out of range\n");
		return 0;
	}

	// Pop the stack so the table is on top
	lua_pop(pState,1);	

	assert( 0 ); // Not yet implemented
	// pObject->m_TextureParams[ slot ].Deserialise( pState );

	return 0;
}

static S32 ChangeTexture(lua_State *pState)
{
	if(!lua_isnumber(pState,3) || !lua_isnumber(pState,2) || !lua_istable(pState,1))
	{
		g_pUILua->StackDump(pState,"ChangeTexture(table,num,ID) - incorrect arguments");
		return 0;
	}

	S32 ID=(S32)lua_tonumber(pState,3);
	lua_pop(pState,1);	//pop the stack so the table is on top

	C_GadgetBase *pObject=gButtons.Find(ID);
	if(!pObject)
		return 0;

	S32 slot = (S32)lua_tonumber(pState,2);
	if( slot >= pObject->m_TextureParams.count )
	{
		LOGERROR( g_Log, DBSTR "Texture slot out of range\n");
		return 0;
	}
	//pop the stack so the table is on top
	lua_pop(pState,1);	

	// Free if allocated
	if( pObject->m_Name )
		pObject->m_XtAllocator.Delete( const_cast<char*>(pObject->m_TextureParams[ slot ].m_Name.Text() ) );

	UiXt::C_PointerMap pointerMap;
	UiXt::GetLuaFromDefImpl( pState, XtGetType<C_TextureParam>(), (U8*)&pObject->m_TextureParams[ slot ], pObject->m_XtAllocator, pointerMap, NULL );
	
	pointerMap.ListRemap();

	pObject->m_TextureParams[ slot ].Bind();

	return 0;
}

static S32 ChangeTextureUV( lua_State *pState )
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "ChangeTextureUVs( id:num, index:num, u:num, v:num ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase* pObject = gButtons.Find( (U32)lua_tonumber( pState, 1 ) );
	if( pObject )
	{
		C_TextureParam& texture = pObject->GetTextureParam( (S32)lua_tonumber( pState, 2 ) );

		if ( lua_type( pState, 3 ) == LUA_TNUMBER )
			texture.m_Position.u = (F32)lua_tonumber( pState, 3 );

		if ( lua_type( pState, 4 ) == LUA_TNUMBER )
			texture.m_Position.v = (F32)lua_tonumber( pState, 4 );

		if ( lua_type( pState, 5 ) == LUA_TNUMBER )
			texture.m_Size.u = (F32)lua_tonumber( pState, 5 );

		if ( lua_type( pState, 6 ) == LUA_TNUMBER )
			texture.m_Size.v = (F32)lua_tonumber( pState, 6 );
	}
	return 0;
}

static S32 ChangeEffectIndex( lua_State* pState )
{
	if ( !lua_isnumber( pState, 1 ) || !lua_isnumber( pState, 2 ) )
	{
		g_pUILua->StackDump( pState, "ChangeEffectIndex( ID, index ) - incorrect arguments" );
		return 0;
	}

	S32 id = (S32)lua_tonumber( pState, 1 );
	
	C_GadgetBase* pObject = gButtons.Find( id );

	if( !pObject )
	{
		LOGWARN( g_Log, "ChangeEffectIndex : Button not found ( %d )", id );
		return 0;
	}
	pObject->m_RenderState.m_EffectIndex = (U8)Maths::Clamp<U16>( (U16)lua_tonumber( pState, 2 ), 0, 15 );

	return 0;
}

static void UpdateButtonData(lua_State* pState, C_GadgetBase *pObject, UiXt::C_PointerMap& pointerMap )
{
	pObject->LuaDeserialise( pState, pointerMap );
	pObject->Setup( pState );
}

static S32 AddColumn(lua_State *pState)
{
	LOGERROR( g_Log, DBSTR " : Deprecated");
	return 0;
}

// push void* as boolean (with cast)
void PtrToBoolean( lua_State* pState, void* data )
{
	lua_pushboolean( pState, *((bool*)data) );
}

// push void* as lua_Integer (with cast)
template< typename _Type >
void PtrToInteger( lua_State* pState, void* data )
{
	lua_pushinteger( pState, (lua_Integer) *((_Type*)data) ); 
}

// push void* as lua_Number (with cast)
template< typename _Type >
void PtrToNumber( lua_State* pState, void* data )
{
	lua_pushnumber( pState, (lua_Number) *((_Type*)data) ); 
}

void PtrToString( lua_State* pState, void* data )
{
	const char** ptr = (const char**)data;

	lua_pushstring( pState, *ptr ); 
}

static S32 GetXtVar(lua_State* pState)
{
	if( lua_type(pState,1) != LUA_TNUMBER || lua_type(pState,2) != LUA_TSTRING )
	{
		g_pUILua->StackDump(pState,"GetXtVar(number, string, (number | bool | string) ) - incorrect arguments");
		return 0;
	}

	// Find gadget
	C_GadgetBase *pObject = gButtons.Find((U32)lua_tonumber(pState,1));
	if( !pObject || !pObject->m_pXtType )
	{
		g_pUILua->StackDump(pState,"GetXtVar() - pObject not found");
		return 0;
	}

	// Find member
	UiXt::C_MemberAddrLink link;
	void *data = UiXt::FindMemberAddrImpl( pObject->m_pXtType, (U8*)pObject, lua_tostring( pState, 2 ), &link );
	if( !data )
	{
		g_pUILua->StackDump(pState,"GetXtVar() - element not found");
		return 0;
	}

	// Find last link
	UiXt::C_MemberAddrLink *p = &link;
	for(; p && p->m_pNext; p = p->m_pNext ) 
		{ 
			// Nothing
		}

	if( !p || !p->m_pDef )
	{
		g_pUILua->StackDump(pState,"GetXtVar() - address link error / XtDef");
		return 0;
	}

	switch( p->m_pDef->atom )
	{
	case kXtBool:	PtrToBoolean( pState, data );			break;
	case kXtI8:		PtrToInteger<XtI8>( pState, data );		break;
	case kXtU8:		PtrToInteger<XtU8>( pState, data );		break;
	case kXtI16:	PtrToInteger<XtI16>( pState, data );	break;
	case kXtU16:	PtrToInteger<XtU16>( pState, data );	break;
	case kXtI32:	PtrToInteger<XtI32>( pState, data );	break;
	case kXtU32:	PtrToInteger<XtU32>( pState, data );	break;
	case kXtF32:	PtrToNumber<XtF32>( pState, data );		break;
	case kXtF64:	PtrToNumber<XtF32>( pState, data );		break;
	case kXtSz8:	PtrToString( pState, data );			break;

	default:
		LOGERROR( g_Log, DBSTR "Type %d not supported", link.m_pDef->atom );
		return  0;
	}

	return 1;
}

// Assign lua_toboolean to void* (with cast)
void BooleanToPtr( lua_State* pState, int param, void* data )
{
	if( lua_type( pState, param ) != LUA_TBOOLEAN )
		g_pUILua->StackDump(pState,"BooleanToPtr : XtType not found");

	if( !data )
		return;

	*((bool*)data) = ( lua_toboolean( pState, param ) != 0 ); 
}

// Assign lua_tointeger to void* (with cast)
template< typename _Type >
void IntegerToPtr( lua_State* pState, int param, void* data )
{
	if( lua_type( pState, param ) != LUA_TNUMBER )
		g_pUILua->StackDump(pState,"IntegerToPtr : XtType not found");

	if( !data )
		return;

	*((_Type*)data) = (_Type)lua_tointeger( pState, param ); 
}

// Assign lua_tonumber to void* (with cast)
template< typename _Type >
void NumberToPtr( lua_State* pState, int param, void* data )
{
	if( lua_type( pState, param ) != LUA_TNUMBER )
		g_pUILua->StackDump(pState,"NumberToPtr : XtType not found");

	*((_Type*)data) = (_Type)lua_tonumber( pState, param ); 
}

static S32 SetXtVar(lua_State* pState)
{
	if( lua_type(pState,1) != LUA_TNUMBER || lua_type(pState,2) != LUA_TSTRING )
	{
		g_pUILua->StackDump(pState,"SetXtVar(number, string, (number | bool | string) ) - incorrect arguments");
		return 0;
	}

	// Find gadget
	C_GadgetBase *pObject = gButtons.Find((U32)lua_tonumber(pState,1));
	if( !pObject || !pObject->m_pXtType )
	{
		g_pUILua->StackDump(pState,"SetXtVar() - gadget not found");
		return 0;
	}

	// Find member
	UiXt::C_MemberAddrLink link;
	void *data = UiXt::FindMemberAddrImpl( pObject->m_pXtType, (U8*)pObject, lua_tostring( pState, 2 ), &link );
	if( !data )
	{
		g_pUILua->StackDump(pState,"SetXtVar() - element not found");
		return 0;
	}

	// Find last link
	UiXt::C_MemberAddrLink *p = &link;
	for(; p && p->m_pNext; p = p->m_pNext ) 
		{ 
			// Nothing
		}

	if( p && p->m_pDef )
	{
		switch( p->m_pDef->atom )
		{
		case kXtBool:	BooleanToPtr( pState, 3, data );		break;
		case kXtI8:		IntegerToPtr<XtI8>( pState, 3, data );	break;
		case kXtU8:		IntegerToPtr<XtU8>( pState, 3, data );	break;
		case kXtI16:	IntegerToPtr<XtI16>( pState, 3, data );	break;
		case kXtU16:	IntegerToPtr<XtU16>( pState, 3, data );	break;
		case kXtI32:	IntegerToPtr<XtI32>( pState, 3, data );	break;
		case kXtU32:	IntegerToPtr<XtU32>( pState, 3, data );	break;
		case kXtF32:	NumberToPtr<XtF32>( pState, 3, data );	break;
		case kXtF64:	NumberToPtr<XtF32>( pState, 3, data );	break;
		case kXtSz8:
			{
				// Yowzers
				const char*p = lua_tostring( pState, 3 );
				if( p && data )
				{
					char **ptr = (char**)data;

					if( pObject->m_XtAllocator.Delete( const_cast<char*>( *ptr ) ) )
					{
						*ptr = pObject->CopyCreateString( p );
					}
					else
					{
						g_pUILua->StackDump(pState,"SetXtVar() - element could no be deleted");
					}
				}
			}
			break;
		default:
			LOGERROR( g_Log, DBSTR "Type %d not supported", link.m_pDef->atom );
			break;
		}
	}
	return 0;
}

static S32 ChangePosition(lua_State *pState)
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump(pState,"ChangePosition( number, [number], [number], [number], [bool] ) - incorrect arguments");
		return 0;
	}

	C_GadgetBase *pObject = gButtons.Find( (U32)lua_tointeger( pState, 1 ) );
	if( !pObject )
		return 0;

	// Offset from position?
	const bool offset = ( lua_isboolean( pState, 5 ) && lua_toboolean( pState, 5 ) != 0 );

	// X
	if( lua_type( pState, 2 ) == LUA_TNUMBER )
	{
		F32 x = (F32)lua_tonumber( pState, 2 );
		if( offset )
			x += pObject->m_Position.x;
		
		pObject->m_Position.x = x;
	}
	// Y
	if( lua_type( pState, 3 ) == LUA_TNUMBER )
	{
		F32 y = (F32)lua_tonumber( pState, 3 );
		if( offset )
			y += pObject->m_Position.y;
		
		pObject->m_Position.y = y;
	}
	// Z
	if( lua_type( pState, 4 ) == LUA_TNUMBER )
	{
		F32 z = (F32)lua_tonumber( pState, 4 );
		if( offset )
			z += pObject->m_Position.z;
		
		pObject->m_Position.z = z;
	}

#ifdef PLATFORM_PC
	pObject->SetDefaultMouseBoundingBox();

	E_UIButtonType type = (E_UIButtonType)pObject->GetType();
	if ( type == E_UIButtonType_StaticText )
		((UIStaticText*)pObject)->SetupMouseBoundingBox();
#endif

	return 0;
}

#ifdef _UI_AUTO_CLIPWINDOW
static S32 ChangeClipWindow(lua_State *pState)
{
	if(!lua_isnumber(pState,1) || !lua_isnumber(pState,2) || !lua_isnumber(pState,3) || !lua_isnumber(pState,4) || !lua_isnumber(pState,5))
	{
		g_pUILua->StackDump(pState,"ChangePosition(ID,clipx,clipy,clipsizex,clipsizey) - incorrect arguments");
		return 0;
	}

	C_GadgetBase *pObject=gButtons.Find((U32)lua_tonumber(pState,1));
	if(!pObject)
		return 0;

	pObject->m_ClipPos.x=(F32)lua_tonumber(pState,2);
	pObject->m_ClipPos.y=(F32)lua_tonumber(pState,3);

	pObject->m_ClipSize.x=(F32)lua_tonumber(pState,4);
	pObject->m_ClipSize.y=(F32)lua_tonumber(pState,5);

	return 0;
}
#endif

static S32 ChangeSize(lua_State *pState)
{
	if(!lua_isnumber(pState,1) || !lua_isnumber(pState,2))
	{
		g_pUILua->StackDump(pState,"ChangeSize(ID,width,height) - incorrect arguments");
		return 0;
	}

	C_GadgetBase *pObject=gButtons.Find((U32)lua_tonumber(pState,1));
	if(!pObject)
		return 0;

	pObject->m_Size.x = (F32)lua_tonumber(pState,2);
	pObject->m_Size.y = (F32)lua_tonumber(pState,3);

	// Static text is special
	if ( pObject->GetType() == E_UIButtonType_StaticText )
	{
		// Calculate height again
		((UIStaticText*)pObject)->ChangeSize( (F32)lua_tonumber(pState,2), (F32)lua_tonumber(pState,3 ) );
	}

	return 0;
}


static S32 ChangeSizeX(lua_State *pState)
{
	if(!lua_isnumber(pState,1) || !lua_isnumber(pState,2))
	{
		g_pUILua->StackDump(pState,"ChangeSizeX(ID,x) - incorrect arguments");
		return 0;
	}

	C_GadgetBase *pObject=gButtons.Find((U32)lua_tonumber(pState,1));
	if(!pObject)
		return 0;

	pObject->m_Size.x = (F32)lua_tonumber(pState,2);

	return 0;
}


static S32 ChangeScale(lua_State* pState)
{
	if( lua_type(pState, 1) != LUA_TNUMBER || lua_type(pState, 2) != LUA_TNUMBER || lua_type(pState, 3) != LUA_TNUMBER )
	{
		g_pUILua->StackDump(pState,"ChangeScale(number, number, number, [number] ) - incorrect arguments");
		return 0;
	}

	C_GadgetBase* pObject = gButtons.Find((U32)lua_tonumber(pState, 1));
	if(!pObject)
		return 0;

	pObject->m_Scale.x = (F32)lua_tonumber(pState, 2);
	pObject->m_Scale.y = (F32)lua_tonumber(pState, 3);

	// Option : change 'z' element
	if( lua_type( pState, 4) == LUA_TNUMBER )
		pObject->m_Scale.z = (F32)lua_tonumber(pState, 4);
	
	return 0;
}

static S32 ChangeJustification( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump(pState,"ChangeJustification( ID, justification ) - incorrect arguments");
		return 0;
	}

	C_GadgetBase* pObject = gButtons.Find((U32)lua_tonumber(pState, 1));
	if(!pObject)
		return 0;

	pObject->m_Justify = (E_Justify)(S32)lua_tonumber( pState, 2 );

	return 0;
}

static S32 ChangeLayer( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump(pState,"ChangeLayer( ID, layer, [children] ) - incorrect arguments");
		return 0;
	}

	C_GadgetBase* pObject = gButtons.Find( (U32)lua_tonumber( pState, 1 ) );

	if ( !pObject )
		return 0;

	LayerIndex layer = (LayerIndex)lua_tonumber( pState, 2 );
	pObject->m_Layer = layer;

	if ( ( lua_type( pState, 3 ) != LUA_TBOOLEAN ) || ( lua_toboolean( pState, 3 ) == 0 ) )
		return 0;

	ButtonArray children;
	pObject->FindChildren( &children, true );

	for ( S32 i = 0; i < children.Size(); i++ )
		children[ i ]->m_Layer = layer;

	return 0;
}

static S32 ChangePanel( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump(pState,"ChangePanel( ID, panel ) - incorrect arguments");
		return 0;
	}

	C_GadgetBase* pObject = gButtons.Find( (U32)lua_tonumber( pState, 1 ) );

	if ( !pObject )
		return 0;

	PanelIndex panel = (PanelIndex)lua_tonumber( pState, 2 );
	F32 fUp = gSimple2D.IsPanel3D( panel ) ? -1.0f : 1.0f;

	pObject->m_Panel = panel;
	pObject->m_Up = fUp;

	if ( ( lua_type( pState, 3 ) != LUA_TBOOLEAN ) || ( lua_toboolean( pState, 3 ) == 0 ) )
		return 0;

	ButtonArray children;
	pObject->FindChildren( &children, true );

	for ( S32 i = 0; i < children.Size(); i++ )
	{
		children[ i ]->m_Panel = panel;
		children[ i ]->m_Up = fUp;
	}

	return 0;
}

static S32 FindChildByName( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TSTRING )
	{
		g_pUILua->StackDump(pState,"FindChildByName( num, string ) - incorrect arguments");
		return 0;
	}

	C_GadgetBase* pObject = gButtons.Find( (U32)lua_tonumber(pState,1) );
	if( !pObject )
		return 0;

	const char* childName = lua_tostring( pState, 2 );
	if( !childName )
		return 0;

	C_GadgetBase* child = pObject->FindChildByName( childName );

	if (child)
	{
		lua_pushinteger( pState, child->m_ID.Number() );
		return 1;
	}

	return 0;
}

static S32 CloneXtGadgetByName(lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TSTRING || lua_type( pState, 2 ) != LUA_TSTRING )
	{
		g_pUILua->StackDump(pState,"CloneXtGadgetByName( string, string ) - incorrect arguments");
		return 0;
	}

	const UiXt::C_Screen* pScreen = UiXt::C_ScreenList::Instance().FindByName( lua_tostring( pState, 1 ) );
	if( !pScreen )
	{
		g_pUILua->StackDump(pState,"CloneXtGadgetByName : Error screen not found");
		return 0;
	}

	S32 sContext = C_ScreenControlManager::Instance().Context();

	if ( lua_type( pState, 3 ) == LUA_TNUMBER )
		sContext = (S32)lua_tonumber( pState, 3 );

	C_ButtonId id = pScreen->AddElementByName( lua_tostring( pState, 2 ), pScreen->m_Self.Record(), sContext );
	if( !id.Valid() )
	{
		g_pUILua->StackDump(pState,"CloneXtGadgetByName : Root gadget not found");
		return 0;
	}

	lua_pushinteger( pState, id.Number() );
	return 1;
}

static S32 AddButton(lua_State *pState)
{
	if(!lua_istable(pState,1))
	{
		g_pUILua->StackDump(pState,"AddButton(table) - incorrect arguments");
		return 0;
	}
	bool forceState = ( lua_type( pState, 2) == LUA_TBOOLEAN ) ? true : false;

	lua_settop(pState,1);
	
	C_GadgetBase *pObject;
	S32 context = C_ScreenControlManager::Instance().Context();

	if( Lua::DataMemberType( pState, "subtype") == LUA_TNUMBER )
	{
		Lua::GetDataMember( pState, "subtype", context, true );
	}

	if( Lua::DataMemberType( pState, "type") != LUA_TNUMBER )
	{
		LOGWARN( g_Log,  "type field is nil!  Check type is registered and name script name is correct\n");
		lua_pushnumber(pState,-1);
		return 1;
	}

	// Get screen control and it's pointer map
	C_ScreenControl& sc = C_ScreenControlManager::Instance().Get(context);
	UiXt::C_PointerMap& pointerMap = sc.PointerMap();

	S32 type;
	Lua::GetDataMember( pState, "type", type );
	pObject=gButtons.AddButton( (E_UIButtonType)type, context );
	if( pObject == NULL )
	{
		g_pUILua->StackDump(pState ,"AddButton() - faileed to create");
		lua_pushinteger( pState, C_ButtonId().Number() );
		return 1;
	}

	UpdateButtonData( pState, pObject, pointerMap );

	if( forceState )
	{
		bool active = lua_toboolean(pState,2) ? true : false;

		// No point being selected when its turned off so pass both
		pObject->Select( active, false );
		pObject->Activate( active, false );
	}

	if( sc.InitState() == false )
	{
		pointerMap.CreateReferencedData( sc.GetRootRecord(), pObject->m_XtAllocator );
		pointerMap.ListRemap();

		pObject->SetParent(pObject->Parent());

		pObject->BindTimeline();
		pObject->PostInit();
		pObject->RemoveSubscriber( E_UiMsg_PostInit );
		pObject->RemoveSubscriber( E_UiMsg_BindTimelines );
	}

	lua_pushinteger( pState, pObject->m_ID.Number() );
	return 1;
}

static S32 AddItem(lua_State *pState)
{
	if(!lua_isnumber(pState,1) || !lua_isnumber(pState,2) || (!lua_isnumber(pState,3) && !lua_isstring(pState,3)) || !lua_isboolean(pState,4))
	{
		g_pUILua->StackDump(pState,"AddItem(buttonid(num,val(num),string(num/string),isdata(bool)) - incorrect arguments");
//		printf(" ... and they are %d %d %d %s\n",lua_tonumber(pState,1),lua_tonumber(pState,2),lua_tonumber(pState,3),lua_toboolean(pState,4) ? "True" : "False");
		return 0;
	}

	C_GadgetBase *pObject=gButtons.Find((U32)lua_tonumber(pState,1));
	if(!pObject)
		return 0;
	
	if(lua_type(pState,3) == LUA_TSTRING)
	{
		((BaseUIVariableSelect *)pObject)->AddItem( C_UIDataFinder::Instance().GetVariableText(lua_tostring(pState,3)),(U32)lua_tonumber(pState,2) );
	}
	else
	{
		if(lua_toboolean(pState,4) ? true : false) // Use string store bodge
		{
			((BaseUIVariableSelect *)pObject)->AddItem(C_StringStore::Instance().Get((U32)lua_tonumber(pState,3)),(U32)lua_tonumber(pState,2) );
		}
		else
		{
			const WCHAR* p = C_TextStringID((U32)lua_tointeger(pState,3)).String();
			((BaseUIVariableSelect *)pObject)->AddItem(p,(U32)lua_tonumber(pState,2) );
		}
	}

	return 0;
}

static S32 ChangeButton(lua_State *pState)
{
	if(!lua_istable(pState,1) || !lua_isnumber(pState,2))
	{
		g_pUILua->StackDump(pState,"ChangeButton(table,ID) - incorrect arguments");
		return 0;
	}
	
	C_GadgetBase *pObject=gButtons.Find((U32)lua_tonumber(pState,2));
	if(!pObject)
		return 0;

	//pop off the vars so we can play with the table
	lua_settop(pState,1);

	UpdateButtonData(pState, pObject, C_ScreenControlManager::Instance().Get( pObject->m_ID.Context() ).PointerMap() );

	return 0;
}

static S32 AddItemHelp(lua_State *pState)
{
	if(lua_type(pState,1)!=LUA_TNUMBER || (lua_type(pState,2)!=LUA_TNUMBER && lua_type(pState,2)!=LUA_TSTRING))
	{
		g_pUILua->StackDump(pState,"AddItemHelp(number,(number|string)) - incorrect arguments");
		return 0;
	}

	C_GadgetBase *pObject=gButtons.Find((U32)lua_tonumber(pState,1));
	if(!pObject)
		return 0;

	if(lua_type(pState,2) == LUA_TSTRING)
	{
		((BaseUIVariableSelect *)pObject)->AddItemHelp( C_UIDataFinder::Instance().GetVariableText(lua_tostring(pState,2)) );
	}
	else
	{
		if(lua_toboolean(pState,4) ? true : false) // Use string store bodge
		{
			((BaseUIVariableSelect *)pObject)->AddItemHelp( C_StringStore::Instance().Get((U32)lua_tonumber(pState,2)) );
		}
		else
		{
			((BaseUIVariableSelect *)pObject)->AddItemHelp( C_TextStringID(lua_tonumber(pState,2)).String() );
		}
	}

	return 0;
}

static S32 AddExtraParams(lua_State *pState)
{
/*
	if(!lua_isnumber(pState,1) || !lua_isnumber(pState,2) || !lua_isnumber(pState,3))
	{
		g_pUILua->StackDump(pState,"AddExtraParams(buttonid(num,ang(num),dist(num)) - incorrect arguments");
		return 0;
	}

	C_GadgetBase *pObject=gButtons.Find((U32)lua_tonumber(pState,1));
	if(!pObject)
		return 0;

	((BaseUIVariableSelect *)pObject)->AddExtraVars((F32)lua_tonumber(pState,2),(F32)lua_tonumber(pState,3));
*/
	return 0;
}

static S32 AddSpacer(lua_State *pState)
{
/*
	if(!lua_isnumber(pState,1) || !lua_isnumber(pState,2))
	{
		g_pUILua->StackDump(pState,"AddSpacer(number,number) - incorrect arguments");
		return 0;
	}

	C_GadgetBase *pObject=gButtons.Find((U32)lua_tonumber(pState,1));
	if(!pObject)
		return 0;

	((BaseUIVariableSelect *)pObject)->AddSpacer((F32)lua_tonumber(pState,2));
*/
	return 0;
}

static S32 ClearItems(lua_State *pState)
{
	if(!lua_isnumber(pState,1))
	{
		g_pUILua->StackDump(pState,"ClearItems(num) - incorrect arguments");
		return 0;
	}

	C_GadgetBase *pObject=gButtons.Find((U32)lua_tonumber(pState,1));
	if(!pObject)
		return 0;

	pObject->ClearItems();

	return 0;
}

static S32 ResetCurrentItem(lua_State *pState)
{
	if(!lua_isnumber(pState,1))
	{
		g_pUILua->StackDump(pState,"ResetCurrentItem(num) - incorrect arguments");
		return 0;
	}

	C_GadgetBase *pObject=gButtons.Find((U32)lua_tonumber(pState,1));
	if(!pObject)
		return 0;

	pObject->ResetCurrentItem();

	return 0;
}

static S32 RefreshItems(lua_State *pState)
{
/*
	if(!lua_isnumber(pState,1))
	{
		g_pUILua->StackDump(pState,"RefreshItems(num) - incorrect arguments");
		return 0;
	}

#if defined( GAME_RACERNOTAMAX )
	C_GadgetBase *pObject=gButtons.Find((U32)lua_tonumber(pState,1));
	if(!pObject)
		return 0;

	((BaseUIVariableSelect *)pObject)->Refresh();
#endif
*/
	return 0;
}

// Get value(s) of current selection
static S32 GetSelection(lua_State *pState)
{
	if( lua_type(pState,1) != LUA_TNUMBER )
	{
		g_pUILua->StackDump(pState,"GetSelection(num,[num]) - incorrect arguments");
		return 0;
	}

	C_GadgetBase *pObject=gButtons.Find((U32)lua_tonumber(pState,1));
	if(!pObject)
		return 0;

	// Optional param list
	S32 args = 1;
	if( lua_type(pState,2) == LUA_TNUMBER )
		args = (S32)lua_tonumber(pState,2);

	for( S32 i = 0; i < args; i++)
		lua_pushnumber( pState, ((I_SelectableGadgetBase *)pObject)->GetCurrentItemValue( i ) );

	return args;
}

// Get current selection index
static S32 GetSelectionIndex( lua_State* pState )
{
	if( lua_type(pState,1) != LUA_TNUMBER )
	{
		g_pUILua->StackDump(pState,"GetSelectionIndex(num) - incorrect arguments");
		return 0;
	}

	C_GadgetBase *pObject = gButtons.Find( (U32)lua_tonumber(pState,1) );
	if(!pObject)
		return 0;

	lua_pushnumber( pState, ((I_SelectableGadgetBase *)pObject)->GetCurrentItemIndex() );

	return 1;
}

static S32 SetSelection(lua_State *pState)
{
	if( lua_type(pState,1) != LUA_TNUMBER || lua_type(pState,2) != LUA_TNUMBER )
	{
		g_pUILua->StackDump(pState,"SetSelection(num,num) - incorrect arguments");
		return 0;
	}

	C_GadgetBase *pObject=gButtons.Find((U32)lua_tonumber(pState,1));
	if(!pObject)
		return 0;

	((I_SelectableGadgetBase *)pObject)->SetCurrentItemByValue( (F64)lua_tonumber(pState,2) );
	
	return 0;
}

static S32 SetSelectionByIndex(lua_State *pState)
{
	if(!lua_isnumber(pState,1) || !lua_isnumber(pState,2))
	{
		g_pUILua->StackDump(pState,"SetSelectionByIndex(num,num) - incorrect arguments");
		return 0;
	}

	C_GadgetBase *pObject=gButtons.Find((U32)lua_tonumber(pState,1));
	if(!pObject)
		return 0;

	((I_SelectableGadgetBase *)pObject)->SetCurrentItemByIndex( (S32)lua_tonumber(pState,2) );
	
	return 0;
}

static S32 SetListSelection(lua_State *pState)
{
	if( !lua_isnumber(pState,1) || !lua_isnumber(pState,2) )
	{
		g_pUILua->StackDump(pState,"GetListSelection(num,num) - incorrect arguments");
		return 0;
	}
	C_GadgetBase* pObject = gButtons.Find( (U32)lua_tonumber(pState,1) );
	
	if( pObject )
	{
		((I_SelectableGadgetBase*)pObject)->SetCurrentItemByIndex( (S32)lua_tonumber(pState,2) );
		return 0;
	}
	return 0;
}

static S32 IsCurrentItemUnlocked( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "IsCurrentItemUnlocked( id ) - incorrect arguments" );
		return 0;
	}
	
	C_GadgetBase* pObject = gButtons.Find( (S32)lua_tonumber( pState, 1 ) );

	if ( !pObject )
		return 0;

	lua_pushboolean( pState, ((BaseUIVariableSelect*)pObject)->IsCurrentItemUnlocked() );
	
	return 1;
}

static S32 GetPosition(lua_State *pState)
{
	if(lua_type(pState,1) != LUA_TNUMBER)
	{
		g_pUILua->StackDump(pState,"GetPosition(button_id) - incorrect arguments");
		return 0;
	}

	C_GadgetBase *pObject=gButtons.Find((U32)lua_tonumber(pState,1));
	if(!pObject)
		return 0;

	lua_pushnumber( pState, (lua_Number)pObject->m_Position.x);
	lua_pushnumber( pState, (lua_Number)pObject->m_Position.y);
	lua_pushnumber( pState, (lua_Number)pObject->m_Position.z);

	return 3;
}

static S32 GetX( lua_State *pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "GetX(button_id) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase *pObject=gButtons.Find((U32)lua_tonumber(pState,1));
	if(!pObject)
		return 0;

	lua_pushnumber( pState, (lua_Number)pObject->m_Position.x );

	return 1;
}

static S32 GetY( lua_State *pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "GetY(button_id) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase *pObject=gButtons.Find((U32)lua_tonumber(pState,1));
	if(!pObject)
		return 0;

	lua_pushnumber( pState, (lua_Number)pObject->m_Position.y );

	return 1;
}

static S32 GetZ( lua_State *pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "GetZ(button_id) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase *pObject=gButtons.Find((U32)lua_tonumber(pState,1));
	if(!pObject)
		return 0;

	lua_pushnumber( pState, (lua_Number)pObject->m_Position.z );

	return 1;
}

static S32 GetPanel( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "GetPanel(number) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase *pObject=gButtons.Find((U32)lua_tonumber(pState,1));
	if(!pObject)
		return 0;

	lua_pushinteger( pState, (lua_Integer)pObject->m_Panel );
	return 1;
}

static S32 GetScale(lua_State* pState)
{
	if(lua_type(pState, 1) != LUA_TNUMBER)
	{
		g_pUILua->StackDump(pState,"GetScale(button_id) - incorrect arguments");
		return 0;
	}

	C_GadgetBase* pObject = gButtons.Find((U32)lua_tonumber(pState, 1));
	if(!pObject)
		return 0;

	lua_pushnumber( pState, (lua_Number)pObject->m_Scale.x);
	lua_pushnumber( pState, (lua_Number)pObject->m_Scale.y);
	lua_pushnumber( pState, (lua_Number)pObject->m_Scale.z);

	return 3;
}

static S32 GetSize( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "GetSize( button_id ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase* pObject = gButtons.Find( (S32)lua_tonumber( pState, 1 ) );
	if ( !pObject )
		return 0;

	lua_pushnumber( pState, (lua_Number)pObject->m_Size.x );
	lua_pushnumber( pState, (lua_Number)pObject->m_Size.y );
	lua_pushnumber( pState, (lua_Number)pObject->m_Size.z );

	return 3;
}

static S32 GetOrientation( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "GetOrientation( button_id ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase* pObject = gButtons.Find( (S32)lua_tonumber( pState, 1 ) );
	if ( !pObject )
		return 0;

	lua_pushnumber( pState, (lua_Number)pObject->m_Rotation.x );
	lua_pushnumber( pState, (lua_Number)pObject->m_Rotation.y );
	lua_pushnumber( pState, (lua_Number)pObject->m_Rotation.z );

	return 3;
}

static S32 GetAlpha( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "GetAlpha( button_id ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase* pObject = gButtons.Find( (S32)lua_tonumber( pState, 1 ) );
	if ( !pObject )
		return 0;

	lua_pushnumber( pState, (lua_Number)pObject->m_SpriteColours[ 0 ].A() );
	lua_pushnumber( pState, (lua_Number)pObject->m_SpriteColours[ 1 ].A() );
	lua_pushnumber( pState, (lua_Number)pObject->m_SpriteColours[ 2 ].A() );
	lua_pushnumber( pState, (lua_Number)pObject->m_SpriteColours[ 3 ].A() );

	return 4;
}

static S32 CanWeProcede(lua_State *pState)
{
	if(!lua_isnumber(pState,1))
	{
		g_pUILua->StackDump(pState,"CanWeProcede(Menu,num,ID) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase *pObject=gButtons.Find((U32)lua_tonumber(pState,1));
	if(!pObject)
		return 0;

	lua_pushboolean(pState,((BaseUIVariableSelect *) pObject)->IsCurrentItemUnlocked());

	return 1;
}

static S32 StoreString(lua_State*pState)
{
	if(!lua_isnumber(pState,1) || !lua_isstring(pState,2))
	{
		g_pUILua->StackDump(pState,"StoreString(idx,str) - incorrect arguments" );
		return 0;
	}
	const F32 f = (F32)lua_tonumber(pState,1);
	if(f < 0 || f >= C_StringStore::m_IndexStoreSize)
	{
		LOGFATAL( g_Log,  "Index %5.2f is out of bounds", f );
	}

	//printf("Storing String %s in %d\n",lua_tostring(pState,2),(U32)f);

	C_StringStore::Instance().Store((U32)f, lua_tostring(pState,2));

	return 0;
}

static S32 ChangeOrientation(lua_State *pState)
{
	if(!lua_isnumber(pState,1) )
	{
		g_pUILua->StackDump(pState,"ChangeOrientation(num,table) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase *pObject=gButtons.Find((U32)lua_tonumber(pState,1));
	if(!pObject)
		return 0;

	if( lua_type(pState,2) == LUA_TTABLE )
	{
		lua_settop(pState,2);

		Lua::GetDataMember( pState, "rotx", pObject->m_Rotation.x );
		Lua::GetDataMember( pState, "roty", pObject->m_Rotation.y );
		Lua::GetDataMember( pState, "rotz", pObject->m_Rotation.z );
	}
	else
	if( lua_type(pState,2) == LUA_TNUMBER && lua_type(pState,3) == LUA_TNUMBER && lua_type(pState,4) == LUA_TNUMBER )
	{
		pObject->m_Rotation.x = (XtF32)lua_tonumber( pState, 2 );
		pObject->m_Rotation.y = (XtF32)lua_tonumber( pState, 3 );
		pObject->m_Rotation.z = (XtF32)lua_tonumber( pState, 4 );
	}
	else
	{
		g_pUILua->StackDump(pState,"ChangeOrientation(num, [table] | [number,number,number]) - incorrect arguments");
	}

	return 0;
}

static S32 ChangeColour(lua_State *pState)
{
	if( lua_type(pState,1) != LUA_TNUMBER || lua_type(pState,2) != LUA_TSTRING )
	{
		g_pUILua->StackDump(pState,"ChangeColour(ID,colour) - incorrect arguments");
		return 0;
	}
	
	C_GadgetBase *pObject=gButtons.Find((U32)lua_tonumber(pState,1));
	if( !pObject )
		return 0;

	if(lua_isnumber(pState,3))
	{
		// Set specific
		U32 index = (U32)lua_tonumber(pState,3);
		if(index >= pObject->m_SpriteColours.m_SpriteColoursSize )
		{
			g_pUILua->StackDump(pState,"ChangeColour(ID,colour,index) - incorrect index");
			return 0;
		}
	
		pObject->m_SpriteColours[ index ].Set(lua_tostring(pState,2));
	}
	else
	{
		// Get valid style pointer
		const char *style = lua_tostring(pState,2);
		if( !style || !style[0] )
			return 0;
						
		// Don't set to the same colour
		if( pObject->m_ColourStyle && strcmp( pObject->m_ColourStyle, style ) == 0 )
			return 0;

		pObject->SetColourStyle( style );
		pObject->ResolveInheritedColours();
		pObject->DirtySet( C_GadgetBaseData::E_Dirty_Colour, true );
	}

	return 0;
}

// Swap matching colours in a heirarchy to a differenr colour
static S32 SwapColour(lua_State* pState )
{
	if( lua_type(pState,1) != LUA_TNUMBER || lua_type(pState,2) != LUA_TSTRING || lua_type(pState,3) != LUA_TSTRING )
	{
		g_pUILua->StackDump(pState,"SwapColour(number,string,string,[bool]) - incorrect arguments");
		return 0;
	}

	// Get object
	C_GadgetBase *pObject = gButtons.Find( (U32)lua_tonumber( pState, 1 ) );
	if( !pObject )
		return 0;

	// Get valid style pointer
	const char *matchingStyle = lua_tostring( pState, 2 );
	if( !matchingStyle || !matchingStyle[0] )
		return 0;


	// Get valid style pointer
	const char *swapStyle = lua_tostring( pState, 3 );
	if( !swapStyle || !swapStyle[0] )
		return 0;

	// Recurse all heirarchy?
	bool all = true;
	if( lua_type( pState, 4 ) == LUA_TBOOLEAN ) 
		all = (lua_toboolean( pState, 4 ) != 0);

	// Get children
	ButtonArray children;
	if( pObject->FindChildren( &children, all ) == 0 )
		return 0;

	for( S32 j = 0; j < children.Size(); j++ )
	{
		C_GadgetBase *pChild = children[ j ];

		// Matching?
		if( !pChild->m_ColourStyle || ( pChild->m_ColourStyle && strcmp( pChild->m_ColourStyle, matchingStyle ) != 0 ) )
			continue;

		// Change
		pChild->SetColourStyle( swapStyle );
		pChild->ResolveInheritedColours();
		pChild->DirtySet( C_GadgetBaseData::E_Dirty_Colour, true );
	}

	return 0;
}

static S32 ChangePivots(lua_State *pState)
{
#if 1
	assert( 0 );
#else
	if(!lua_isnumber(pState,1) || !lua_isnumber(pState,2) || !lua_isnumber(pState,3))
	{
		g_pUILua->StackDump(pState,"ChangePivots(ID,num,num,[num]) - incorrect arguments");
		return 0;
	}

	C_GadgetBase *pObject=gButtons.Find((U32)lua_tonumber(pState,1));
	if(!pObject)
		return 0;
	
	pObject->m_RotationPivot.x = (F32)lua_tonumber(pState,2);
	pObject->m_RotationPivot.y = (F32)lua_tonumber(pState,3);

	// Optional Z
	if(lua_type(pState,4) == LUA_TNUMBER)
		pObject->m_RotationPivot.z = (F32)lua_tonumber(pState,4);
#endif
	return 0;
}

static S32 ChangeScalePivots( lua_State* pState )
{
#if 1
	assert( 0 );
#else
	if ( ( lua_type( pState, 1 ) != LUA_TNUMBER ) || ( lua_type( pState, 2 ) != LUA_TNUMBER ) || ( lua_type( pState, 3 ) != LUA_TNUMBER ) )
	{
		g_pUILua->StackDump( pState, "ChangePivots( ID, scale_pivotx, scale_pivoty ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase* pElem = gButtons.Find( (S32)lua_tonumber( pState, 1 ) );

	if ( !pElem )
		return 0;
	
	pElem->mData.scale_pivotx = (F32)lua_tonumber( pState, 2 );
	pElem->mData.scale_pivoty = (F32)lua_tonumber( pState, 3 );
#endif
	return 0;
}

static S32 ChangeAlpha(lua_State *pState)
{
	if(!lua_isnumber(pState,1) || !lua_isnumber(pState,2))
	{
		g_pUILua->StackDump(pState,"ChangeAlpha(ID,alpha) - incorrect arguments");
		return 0;
	}
	
	C_GadgetBase *pObject=gButtons.Find((U32)lua_tonumber(pState,1));
	if(!pObject)
		return 0;

	pObject->m_Alpha = Maths::Clamp( (F32)lua_tonumber(pState,2), 0.0f, 1.0f );
	pObject->DirtySet( C_GadgetBaseData::E_Dirty_Alpha, true );

	return 0;
}

static S32 ChangeText(lua_State *pState)
{
	if(!lua_isnumber(pState,1) || (!lua_isnumber(pState,2) && !lua_isstring(pState,2)))
	{
		g_pUILua->StackDump(pState,"ChangeText(ID,[IDS|string]) - incorrect arguments");
		return 0;
	}
	
	C_GadgetBase *pObject=gButtons.Find((U32)lua_tonumber(pState,1));
	if( !pObject ) 
	{
		g_pUILua->StackDump(pState, "ChangeText() : Button ID not found\n");
		return 0;
	}

	E_UIButtonType type = (E_UIButtonType)pObject->GetType();
	if( type == E_UIButtonType_StaticText )
	{
		UIStaticText* stxt = (UIStaticText*)pObject;

		if( lua_type(pState,2) == LUA_TNUMBER )
			stxt->ChangeText( lua_tonumber( pState, 2 ) );
		else
		if( lua_type(pState,2) == LUA_TSTRING )
			stxt->ChangeText( lua_tostring( pState, 2 ) );
	}
	else
	{
		g_pUILua->StackDump(pState,"UIButtons.ChangeText() : You are trying to change the text of a button that doesn't have any.");
	}

	return 0;
}

static S32 LerpColour( lua_State* pState )
{
	if ( !lua_isstring( pState, 1 ) || !lua_isstring( pState, 2 ) || !lua_isnumber( pState, 3 ) )
	{
		g_pUILua->StackDump( pState, "SplitColourString( src, dest, f ) - incorrect arguments");
		return 0;
	}

	C_UIColour src;
	src.Set( lua_tostring( pState, 1 ) );
	
	C_UIColour dest;
	dest.Set( lua_tostring( pState, 2 ) );

	src.Lerp( src, dest, (F32)lua_tonumber( pState, 3 ) );

	lua_pushnumber( pState, src.A() );
	lua_pushnumber( pState, src.R() );
	lua_pushnumber( pState, src.G() );
	lua_pushnumber( pState, src.B() );

	return 4;
}

static S32 ChangeMouseBoundingBox(lua_State* pState)
{
	if(lua_type(pState,1)!=LUA_TNUMBER || lua_type(pState,2)!=LUA_TNUMBER || lua_type(pState,3)!=LUA_TNUMBER || lua_type(pState,4)!=LUA_TNUMBER || lua_type(pState,5)!=LUA_TNUMBER)
	{
		g_pUILua->StackDump(pState,"ChangeMouseBoundingBox(number,number,number,number,number) - incorrect arguments");
		return 0;
	}

	C_GadgetBase *pObject=gButtons.Find((U32)lua_tonumber(pState,1));
	if(!pObject)
	{
		g_pUILua->StackDump(pState, "ChangeMouseBoundingBox() : Button ID not found\n");
		return 0;
	}

#ifdef PLATFORM_PC
	pObject->m_Mouse.clickable	= true;
	pObject->m_Mouse.bb_left		= (F32)lua_tonumber(pState,2);
	pObject->m_Mouse.bb_top		= (F32)lua_tonumber(pState,3);
	pObject->m_Mouse.bb_right	= (F32)lua_tonumber(pState,4);
	pObject->m_Mouse.bb_bottom	= (F32)lua_tonumber(pState,5);
#endif

	return 0;
}

static S32 GetScrollDelta(lua_State *pState)
{
	if(lua_type(pState,1)!=LUA_TNUMBER)
	{
		g_pUILua->StackDump(pState,"GetScrollDelta(num) - incorrect arguments");
		return 0;
	}

	C_GadgetBase *pObject = gButtons.Find((U32)lua_tonumber(pState,1));
	if(!pObject)
	{
		g_pUILua->StackDump(pState, "GetScrollDelta() : Button ID not found\n");
		return 0;
	}

	lua_pushnumber(pState,((BaseUIVariableSelect*)pObject)->GetDelta());

	return 1;
}

static S32 GetSliderValue( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "GetSliderValue( id ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase* pObject = gButtons.Find( (S32)lua_tonumber( pState, 1 ) );
	
	if ( !pObject )
		return 0;

	if ( (E_UIButtonType)pObject->GetType() != E_UIButtonType_Slider )
	{
		LOGERROR( g_Log, DBSTR "Try to get slider value from an pObject that's not a slider" );
		return 0;
	}

	lua_pushnumber( pState,((UISlider*)pObject)->Value() );

	return 1;
}

static S32 SetSliderValue( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "SetSliderValue( id, value ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase* pObject = gButtons.Find( (S32)lua_tonumber( pState, 1 ) );
	
	if ( !pObject )
		return 0;

	if ( (E_UIButtonType)pObject->GetType() != E_UIButtonType_Slider )
	{
		LOGERROR( g_Log, DBSTR "Trying to set slider value of an pObject that's not a slider" );
		return 0;
	}
	
	((UISlider*)pObject)->Value( (F32)lua_tonumber( pState, 2 ) );

	return 0;
}

static S32 SetZEnable( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TBOOLEAN )
	{
		LOGERROR( g_Log, DBSTR "SetZEnableOff expected a bool" );
		return 0;
	}

	bool enable = lua_toboolean( pState, 1 ) ? true : false;
	gSimple2D.SetZEnable( enable );

	return 0;
}


static S32 AddSlot( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		LOGERROR( g_Log, DBSTR "Expected gadget id" );
		return 0;
	}

	C_GadgetBase* pObject = gButtons.Find( (S32)lua_tonumber( pState, 1 ) );

	if ( !pObject )
	{
		LOGWARN( g_Log, DBSTR" : Button ID not found\n");
		return 0;
	}
	if ( (E_UIButtonType)pObject->GetType() == E_UIButtonType_ItemSpinner )
	{
		lua_pushnumber( pState, ((UIItemSpinner*)pObject)->AddSlot() );

		return 1;
	}

	return 0;
}

static S32 AttachToSlot( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER || lua_type( pState, 3 ) != LUA_TNUMBER )
	{
		LOGERROR( g_Log, DBSTR "Expected (num,id) (num,id) (num,slot)" );
		return 0;
	}

	C_GadgetBase* itemBase = gButtons.Find( (S32)lua_tonumber( pState, 1 ) );
	C_GadgetBase* slotBase = gButtons.Find( (S32)lua_tonumber( pState, 2 ) );

	if ( !itemBase || !slotBase )
	{
		LOGWARN( g_Log, DBSTR" : Button ID not found\n");
		return 0;
	}
	if ( (E_UIButtonType)slotBase->GetType() == E_UIButtonType_ItemSpinner )
	{
		S32 slot = (S32)lua_tonumber( pState, 3 );
		((UIItemSpinner*)slotBase)->AddItem( slot, itemBase );
	}

	return 0;
}

static S32 ResetTextPrint( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "ResetTextPrint( id ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase* pObject = gButtons.Find( (S32)lua_tonumber( pState, 1 ) );

	if ( !pObject )
		return 0;

	if ( (E_UIButtonType)pObject->GetType() == E_UIButtonType_StaticText )
		((UIStaticText*)pObject)->ResetTextPrint();
	
	return 0;
}

static S32 SkipTextPrint( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "SkipTextPrint( id ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase* pObject = gButtons.Find( (S32)lua_tonumber( pState, 1 ) );

	if ( !pObject )
		return 0;

	if ( (E_UIButtonType)pObject->GetType() == E_UIButtonType_StaticText )
		((UIStaticText*)pObject)->SkipTextPrint();
	
	return 0;
}

static S32 TimeLineActive( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TSTRING )
	{
		g_pUILua->StackDump( pState, "TimeLineActive{ string,[bool],[number],[bool],[number] ) - incorrent arguments" );
		return 0;
	}

	const char* pString = lua_tostring( pState, 1 );
	bool bActive = ( lua_type( pState, 2 ) == LUA_TBOOLEAN ) && ( lua_toboolean( pState, 2 ) != 0 );

	C_UiTimelineMessage message( pString, bActive );

	if ( lua_type( pState, 3 ) == LUA_TNUMBER )
		message.m_fTime = (F32)lua_tonumber( pState, 3 );

	if ( lua_type( pState, 4 ) == LUA_TBOOLEAN )
		message.m_Reset = (lua_toboolean( pState, 4 ) != 0);

	// Optional mask param
	int mask = 0xffffffff;
	if( lua_type( pState, 5 ) == LUA_TNUMBER )
	{
		mask = 1 << ((U32)lua_tonumber( pState, 5 ));
	}

	gButtons.BroadcastMessageNow( mask, &message );
	
	return 0;
}


static S32 PrivateTimeLineActive( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TSTRING )
	{
		g_pUILua->StackDump( pState, "PrivateTimeLineActive{ id(num), name(string), [active(bool)] ) - incorrent arguments" );
		return 0;
	}

	U32 id = (U32)lua_tonumber( pState, 1 );
	C_GadgetBase* pObject = gButtons.Find( id );

	if ( pObject )
	{
		const char* pString = lua_tostring( pState, 2 );
		bool active = ( lua_type( pState, 3 ) == LUA_TBOOLEAN ) && ( lua_toboolean( pState, 3 ) != 0 );
		F32 time = ( lua_type( pState, 4 ) == LUA_TNUMBER ) ? (F32)lua_tonumber( pState, 4 ) : -1.0f;
		bool reset = ( lua_type( pState, 5 ) == LUA_TBOOLEAN ) && ( lua_toboolean( pState, 5 ) != 0 );

		pObject->SendPrivateTimelineMessaage( pString, active, time, reset );
	}
	return 0;
}


/*---------------------------------------------------------------------------------------
	Scrollbar Manipulation functions.
---------------------------------------------------------------------------------------*/
static S32 SetSliderPosition( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "SetSliderPosition( id, position ) - incorrect arguments" );
		return 0;
	}

	S32 id = (S32)lua_tonumber( pState, 1 );
	F32 position = (F32)lua_tonumber( pState, 2 );

	C_GadgetBase* pObject = gButtons.Find( id );

	if ( pObject && pObject->GetType() == E_UIButtonType_ScrollBar )
	{
		((UIScrollBar*)pObject)->SetSliderPosition( position );
	}
	else
	{
		LOGWARN( g_Log, DBSTR "Gadget not found!" );
	}
	return 0;
}

// SetAlphaIntensity
static S32 SetAlphaIntensity( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "SetAlphaIntensity( number, [number], [number] ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase* pObject = gButtons.Find( (S32)lua_tonumber( pState, 1 ) );

	if ( !pObject )
		return 0;

	if ( lua_type( pState, 2 ) == LUA_TNUMBER )
	{
		pObject->m_Alpha = (F32)lua_tonumber( pState, 2 );
		pObject->DirtySet( C_GadgetBaseData::E_Dirty_Alpha, true );
	}

	if ( lua_type( pState, 3 ) == LUA_TNUMBER )
	{
		pObject->m_Intensity = (F32)lua_tonumber( pState, 3 );
		pObject->DirtySet( C_GadgetBaseData::E_Dirty_Colour, true );
	}

	// re-resolve the colour
	pObject->ResolveColourStyle( pObject->m_Intensity, pObject->m_Alpha );

	return 0;
}

static S32 RemoveSubscriber( lua_State* pState ) 
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "RemoveSubscriber( number, number ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase* pObject = gButtons.Find( (S32)lua_tonumber( pState, 1 ) );
	if( !pObject )
		return 0;

	E_UiMsg msg = (E_UiMsg)(S32)lua_tonumber(pState,2);

	bool success = pObject->RemoveSubscriber( msg );

	lua_pushboolean( pState, success ? 1 : 0 );
	return 0;
}


static int RenderBranchScan( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "RenderBranchScan( number ) - incorrect arguments" );
		return 0;
	}

	// Get gadget
	C_GadgetBase *pObject = gButtons.Find( (S32)lua_tonumber(pState,1) );
	if( !pObject )
		return 0;

	UIButtonTypeID type = E_UIButtonType_RenderBranch;
	// Type check against something else?
	if( lua_type( pState, 2 ) == LUA_TNUMBER )
		type = (UIButtonTypeID)(S32)lua_tonumber(pState,2);

	// Check type
	if( pObject->GetType() !=  type )
	{
		g_pUILua->StackDump( pState, "RenderBranchScan - gadget type not a branch" );
		return 0;
	}

	UIRenderBranch* pBranch = (UIRenderBranch*)pObject;
	pBranch->Scan();

	return 0;
}


static int SetWaveFrequency( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "SetWaveFrequency( id, number ) - incorrect arguments" );
		return 0;
	}

		// Get gadget
	C_GadgetBase *pObject = gButtons.Find( (U32)lua_tonumber(pState,1) );
	if( pObject )
	{
		if ( pObject->GetType() == E_UIButtonType_AdobeTrail )
			((UIAdobeTrail*)pObject)->Frequency( (F32)lua_tonumber(pState,2) );
	}
	return 0;
}


static int SetWaveIntensity( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "SetWaveIntensity( id, number ) - incorrect arguments" );
		return 0;
	}

		// Get gadget
	C_GadgetBase *pObject = gButtons.Find( (U32)lua_tonumber(pState,1) );
	if( pObject )
	{
		if ( pObject->GetType() == E_UIButtonType_AdobeTrail )
			((UIAdobeTrail*)pObject)->Intensity( (F32)lua_tonumber(pState,2) );
	}
	return 0;
}


static int SetWaveSpeed( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "SetWaveSpeed( id, number ) - incorrect arguments" );
		return 0;
	}

		// Get gadget
	C_GadgetBase *pObject = gButtons.Find( (U32)lua_tonumber(pState,1) );
	if( pObject )
	{
		if ( pObject->GetType() == E_UIButtonType_AdobeTrail )
			((UIAdobeTrail*)pObject)->Speed( (F32)lua_tonumber(pState,2) );
	}
	return 0;
}


static int AddListItem( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "AddListItem( num(id), num(id) ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase *pObject = gButtons.Find( (U32)lua_tonumber( pState, 1 ) );
	C_GadgetBase *pItem = gButtons.Find( (U32)lua_tonumber( pState, 2 ) );

	if ( pObject && pItem )
	{
		if ( pObject->GetType() == E_UIButtonType_NodeList && pItem->GetType() == E_UIButtonType_Node )
		{
			if ( lua_type( pState, 3 ) == LUA_TNUMBER && lua_type( pState, 4 ) == LUA_TBOOLEAN )
			{
				((UINodeList*)pObject)->AddNode( (UINode*)pItem, (F64)lua_tonumber( pState, 3 ), lua_toboolean( pState, 4 ) ? true : false );
			}
			else
			if ( lua_type( pState, 3 ) == LUA_TNIL && lua_type( pState, 4 ) == LUA_TBOOLEAN )
			{
				((UINodeList*)pObject)->AddNode( (UINode*)pItem, lua_toboolean( pState, 4 ) ? true : false );
			}
			else
			if ( lua_type( pState, 3 ) == LUA_TNUMBER )
			{
				((UINodeList*)pObject)->AddNode( (UINode*)pItem, (F64)lua_tonumber( pState, 3 ), false );
			}
			else
				((UINodeList*)pObject)->AddNode( (UINode*)pItem );
		}
	}
	return 0;
}


static S32 OpenCurrentItem( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "OpenCurrentItem( id ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase *pObject = gButtons.Find( (U32)lua_tonumber( pState, 1 ) );

	if ( pObject->GetType() == E_UIButtonType_NodeList )
	{
		((UINodeList*)pObject)->OpenCurrentItem();
	}

	return 0;
}


static S32 CloseCurrentItem( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "CloseCurrentItem( id ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase *pObject = gButtons.Find( (U32)lua_tonumber( pState, 1 ) );

	if ( pObject->GetType() == E_UIButtonType_NodeList )
	{
		((UINodeList*)pObject)->CloseCurrentItem();
	}

	return 0;
}


static int NodeListScan( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "NodeListScan( num(id) ) - incorrect arguments" );
		return 0;
	}

	// optional snap param
	bool bSnap = ( lua_type( pState, 2 ) == LUA_TBOOLEAN ) ? ( lua_toboolean( pState, 2 ) != 0 ) : false;

	// Get gadget
	C_GadgetBase *pObject = gButtons.Find( (U32)lua_tonumber( pState, 1 ) );
	if( pObject )
	{
		if ( pObject->GetType() == E_UIButtonType_NodeList )
		{
			((UINodeList*)pObject)->ScanForNodes( bSnap );
		}
	}
	return 0;
}


static int SetNodeListStyle( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "SetNodeListStyle( num(id), num(style) ) - incorrect arguments" );
		return 0;
	}

	// Get gadget
	C_GadgetBase *pObject = gButtons.Find( (U32)lua_tonumber( pState, 1 ) );
	if( pObject )
	{
		if ( pObject->GetType() == E_UIButtonType_NodeList )
		{
			((UINodeList*)pObject)->SetListStyle( (UINodeList::E_ListStyle)(S32)lua_tonumber( pState, 2 ) );
		}
	}
	return 0;
}

static int SetNodeValue( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "SetNodeValue( number, number ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase *pObject = gButtons.Find( (U32)lua_tonumber( pState, 1 ) );
	if ( pObject && pObject->GetType() == E_UIButtonType_Node )
		((UINode*)pObject)->m_Value = (F64)lua_tonumber( pState, 2 );

	return 0;
}

static int GetNodeID( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "GetNodeID( number, number ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase *pObject = gButtons.Find( (U32)lua_tonumber( pState, 1 ) );
	if( !pObject )
		return 0;
	
	U32 nodeID = 0;
	if ( pObject->GetType() == E_UIButtonType_NodeList )
	{
		const UINode* pNode = ((UINodeList*)pObject)->GetNodeByIndex( lua_tointeger( pState, 2 ) );
		if( pNode )
			nodeID = pNode->m_ID.Number();
	}
	else
	if ( pObject->GetType() == E_UIButtonType_Table )
	{
		const UINode* pNode = ((UITable*)pObject)->GetNodeByValue( lua_tointeger( pState, 2 ) );
		if( pNode )
			nodeID = pNode->m_ID.Number();
	}

	lua_pushinteger( pState, nodeID );
	return 1;
}

static int DummyLerpMatrixCopy( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || (lua_type( pState, 2 ) != LUA_TNUMBER && lua_type( pState, 2 ) != LUA_TSTRING ) )
	{
		g_pUILua->StackDump( pState, "GetNodeID( number, number|string ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase *pSrcObject = gButtons.Find( (U32)lua_tonumber( pState, 1 ) );
	if( !pSrcObject )
		return 0;
	if( pSrcObject->GetType() != E_UIButtonType_DummyLerp )
		return 0;

	C_GadgetBase *pObject = NULL;
	if( lua_type( pState, 2 ) == LUA_TNUMBER  )
		pObject = gButtons.Find( (U32)lua_tonumber( pState, 2 ) );
	else
	if( lua_type( pState, 2 ) == LUA_TSTRING )
		pObject = gButtons.FindByName( lua_tostring( pState, 2 ) );

	if( !pObject )
		return 0;

	Maths::HMat43 mat;

	for( C_GadgetBase* p = pObject; p; p = p->Parent() )
		p->DirtySet( C_GadgetBaseData::E_Dirty_Matrix, true );

	pObject->CalcTransformHierarchy( mat );

	((UIDummyLerp*)pSrcObject)->CopyMatrix( mat );
	return 0;
}

static int SetNodeItemLocked( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER || lua_type( pState, 3 ) != LUA_TBOOLEAN )
	{
		g_pUILua->StackDump( pState, "SetNodeItemLocked( num(id), num(index), bool(locked) ) - incorrect arguments" );
		return 0;
	}

	// Get gadget
	C_GadgetBase *pObject = gButtons.Find( (U32)lua_tonumber( pState, 1 ) );
	if( pObject )
	{
		if ( pObject->GetType() == E_UIButtonType_NodeList )
		{
			bool bByValue = ( lua_type( pState, 4 ) == LUA_TBOOLEAN ) ? ( lua_toboolean( pState, 4 ) != 0 ) : false;

			if ( bByValue )
				((UINodeList*)pObject)->SetItemLockedByValue( (F64)lua_tonumber( pState, 2 ), ( lua_toboolean( pState, 3 ) ? true : false ) );
			else
				((UINodeList*)pObject)->SetItemLocked( (U32)lua_tonumber( pState, 2 ), ( lua_toboolean( pState, 3 ) ? true : false ) );
		}
	}
	return 0;
}

static int LockNode( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "LockNode( num(id) ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase *pObject = gButtons.Find( (U32)lua_tonumber( pState, 1 ) );
	if ( pObject && pObject->GetType() == E_UIButtonType_Node )
		((UINode*)pObject)->m_Locked = true;
	
	return 0;
}

static int UnlockNode( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "UnlockNode( num(id) ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase *pObject = gButtons.Find( (U32)lua_tonumber( pState, 1 ) );
	if ( pObject && pObject->GetType() == E_UIButtonType_Node )
		((UINode*)pObject)->m_Locked = false;
	
	return 0;
}

// SetNodeSortKey
static S32 SetNodeSortKey( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER || lua_type( pState, 3 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "SetNodeItemLocked( num(id), num(index/value), num(sortKey), [bool(byValue)] ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase *pObject = gButtons.Find( (U32)lua_tointeger( pState, 1 ) );

	if ( !pObject || ( pObject->GetType() != E_UIButtonType_NodeList ) )
		return 0;

	S32 sSortKey = (S32)lua_tonumber( pState, 3 );
	bool bByValue = ( ( lua_type( pState, 4 ) == LUA_TBOOLEAN ) && ( lua_toboolean( pState, 4 ) != 0 ) );

	if ( bByValue )
		((UINodeList*)pObject)->SetItemSortKeyByValue( (F64)lua_tonumber( pState, 2 ), sSortKey );
	else	
		((UINodeList*)pObject)->SetItemSortKey( (U32)lua_tointeger( pState, 2 ), sSortKey );

	return 0;
}

// NodeListSort
static S32 NodeListSort( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "NodeListSort( id ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase* pObject = gButtons.Find( (U32)lua_tointeger( pState, 1 ) );

	if ( pObject && pObject->GetType() == E_UIButtonType_NodeList )
		((UINodeList*)pObject)->Sort();
	
	return 0;
}

static int GetStaticTextLength( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "GetStaticTextLength( num(id) ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase* pObject = gButtons.Find( (U32)lua_tonumber( pState, 1 ) );

	if( pObject && pObject->GetType() == E_UIButtonType_StaticText )
	{
		bool bUpdateLengths = ( ( lua_type( pState, 2 ) == LUA_TBOOLEAN ) && ( lua_toboolean( pState, 2 ) != 0 ) );
		const F32 length = ((UIStaticText*)pObject)->GetStringLength( bUpdateLengths );

		lua_pushnumber( pState, length );

		return 1;
	}
	return 0;
}

static int GetStaticTextHeight( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "GetStaticTextHeight( num(id) ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase* pObject = gButtons.Find( (U32)lua_tonumber( pState, 1 ) );

	if( pObject && pObject->GetType() == E_UIButtonType_StaticText )
	{
		const F32 length = ((UIStaticText*)pObject)->GetStringHeight();

		lua_pushnumber( pState, length );

		return 1;
	}
	return 0;
}

// AddGraphData
static S32 AddGraphData( lua_State* pState )
{
	if ( ( lua_type( pState, 1 ) != LUA_TNUMBER ) || ( lua_type( pState, 2 ) != LUA_TNUMBER ), ( lua_type( pState, 3 ) != LUA_TNUMBER ) )
	{
		g_pUILua->StackDump( pState, "AddGraphData( id, num, num ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase* pObject = gButtons.Find( (U32)lua_tonumber( pState, 1 ) );

	if ( pObject && ( pObject->GetType() == E_UIButtonType_Graph ) )
		((UIGraph*)pObject)->AddData( (F32)lua_tonumber( pState, 2 ), (F32)lua_tonumber( pState, 3 ) );

	return 0;
}

static S32 SetProgressBarPosition( lua_State* pState )
{
	if ( ( lua_type( pState, 1 ) != LUA_TNUMBER ) || ( lua_type( pState, 2 ) != LUA_TNUMBER ) )
	{
		g_pUILua->StackDump( pState, "SetProgressBarPosition( id, num ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase* pObject = gButtons.Find( (U32)lua_tonumber( pState, 1 ) );

	if ( pObject && pObject->GetType() == E_UIButtonType_ProgressBar )
	{
		((UIProgressBar*)pObject)->SetPosition( (F32)lua_tonumber( pState, 2 ) );
	}
	return 0;
}

// ReplaceTimeLineLabel
static S32 ReplaceTimeLineLabel( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TSTRING || lua_type( pState, 3 ) != LUA_TSTRING )
	{
		g_pUILua->StackDump( pState, "ReplaceTimeLineLabel( id, string, string ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase* pObject = gButtons.Find( (U32)lua_tonumber( pState, 1 ) );
	const char* pName = lua_tostring( pState, 2 );
	const char* pNewName = lua_tostring( pState, 3 );

	if ( !pObject || !pName || !pNewName )
		return 0;

	for ( S32 i = 0; i < pObject->m_Timeline.count; i++ )
	{
		if ( !pObject->m_Timeline[ i ].m_Name || !pObject->m_Timeline[ i ].m_Name[ 0 ] )
			continue;

		if ( !strcmp( pObject->m_Timeline[ i ].m_Name.Text(), pName ) )
			pObject->ReplaceTimeLineLabel( i, pNewName );
	}

	return 0;
}

// StartKeyboard
static S32 StartKeyboardUI( lua_State* pState )
{
	S32 sMaxCharacters = ( ( lua_type( pState, 1 ) == LUA_TNUMBER ) ? (S32)lua_tonumber( pState, 1 ) : -1 );

	const WCHAR* pString = NULL;

	if ( lua_type( pState, 2 ) == LUA_TNUMBER )
		pString = C_TextStringID( lua_tonumber( pState, 2 ) ).String();

	else if ( lua_type( pState, 2 ) == LUA_TSTRING )
		pString = C_UIDataFinder::Instance().GetVariableText( lua_tostring( pState, 2 ) );

	bool bHighlight = ( (lua_type( pState, 3 ) == LUA_TBOOLEAN ) ? ( lua_toboolean( pState, 3 ) != 0 ) : false );

	C_UIKeyboard::Instance().Start( sMaxCharacters, pString, bHighlight );

	return 0;
}

static S32 AddTableCol( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "AddTableCol( id ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase* pTableObject = gButtons.Find( (U32)lua_tonumber( pState, 1 ) );

	if ( pTableObject )
	{
		if ( pTableObject->GetType() == E_UIButtonType_Table )
		{
			((UITable*)pTableObject)->AddColumn();
		}
	}

	return 0;
}

static S32 AddTableRow( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "AddTableCol( id, id ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase* pTableObject = gButtons.Find( (U32)lua_tonumber( pState, 1 ) );
	C_GadgetBase* pNodeObject = gButtons.Find( (U32)lua_tonumber( pState, 2 ) );

	const F64 value = ( lua_type( pState, 3 ) == LUA_TNUMBER ) ? lua_tonumber( pState, 3 ) : 0.0f;

	if ( pTableObject && pNodeObject )
	{
		if ( pTableObject->GetType() == E_UIButtonType_Table && pNodeObject->GetType() == E_UIButtonType_Node )
		{
			((UITable*)pTableObject)->AddRow( (UINode*)pNodeObject, value );
		}
	}

	return 0;
}

static S32 ActivateTableElement( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TBOOLEAN )
	{
		g_pUILua->StackDump( pState, "ActivateTableElement( id, bool ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase* pTableObject = gButtons.Find( (U32)lua_tonumber( pState, 1 ) );
	if ( pTableObject )
	{
		if ( pTableObject->GetType() == E_UIButtonType_Table )
		{
			((UITable*)pTableObject)->ActivateElement( ( lua_toboolean( pState, 2 ) ? true : false ) );
		}
	}

	return 0;
}

static S32 TableElementActivated( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "TableElementActivated( id ) - incorrect arguments" );
		return 0;
	}

	bool active = false;
	C_GadgetBase* pTableObject = gButtons.Find( (U32)lua_tonumber( pState, 1 ) );
	if ( pTableObject && pTableObject->GetType() == E_UIButtonType_Table )
	{
		active = ((UITable*)pTableObject)->ElementActivated();
	}

	lua_pushboolean( pState, active );
	return 1;
}

static S32 GetTableSelection( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "GetTableSelection( id ) - incorrect arguments" );
		return 0;
	}

	S32 value = 0;
	C_GadgetBase* pTableObject = gButtons.Find( (U32)lua_tonumber( pState, 1 ) );
	if ( pTableObject && pTableObject->GetType() == E_UIButtonType_Table )
	{
		value = (S32)((UITable*)pTableObject)->GetSelection();
	}

	lua_pushnumber( pState, value );
	return 1;
}

static S32 SetTableSelection( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "SetTableSelection( id ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase* pTableObject = gButtons.Find( (U32)lua_tonumber( pState, 1 ) );
	if ( pTableObject && pTableObject->GetType() == E_UIButtonType_Table )
	{
		((UITable*)pTableObject)->SetSelection( (F64)lua_tonumber( pState, 2 ) );
	}
	return 0;
}

static const luaL_reg button_lib[] = 
{
	{"AddButton",				AddButton},
	{"AddColumn",				AddColumn},
	{"AddEffect",				AddEffect},
	{"AddExtraParams",			AddExtraParams},
	{"AddGraphData",			AddGraphData },
	{"AddItem",					AddItem},
	{"AddItemHelp",				AddItemHelp},
	{"AddListItem",				AddListItem},
	{"AddTexture",				AddTexture},
	{"AddTimer",				AddTimer},
	{"AddSpacer",				AddSpacer},

	{"AddTableCol",				AddTableCol},
	{"AddTableRow",				AddTableRow},
	{"ActivateTableElement",	ActivateTableElement},
	{"TableElementActivated",	TableElementActivated},
	{"GetTableSelection",		GetTableSelection},
	{"SetTableSelection",		SetTableSelection},

	{"ChangeAlpha",				ChangeAlpha},
	{"ChangeButton",			ChangeButton},
#ifdef _UI_AUTO_CLIPWINDOW
	{"ChangeClipWindow",		ChangeClipWindow},
#endif
	{"ChangeColour",			ChangeColour},
	{"ChangeEffect",			ChangeEffect},
	{"ChangeEffectIndex",		ChangeEffectIndex},
	{"ChangeJustification",		ChangeJustification},
	{"ChangeLayer",				ChangeLayer},
	{"ChangePanel",				ChangePanel},
	{"ChangeMouseBoundingBox",	ChangeMouseBoundingBox},
	{"ChangeMouseClickable",	ChangeMouseClickable},
	{"ChangeOrientation",		ChangeOrientation},
	{"ChangePivots",  			ChangePivots},
	{"ChangePosition",			ChangePosition},
	{"ChangeScale",				ChangeScale},
	{"ChangeScalePivots",		ChangeScalePivots},
	{"ChangeSize",				ChangeSize},
	{"ChangeSizeX",				ChangeSizeX},
	{"ChangeText",				ChangeText},
	{"ChangeTexture",			ChangeTexture},
	{"ChangeTextureUV",			ChangeTextureUV},

	{"GetAlpha",				GetAlpha},
	{"GetOrientation",			GetOrientation},
	{"GetPosition",				GetPosition},
	{"GetScale",				GetScale},
	{"GetPanel",				GetPanel},
	{"GetScrollDelta",			GetScrollDelta},
	{"GetSelection",			GetSelection},
	{"GetSelectionIndex",		GetSelectionIndex},
	{"GetSize",					GetSize},
	{"GetSliderValue",			GetSliderValue },
	{"GetStaticTextLength",		GetStaticTextLength},
	{"GetStaticTextHeight",		GetStaticTextHeight},
	{"GetX",					GetX},
	{"GetY",					GetY},
	{"GetZ",					GetZ},

	{"IsActive",				IsActive},
	{"IsCurrentItemUnlocked",	IsCurrentItemUnlocked},
	{"IsSelected",				IsSelected},

	{"SetActive",				SetActive},
	{"SetAlphaIntensity",		SetAlphaIntensity},
	{"SetMenuItemActive",		SetMenuItemActive},
	{"SetNodeListStyle",		SetNodeListStyle},
	{"SetPad",					SetPad},
	{"SetParent",				SetParent},
	{"SetProgressBarPosition",	SetProgressBarPosition},
	{"SetSelected",				SetSelected},
	{"SetSelection",			SetSelection},
	{"SetSelectionByIndex",		SetSelectionByIndex},
	{"SetSliderValue",			SetSliderValue},
	{"SetSliderPosition",		SetSliderPosition},
	{"SetTimerStart",			SetTimerStart},
	{"SetTimerDuration",		SetTimerDuration},
	{"SetWaveFrequency",		SetWaveFrequency},
	{"SetWaveIntensity",		SetWaveIntensity},
	{"SetWaveSpeed",			SetWaveSpeed},
	{"SetZEnable",				SetZEnable},

	{"CanWeProcede",			CanWeProcede},
	{"ClearItems",				ClearItems},
	{"LerpColour",				LerpColour},
	{"NodeListScan",			NodeListScan},
	{"RefreshItems",			RefreshItems},
	{"RemoveSubscriber",		RemoveSubscriber},
	{"RenderBranchScan",		RenderBranchScan},
	{"ResetCurrentItem",		ResetCurrentItem},
	{"StoreString",				StoreString},
	{"SwapColour",				SwapColour},

	{"SetTimer",				UIButtons_SetTimer},
	{"GetTimer",				UIButtons_GetTimer},

	// static text print-out stuff
	{ "ResetTextPrint",			ResetTextPrint },
	{ "SkipTextPrint",			SkipTextPrint },

	// Timeline functions
	{ "TimeLineActive",			TimeLineActive },
	{ "PrivateTimeLineActive",	PrivateTimeLineActive },

	{ "CloneXtGadgetByName",	CloneXtGadgetByName },
	{ "FindChildByName",		FindChildByName },
	{ "CloseCurrentItem",		CloseCurrentItem },
	{ "OpenCurrentItem",		OpenCurrentItem },

	{ "ReplaceTimeLineLabel",	ReplaceTimeLineLabel },
	{ "SetXtVar",				SetXtVar },
	{ "GetXtVar",				GetXtVar },

	{ "SetNodeItemLocked",		SetNodeItemLocked },
	{ "SetNodeValue",			SetNodeValue },
	{ "GetNodeID",				GetNodeID },
	{ "LockNode",				LockNode },
	{ "UnlockNode",				UnlockNode },
	{ "SetNodeSortKey",			SetNodeSortKey },
	{ "NodeListSort",			NodeListSort },

	{ "DummyLerpMatrixCopy",	DummyLerpMatrixCopy },

	{ "StartKeyboard",			StartKeyboardUI },	// UIHardware already has StartKeyboard

	{NULL,			NULL}		// end of list
};

LUALIB_API S32 luaopen_buttons (lua_State *pState)
{
	luaL_openlib(pState,"UIButtons",button_lib,0);

	return 1;
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on) 
#endif
