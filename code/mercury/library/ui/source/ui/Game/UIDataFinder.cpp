#include "stdafx.h"

#include "ui/Game/StringStore.h"
#include "ui/Game/UIDataFinder.h"

// TODO: [Rog] Not how we should be turning off optimisations
#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace UI
{

// ----------------------
// C_UIBaseGameDataFinder
// ----------------------
S64 C_UIBaseGameDataFinder::ParseIndex( const char* pVariableName, const char* pHeader, const bool bFindFirstNumber )
{
	S32 len = strlen( pHeader );

	if ( strncmp( pVariableName, pHeader, len ) )
		return -1;	// header doesn't match

	char buff[ 17 ];	// should be fine

	const char* pStart = pVariableName + len;

	if ( bFindFirstNumber )
	{
		while ( ( *pStart < '0' || *pStart > '9' ) && ( *pStart != '\0' && *pStart != '-' ) )
			pStart++;
	}

	bool negative = false;
	if ( *pStart == '-' )
	{
		negative = true;
		pStart++;
	}

	// could be that tokens start with the same header string, but are different tokens e.g. PL9_NAME, PL_LOBBY9_NAME
	if ( *pStart < '0' || *pStart > '9' )
		return -1;

	const char* pEnd = pStart;

	while ( ( *pEnd >= '0') && ( *pEnd <= '9' ) && ( *pEnd != '\0' ) && ( ( pEnd - pStart ) < ( (S32)sizeof( buff ) -1 ) ) )
		pEnd++;

	strncpy( buff, pStart, ( pEnd - pStart ) );
	buff[ ( pEnd - pStart ) ] = '\0';

	// need this incase we are parsing sorcery id's as they are U32's.
	S64 returnNum = 0;
	U32 count = strlen( buff )-1;

	for ( char* ptr = buff; *ptr; ptr++ )
	{
		U32 power = 1;
		for ( U32 i = 0; i < count; i++ )
			power *= 10;

		returnNum += ( ( *ptr - '0' ) * power );
		count--;
	}
	return ( negative ) ? -returnNum : returnNum; //atol( buff );
}


S64 C_UIBaseGameDataFinder::ParseIndexAdvance( const char*& pVariableName, const char* pHeader, const bool bFindFirstNumber )
{
	S64 index = ParseIndex( pVariableName, pHeader, bFindFirstNumber );

	pVariableName += strlen( pHeader );

	// move past the minus
	if ( *pVariableName == '-' )
		pVariableName++;

	const char* pEnd = pVariableName;

	while ( *pEnd >= '0' && *pEnd <= '9' && *pEnd != '\0' )
		pEnd++;

	pVariableName += ( pEnd - pVariableName );

	return index;
}


// global instance
C_UIDataFinder C_UIDataFinder::m_sInstance;

// --------------
// C_UIDataFinder
// --------------
void C_UIDataFinder::AddGameDataFinder( C_UIBaseGameDataFinder* pGameDataFinder )
{
	assert( pGameDataFinder );
	m_GameDataFinders.PushBack( pGameDataFinder );
	pGameDataFinder->Init();
}

const char* UiDfMatchesPrefix(const char* text, const char* toMatch)
{
	for(; *toMatch; text++, toMatch++)
	{
		if (!*text)
			return 0;

		if (*text != *toMatch)
			return 0;
	}

	return text;
}

const WCHAR* C_UIDataFinder::GetVariableText( const char* pVariableName )
{
	// string store
	if (
			(pVariableName[0] == 'S') &&
			(pVariableName[1] == 'T') &&
			(pVariableName[2] == 'R') &&
			(pVariableName[3] == '_')
		)
	{
		return C_StringStore::Instance().Get( atoi( pVariableName + 4 ) );
	}

	// else we check through all of the derived classes to see if the headers match
	for ( S32 i = 0; i < m_GameDataFinders.Size(); i++ )
	{
		const char* pHeader		= m_GameDataFinders[ i ]->GetHeader();
		const char* pMatchEnd	= UiDfMatchesPrefix( pVariableName, pHeader );

		if ( pMatchEnd )
			return m_GameDataFinders[ i ]->GetVariableText( pMatchEnd );
	}

	// unhandled
	LOGWARN( g_Log, DBSTR "name \"%s\" not found", pVariableName );

	return L"<<DATAFINDER ERROR>>";
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif
