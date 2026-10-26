#include "Stdafx.h"

#include "GameDataFinder.h"

#include <ui/Graphics/Localisation/Loc.h>
#include <ui/Graphics/movie.h>
#include <ui/Graphics/SimpleTimer.h>
#include <ui/Game/UILUA.h>
#include <ui/Game/StringStore.h>
#include <ui/Game/UIVariableStore.h>
#include <ui/Game/UILocaliser.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace UI
{

void C_GameDataFinder::Init()
{
	//
}

const WCHAR* C_GameDataFinder::GetVariableText( const char* pVariableName )
{
	//S32 sIndex = ParseIndex( pVariableName, "TEST_" );
	//SWNPRINTF( m_VariableText, m_uMaxStringSize, L"%d", sIndex );
	//return m_VariableText;
	
	//if ( !strcmp( pVariableName, "TEST" ) )
		//return L"TEST WORKED!";

	if ( !strcmp( pVariableName, "TEST" ) )
	{
		static S32 sCount = 0;

		m_VariableText.Format( L"%d", sCount++ );

		if ( sCount > 5000 )
			sCount = 0;
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
