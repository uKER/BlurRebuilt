#include "Stdafx.h"

#include "VinylEditorDataFinder.h"
#include "ui/Game/Gadgets/VinylManager.h"


#include <ui/Graphics/Localisation/Loc.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace UI
{

void C_VinylEditorDataFinder::Init()
{
	//
}

const WCHAR* C_VinylEditorDataFinder::GetVariableText( const char* pVariableName )
{
	const char* p = pVariableName;
	const U32 len = strlen( "SELECTED_" );
	if( strncmp( p, "SELECTED_", len ) == 0 )
	{
		p += len;

		VinylItem* item = C_VinylManager::Instance().scList().At( C_VinylManager::Instance().scCurrent() );
		if( !p )
			return L"<NONE_SELECTED>";

		if( !strcmp( p, "TRANS_X" ) )
		{
			m_VariableText.Format( L"X - %.1f", item->PosX() );
		}
		else
		if( !strcmp( p, "TRANS_Y" ) )
		{
			m_VariableText.Format( L"Y - %.1f", item->PosY() );
		}
		else
		if( !strcmp( p, "SCALE_X" ) )
		{
			m_VariableText.Format( L"X - %.1f", item->ScaleX() );
		}
		else
		if( !strcmp( p, "SCALE_Y" ) )
		{
			m_VariableText.Format( L"Y - %.1f", item->ScaleY() );
		}
		else
		if( !strcmp( p, "ROTATION" ) )
		{
			m_VariableText.Format( L"Angle - %.1f", item->Rotation() );
		}
		else
		if( !strcmp( p, "SKEW_X" ) )
		{
			m_VariableText.Format( L"%.1f", item->ShearX() );
		}
		else
		if( !strcmp( p, "SKEW_Y" ) )
		{
			m_VariableText.Format( L"%.1f", item->ShearY() );
		}
		else
		if( !strcmp( p, "GRD_TRANS" ) )
		{
			m_VariableText.Format( L"%.1f", item->ColourPos() );
		}
		else
		if( !strcmp( p, "GRD_SCALE" ) )
		{
			m_VariableText.Format( L"%.1f", item->ColourScale() );
		}
		else
		if( !strcmp( p, "GRD_ROT" ) )
		{
			m_VariableText.Format( L"%.1f", item->ColourRotation() );
		}		
		else
		if( !strcmp( p, "ALPHA0" ) )
		{
			m_VariableText.Format( L"%.1f", item->Alpha0() * 100.0f );
		}
		else
		if( !strcmp( p, "ALPHA1" ) )
		{
			m_VariableText.Format( L"%.1f", item->Alpha1() * 100.0f );
		}
	}
	else
	{
		const U32 len = strlen( "MANAGER_" );
		if( strncmp( p, "MANAGER_", len ) == 0 )
		{
			p += len;

			if( !strcmp( p, "FREE" ) )
			{
				m_VariableText.Format( L"Free - %3d", C_VinylManager::Instance().scList().FreeCount() );
			}
			else
			if( !strcmp( p, "RENDER" ) )
			{
				m_VariableText.Format( L"Render - %3d",  C_VinylManager::Instance().scList().UsedCount() );
			}
			else
			if( !strcmp( p, "HIGHLIGHT" ) )
			{
				VinylList& list = C_VinylManager::Instance().scList();

				U32 cnt = 0;
				for(VinylIterator src_it( GetFirstSiblingIterator( &list, C_VinylManager::Instance().scCurrent() ) ); src_it; ++src_it )
				{
					VinylItem* item = list.At( src_it );
					if( item && item->Highlight() )
						cnt++;
				}

				m_VariableText.Format( L"Highlight - %3d", cnt );
			}
			else
			if( !strcmp( p, "CLIPBOARD" ) )
			{
				m_VariableText.Format( L"Clipboard - %3d", C_VinylManager::Instance().scClipboardList().UsedCount() );
			}
			else
			if( !strcmp( p, "SELECTED" ) )
			{
				VinylItem* item = C_VinylManager::Instance().scList().At( C_VinylManager::Instance().scCurrent() );

				if( item )
				{
					if( item->child < 0 )
					{
						if( item->Shape() == 0xffff )
							m_VariableText.Format( L"[empty]" );
						else
							m_VariableText.Format( L"[Shape]" );
					}
					else
					{
						VinylList& list = C_VinylManager::Instance().scList();

						S32 cnt = 0;
						for(VinylIterator src_it( GetSiblingIterator( &list, item->child ) ); src_it; ++src_it )
							cnt++;

						m_VariableText.Format( L"[Group] - %3d", CountItems( &list, C_VinylManager::Instance().scCurrent() )-1 ); // Don't include 'space' shape
					}
				}
			}
		}
	}

	// unhandled
	if ( !m_VariableText.Length()  )
	{
		LOGWARN( g_Log, DBSTR "name \"%s\" not found", pVariableName );
		return L"";
	}

	return m_VariableText;
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif
