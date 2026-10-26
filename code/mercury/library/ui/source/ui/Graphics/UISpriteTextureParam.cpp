#include "stdafx.h"

#include "lua_shared/LuaHelper.h"
#include "ui/Graphics/UISpriteTextureParam.h"
#include "ui/Graphics/Simple2d.h"
#include "ui/Graphics/UITextureManager.h"
#include "ui/Game/UiXt.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

// XT Definition
XT_TYPE( UI::C_TextureParam::C_Vector2UV, "UIVector2UV" )
	XT_FIELD( u, "u" )
	XT_FIELD( v, "v" )
XT_END_TYPE

XT_TYPE( UI::C_TextureParam, "UITextureParam" )
	XT_CALLBACK( UI::C_TextureParam::SorceryCallback )
	XT_FIELD( m_Name, "filename" )
	XT_FIELD( m_TextureID, "name" ) // Grr! For old times sake
	XT_FIELD( m_Position, "pos" )
	XT_FIELD( m_Size, "size" )
XT_END_TYPE

namespace UI
{

void C_TextureParam::SorceryCallback( XtEventType type, XtRecord* pRecord, C_TextureParam* pBase, XtCallbackContext* pContext )
{
	switch ( type )
	{
	case kXtEtConstruct:
		break;

	case kXtEtDestruct:
		break;

	case kXtEtUser :
		// Animating filename support
		switch( pContext->tag )
		{
		case UiXt::E_CallbackContext_Setup:
			{
				assert( pContext );
				UiXt::C_CallbackAssignContext *p = (UiXt::C_CallbackAssignContext *)pContext;
				
				if( stricmp( p->m_pLink->m_pComponentName, "filename" ) == 0 )
					pContext->tag = E_CallbackContext_Bind;
			}
			break;

		case E_CallbackContext_Bind:
			pBase->Bind( );
			break;
		}
	}
}

void C_TextureParam::Bind( )
{
	if( !m_Name || !m_Name[0] )
		return;

	const C_TextureHandleIndex handle = gSimple2D.AlternateTextureFindByName( m_Name );
	if( handle.Active() )
	{
		m_TextureID = (S32)handle;
	}
	else
	{
		m_TextureID = gUITextureManager.GetTextureByName( m_Name );
	}
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on) 
#endif
