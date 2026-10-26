#include "Stdafx.h"

#include <ui/Graphics/Localisation/Loc.h>
#include <ui/Game/StringStore.h>
#include <ui/Game/UILocaliser.h>
#include <ui/Game/UIVariableStore.h>
#include <ui/Game/Profiles/ProfileDataFinder.h>
#include <ui/Game/Profiles/ProfileInterface.h>
#include <ui/Game/SaveSystem/SaveSystem.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace UI
{

// global instance
C_ProfileDataFinder C_ProfileDataFinder::m_sInstance;

// -------------------
// C_ProfileDataFinder
// -------------------
void C_ProfileDataFinder::Init()
{
	//
}

const WCHAR* C_ProfileDataFinder::GetVariableText( const char* pVariableName )
{
	m_VariableText.Clear();

	// pad profile data
	if ( !strncmp( pVariableName, "PAD", 3 ) )
	{
		pVariableName += 3;	// move passed the header

		S32 sIndex = (S32)ParseIndex( pVariableName, "" );

		if ( sIndex == -1 )
			sIndex = SysProfile::C_Manager::Instance().GetPrimaryPadNo();
		else
			pVariableName += 1;	// move passed the number

		// get the pad profile
		SysProfile::FullProfile& padProfile = C_ProfileInterface::Instance().GetPadProfile( sIndex );

		if ( !strcmp( pVariableName, "_NAME" ) )
		{
			if ( !padProfile.Name()[ 0 ] )
		#ifdef PLATFORM_PS3
				m_VariableText.Format( L"---" );
		#else
				m_VariableText.Format( C_TextStringID( "PRO_NOT_SIGNED_IN" ).String() );
		#endif
			else
				m_VariableText = padProfile.Name();
		}
		else if ( !strcmp( pVariableName, "_STATUS" ) )
		{
			#ifdef CFG_GOLD
			return L"";
			#else
			switch ( padProfile.m_State )
			{
				case SysProfile::eProfileStateNotActive:
					m_VariableText.Format( L"-NotActive-" ); break;
				case SysProfile::eProfileStateOnline:
					m_VariableText.Format( L"-Online-" ); break;
				case SysProfile::eProfileStateOffline:
					m_VariableText.Format( L"-Offline-" ); break;
				case SysProfile::eProfileStateBlagged:
					m_VariableText.Format( L"-Blagged-" ); break;
				default:
					return L"";
			}
			#endif
		}
		else if ( !strcmp( pVariableName, "_BLAGGED" ) )
		{
			m_VariableText.Format( C_TextStringID( "PRO_BLAGGED" ).String(), padProfile.Name() );
		}
		else if ( !strcmp( pVariableName, "_LOADED" ) )
		{
			m_VariableText.Format( C_TextStringID( "PRO_LOADED" ).String(), padProfile.Name() );
		}
		else if ( !strcmp( pVariableName, "_CREATED" ) )
		{
			m_VariableText.Format(  C_TextStringID( "PRO_CREATED" ).String(), padProfile.Name() );
		}
		else if ( !strcmp( pVariableName, "_CORRUPT" ) )
		{
			m_VariableText.Format( C_TextStringID( "PRO_CORRUPT" ).String(), padProfile.Name() );
		}
		else if ( !strcmp( pVariableName, "_DIFFERENT_OWNER" ) )
		{
			m_VariableText.Format( C_TextStringID( "PRO_DIFFERENT_OWNER" ).String(), padProfile.Name() );
		}
		else if ( !strcmp( pVariableName, "_DELETE_FAILED" ) )
		{
			m_VariableText.Format( C_TextStringID( "PRO_DELETE_FAILED" ).String(), padProfile.Name() );
		}
		else if ( !strcmp( pVariableName, "_ENUMERATION_FAILED" ) )
		{
			m_VariableText.Format( C_TextStringID( "PRO_ENUMERATION_FAILED" ).String(), padProfile.Name() );
		}
		else if ( !strcmp( pVariableName, "_ENUMERATION_LOADED" ) )
		{
			m_VariableText.Format( C_TextStringID( "PRO_ENUMERATION_LOADED" ).String(), padProfile.Name() );
		}
		else if ( !strcmp( pVariableName, "_ENUMERATION_CORRUPT" ) )
		{
			m_VariableText.Format( C_TextStringID( "PRO_ENUMERATION_CORRUPT" ).String(), padProfile.Name() );
		}
	#ifdef PLATFORM_PS3
		else if ( !strcmp( pVariableName, "_NO_SPACE_PS3" ) )
		{
			m_VariableText.Format( C_TextStringID( "PRO_NO_SPACE_PS3" ).String(), padProfile.Name(), C_SaveSystem::Instance().FreeSpaceNeededKB() );
		}
	#endif
		else if ( !strcmp( pVariableName, "_NO_SPACE" ) )
		{
			m_VariableText.Format( C_TextStringID( "PRO_NO_SPACE" ).String(), padProfile.Name() );
		}
	}
	else if ( !strncmp( pVariableName, "SAVE_ENUMERATION", 16 ) )
	{
		pVariableName += 16;	// move passed the header

		S32 sIndex = (S32)ParseIndex( pVariableName, "" );

		if ( sIndex == -1 )
			return L"<<INVALID ENUMERATION>>";

		m_VariableText.Format( C_SaveSystem::Instance().GetEnumerationName( --sIndex ) );
	}
	else
	{
		// unhandled
		LOGWARN( g_Log, DBSTR "name \"%s\" not found", pVariableName );
		return L"";
	}

	return m_VariableText;	
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif
