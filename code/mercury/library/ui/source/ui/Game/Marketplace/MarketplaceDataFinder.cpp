#include "Stdafx.h"

#include "ui/Game/Marketplace/MarketplaceDataFinder.h"
#include "ui/Game/Marketplace/Marketplace.h"

#include "ui/Game/UILocaliser.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace UI
{

// global instance
C_MarketplaceDataFinder C_MarketplaceDataFinder::m_sInstance;

// -----------------------
// C_MarketplaceDataFinder
// -----------------------
void C_MarketplaceDataFinder::Init()
{
	//
}

const WCHAR* C_MarketplaceDataFinder::GetVariableText( const char* pVariableName )
{
	m_VariableText.Clear();

	if ( !strcmp( pVariableName, "TOTAL_OFFERS" ) )
	{
	}
	else if ( !strcmp( pVariableName, "NEW_OFFERS" ) )
	{
	}
	else if ( !strncmp( pVariableName, "OFFER_", 6 ) )
	{
		S64 sIndex = ParseIndexAdvance( pVariableName, "OFFER_" );

		if ( sIndex == -1 )
			return L"";

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
