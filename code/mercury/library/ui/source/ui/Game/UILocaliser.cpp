#include "stdafx.h"

#include "ui/Game/UILocaliser.h"
#include "ui/UIDefines.h"
#include "ui/Graphics/Localisation/Loc.h"

#ifdef PLATFORM_PS3
#include <cell/sysmodule.h>
#include <sysutil/sysutil_common.h>
#include <sysutil/sysutil_licensearea.h>
#endif

XT_TYPE( UI::E_UILanguage, "UILanguage" )
	XT_OPTION( UI::E_UILanguage_English, "English" )
	XT_OPTION( UI::E_UILanguage_French, "French" )
	XT_OPTION( UI::E_UILanguage_German, "German" )
	XT_OPTION( UI::E_UILanguage_Italian, "Italian" )
	XT_OPTION( UI::E_UILanguage_Spanish, "Spanish" )
	XT_OPTION( UI::E_UILanguage_Hungarian, "Hungarian" )
	XT_OPTION( UI::E_UILanguage_Russian, "Russian" )
	XT_OPTION( UI::E_UILanguage_Polish, "Polish" )
	XT_OPTION( UI::E_UILanguage_Czech, "Czech" )
	XT_OPTION( UI::E_UILanguage_SimplifiedChinese, "SimplifiedChinese" )
	XT_OPTION( UI::E_UILanguage_TraditionalChinese, "TraditionalChinese" )
	XT_OPTION( UI::E_UILanguage_Japanese, "Japanese" )
	XT_OPTION( UI::E_UILanguage_Korean, "Korean" )
	XT_OPTION( UI::E_UILanguage_Other, "Other" )
XT_END_TYPE

XT_TYPE( UI::E_UIRegion, "UIRegion" )
	XT_OPTION( UI::E_UIRegion_Europe, "Europe" )
	XT_OPTION( UI::E_UIRegion_NorthAmerica, "NorthAmerica" )
	XT_OPTION( UI::E_UIRegion_Other, "Other" )
XT_END_TYPE

// helpers
static const bool LeapYear( const U32 year )
{
	if ( ( year % 400 ) == 0 )
		return true;
	
	if ( ( year % 100 ) == 0 )
		return false;

	if ( ( year % 4 ) == 0 )
		return true;

	return false;
}

static const U32 GetDayOffsetForYear( const U32 year )
{
	assert( year >= 2007 ); // TODO : years pre 2007

	U32 offset = 0;

	for ( U32 y = 2007; y < year; y++ )
	{
		offset++;

		if ( LeapYear( y ) )
			offset++;
		
		offset = ( offset % 7 );
	}

	return offset;
}

static const U32 Day( const Time::C_DecomposedTime& time )		{ return time.m_mday; }
static const U32 Month( const Time::C_DecomposedTime& time )	{ return time.m_mon + 1; }
static const U32 Year( const Time::C_DecomposedTime& time )		{ return time.m_year + 1900; }
static const U32 Hour( const Time::C_DecomposedTime& time )		{ return time.m_hour; }
static const U32 Minute( const Time::C_DecomposedTime& time )	{ return time.m_min; }
static const U32 Second( const Time::C_DecomposedTime& time )	{ return time.m_sec; }

namespace UI
{

// static instance
C_UILocaliser	C_UILocaliser::m_sInstance;

// -------------
// C_UILocaliser
// -------------
void C_UILocaliser::Init()
{
	// pre-store day and month strings
	char* pDayStrings[ m_DaysInWeek ] = 
	{
		"LOC_DAY_1",
		"LOC_DAY_2",
		"LOC_DAY_3",
		"LOC_DAY_4",
		"LOC_DAY_5",
		"LOC_DAY_6",
		"LOC_DAY_7",
	};

	char* pMonthStrings[ m_MonthsInYear ] = 
	{
		"LOC_MONTH_1",
		"LOC_MONTH_2",
		"LOC_MONTH_3",
		"LOC_MONTH_4",
		"LOC_MONTH_5",
		"LOC_MONTH_6",
		"LOC_MONTH_7",
		"LOC_MONTH_8",
		"LOC_MONTH_9",
		"LOC_MONTH_10",
		"LOC_MONTH_11",
		"LOC_MONTH_12",
	};

	for ( U32 i = 0; i < m_DaysInWeek; i++ )
		m_DayString[ i ] = pDayStrings[ i ];

	for ( U32 i = 0; i < m_MonthsInYear; i++ )
		m_MonthString[ i ] = pMonthStrings[ i ];
}

E_UIRegion C_UILocaliser::ReadRegion()
{
	E_UIRegion eRegion;


#ifdef PLATFORM_PS3

#ifdef USING_THIS_GIVES_WARNINGS_WHEN_CREATING_DISC_IMAGES
	cellSysmoduleLoadModule( CELL_SYSMODULE_SYSUTIL_LICENSEAREA );

	S32 sRegion = cellSysutilGetLicenseArea();

	switch ( sRegion )
	{
		case CELL_SYSUTIL_LICENSE_AREA_E:
			eRegion = E_UIRegion_Europe;
			break;
		case CELL_SYSUTIL_LICENSE_AREA_A:
			eRegion = E_UIRegion_NorthAmerica;
			break;
		default:
			eRegion = E_UIRegion_Other;
			break;
	}

	cellSysmoduleUnloadModule( CELL_SYSMODULE_SYSUTIL_LICENSEAREA );
#else
	if (memcmp(PlatformPS3::g_ps3_titleId,"ABCD",4) == 0)
	{
		eRegion = E_UIRegion_Europe;
	} else
	if (memcmp(PlatformPS3::g_ps3_titleId,"BLES",4) == 0)
	{
		eRegion = E_UIRegion_Europe;
	} else
	if (memcmp(PlatformPS3::g_ps3_titleId,"BLUS",4) == 0)
	{
		eRegion = E_UIRegion_NorthAmerica;
	} else
	{
		eRegion = E_UIRegion_Other;
	}
#endif
#endif
	return eRegion;
}

E_UILanguage C_UILocaliser::ReadLanguage( E_UIRegion eRegion, E_UILanguage eDefaultLanguage )
{
	E_UILanguage eLanguage = eDefaultLanguage;



#ifdef PLATFORM_PC
	/*
	// From Moppy
	 DWORD gLanguage = LANG_ENGLISH;

	void InitialiseLanguage()
	{
		LCID lcid = GetUserDefaultLCID();

		gLanguage = PRIMARYLANGID(lcid);
	}
	*/
#endif

	#ifdef PLATFORM_PS3
	S32 sLang = -1;

	if ( eRegion == E_UIRegion_NorthAmerica )
	{
		// Set english for german italian spanish bug ID: 24680  
		if ( cellSysutilGetSystemParamInt( CELL_SYSUTIL_SYSTEMPARAM_ID_LANG, &sLang ) == CELL_OK )
		{
			switch ( sLang )
			{
				case CELL_SYSUTIL_LANG_FRENCH:		eLanguage = E_UILanguage_French;				break;
				case CELL_SYSUTIL_LANG_GERMAN:		eLanguage = E_UILanguage_English;				break;
				case CELL_SYSUTIL_LANG_ITALIAN:		eLanguage = E_UILanguage_English;				break;
				case CELL_SYSUTIL_LANG_RUSSIAN:		eLanguage = E_UILanguage_Russian;				break;
				case CELL_SYSUTIL_LANG_SPANISH:		eLanguage = E_UILanguage_English;				break;
				case CELL_SYSUTIL_LANG_CHINESE_S:	eLanguage = E_UILanguage_SimplifiedChinese;		break;
				case CELL_SYSUTIL_LANG_CHINESE_T:	eLanguage = E_UILanguage_TraditionalChinese;	break;
				case CELL_SYSUTIL_LANG_JAPANESE:	eLanguage = E_UILanguage_English;				break;// #26065
				case CELL_SYSUTIL_LANG_KOREAN:		eLanguage = E_UILanguage_Korean;				break;
				default:							eLanguage = E_UILanguage_English;				break;
			}
		}
	}
	else
	{
		if ( cellSysutilGetSystemParamInt( CELL_SYSUTIL_SYSTEMPARAM_ID_LANG, &sLang ) == CELL_OK )
		{
			switch ( sLang )
			{
				case CELL_SYSUTIL_LANG_FRENCH:		eLanguage = E_UILanguage_French;				break;
				case CELL_SYSUTIL_LANG_GERMAN:		eLanguage = E_UILanguage_German;				break;
				case CELL_SYSUTIL_LANG_ITALIAN:		eLanguage = E_UILanguage_Italian;				break;
				case CELL_SYSUTIL_LANG_RUSSIAN:		eLanguage = E_UILanguage_Russian;				break;
				case CELL_SYSUTIL_LANG_SPANISH:		eLanguage = E_UILanguage_Spanish;				break;
				case CELL_SYSUTIL_LANG_CHINESE_S:	eLanguage = E_UILanguage_SimplifiedChinese;		break;
				case CELL_SYSUTIL_LANG_CHINESE_T:	eLanguage = E_UILanguage_TraditionalChinese;	break;
				case CELL_SYSUTIL_LANG_JAPANESE:	eLanguage = E_UILanguage_Japanese;				break;
				case CELL_SYSUTIL_LANG_KOREAN:		eLanguage = E_UILanguage_Korean;				break;
				default:							eLanguage = E_UILanguage_English;				break;
			}
		}
	}
	
#endif

	return eLanguage;
}

void C_UILocaliser::DetermineLanguageAndRegion( const S32 sSupportedLanguages, const E_UILanguage eDefaultLanguage )
{
	// default to english and european console
	m_eLanguage = eDefaultLanguage;
	m_eRegion = E_UIRegion_Europe;

	// determine region
	E_UIRegion eRegion = ReadRegion();

	// determine language
	E_UILanguage eLanguage = ReadLanguage( eRegion, eDefaultLanguage );

	// validate language
	if ( LOC_MASK( eLanguage ) & sSupportedLanguages )
		m_eLanguage	= eLanguage;

#ifdef PLATFORM_PC
	// TODO
#endif

	// set region
	m_eRegion = eRegion;
}

const char* C_UILocaliser::GetLanguageCode()
{
	// should never be set to other - debug purposes only
	assert( m_eLanguage != E_UILanguage_Other );

	static const char* s_pUILanguages[ E_UILanguage_Count ] =
	{
		"en",	// E_UILanguage_Other
		"en",	// E_UILanguage_English
		"fr",	// E_UILanguage_French
		"de",	// E_UILanguage_German
		"it",	// E_UILanguage_Italian
		"es",	// E_UILanguage_Spanish
		"hu",	// E_UILanguage_Hungarian
		"ru",	// E_UILanguage_Russian
		"pl",	// E_UILanguage_Polish
		"cs",	// E_UILanguage_Czech
		"sc",	// E_UILanguage_SimplifiedChinese
		"tc",	// E_UILanguage_TraditionalChinese
		"ja",	// E_UILanguage_Japanese
		"ko",	// E_UILanguage_Korean
	};

	return s_pUILanguages[ m_eLanguage ];
}

WCHAR* C_UILocaliser::ConvertSystemTimeToFileName( const Time::C_DecomposedTime& time )
{
	m_String.Format( L"%d-%d-%d_@_%02d-%02d-%02d", Day( time ), Month( time ), Year( time ), Hour( time ), Minute( time ), Second( time ) );		
	
	return m_String.NonConstChars();
}

WCHAR* C_UILocaliser::ConvertSystemTimeToDate( const Time::C_DecomposedTime& time, const bool bFileSystemSafe )
{
	switch ( m_eLanguage )
	{
		case E_UILanguage_English:
		{
			if ( m_eRegion ==  E_UIRegion_NorthAmerica )
				m_String.Format( C_TextStringID( "LOC_TIME_AND_DATE" ).String(), Month( time ), Day( time ), Year( time ), Hour( time ), Minute( time ) );
			else
				m_String.Format( C_TextStringID( "LOC_TIME_AND_DATE" ).String(), Day( time ), Month( time ), Year( time ), Hour( time ), Minute( time ) );
			break;
		}
		case E_UILanguage_Japanese:	// FALL THROUGH
		case E_UILanguage_SimplifiedChinese:
		{
			m_String.Format( C_TextStringID( "LOC_TIME_AND_DATE" ).String(), Year( time ), Month( time ), Day( time ), Hour( time ), Minute( time ) );
			break;
		}
		case E_UILanguage_Korean:
		{
			m_String.Format( C_TextStringID( "LOC_TIME_AND_DATE_DASHES" ).String(), Year( time ), Month( time ), Day( time ), Hour( time ), Minute( time ) );
			break;
		}
		case E_UILanguage_German:
		{
			m_String.Format( C_TextStringID( "LOC_TIME_AND_DATE_DOTS" ).String(), Day( time ), Month( time ), Year( time ), Hour( time ), Minute( time ) );
			break;
		}
		case E_UILanguage_Italian:
		{
			m_String.Format( C_TextStringID( "LOC_TIME_DOTS_AND_DATE" ).String(), Day( time ), Month( time ), Year( time ), Hour( time ), Minute( time ) );
			break;
		}
		default:
		{
			m_String.Format( C_TextStringID( "LOC_TIME_AND_DATE" ).String(), Day( time ), Month( time ), Year( time ), Hour( time ), Minute( time ) );
			break;
		}
	}

	return m_String.NonConstChars();
}

WCHAR* C_UILocaliser::ConvertSystemTimeToDateOnly( const Time::C_DecomposedTime& time, const bool bFileSystemSafe )
{
	switch ( m_eLanguage )
	{
		case E_UILanguage_English:
		{
			if ( m_eRegion == E_UIRegion_NorthAmerica )
				m_String.Format( bFileSystemSafe ? L"%d-%d-%d" : L"%d/%d/%d", Month( time ), Day( time ), Year( time ) );
			else
				m_String.Format( bFileSystemSafe ? L"%d-%d-%d" : L"%d/%d/%d", Day( time ), Month( time ), Year( time ) );

			break;
		}
		case E_UILanguage_Korean:
		{
			m_String.Format( L"%d-%d-%d", Year( time ), Month( time ), Day( time ) );
			break;
		}
		case E_UILanguage_Hungarian:	// FALL THROUGH
		case E_UILanguage_Japanese:		// FALL THROUGH
		case E_UILanguage_SimplifiedChinese:
		{
			m_String.Format( bFileSystemSafe ? L"%d-%d-%d" : L"%d/%d/%d", Year( time ), Month( time ), Day( time ) );
			break;
		}
		case E_UILanguage_German:
		{
			m_String.Format( bFileSystemSafe ? L"%d-%d-%d" : L"%d.%d.%d", Day( time ), Month( time ), Year( time ) );
			break;
		}
		default:
		{
			m_String.Format( bFileSystemSafe ? L"%d-%d-%d" : L"%d/%d/%d", Day( time ), Month( time ), Year( time ) );
			break;
		}
	}

	return m_String.NonConstChars();
}

WCHAR* C_UILocaliser::ConvertSystemTimeToTimeOnly( const Time::C_DecomposedTime& time, const bool bFileSystemSafe )
{
	switch( m_eLanguage )
	{
		case E_UILanguage_Italian:
			m_String.Format( L"%02d.%02d", Hour( time ), Minute( time ) );
			break;
		default:
			m_String.Format( L"%02d:%02d", Hour( time ), Minute( time ) );
			break;
	}

	return m_String.NonConstChars();
}

WCHAR* C_UILocaliser::ConvertNumberToCommas( S64 number )
{
	bool bNeg = false;

	if ( number < 0 )
	{
		bNeg = true;
		number = -number;
	}

	S64 billions = number / 1000000000;
	number -= billions * 1000000000;

	S64 hundredMillions = number / 1000000;
	number -= hundredMillions * 1000000;

	S64 hundredThousands = number / 1000;
	number -= hundredThousands * 1000;
	
	switch ( m_eLanguage )
	{
		case E_UILanguage_French:		// FALL THROUGH
		case E_UILanguage_Polish:		// FALL THROUGH
		case E_UILanguage_Russian:		// FALL THROUGH
		case E_UILanguage_Hungarian:	// FALL THROUGH
		case E_UILanguage_Czech:
		{
			if ( billions )
				m_String.Format( bNeg ? L"-%d %03d %03d %03d" : L"%d %03d %03d %03d", billions, hundredMillions, hundredThousands, number );
			else if ( hundredMillions )
				m_String.Format( bNeg ? L"-%d %03d %03d" : L"%d %03d %03d", hundredMillions, hundredThousands, number );
			else if ( hundredThousands )
				m_String.Format( bNeg ? L"-%d %03d" : L"%d %03d", hundredThousands, number );
			else
				m_String.Format( bNeg ? L"-%d" : L"%d", number );

			break;
		}
		case E_UILanguage_German:		// FALL THROUGH
		case E_UILanguage_Italian:		// FALL THROUGH
		case E_UILanguage_Spanish:
		{
			if ( billions )
				m_String.Format( bNeg ? L"-%d.%03d.%03d.%03d" : L"%d.%03d.%03d.%03d", billions, hundredMillions, hundredThousands, number );
			else if ( hundredMillions )
				m_String.Format( bNeg ? L"-%d.%03d.%03d" : L"%d.%03d.%03d", hundredMillions, hundredThousands, number );
			else if ( hundredThousands )
				m_String.Format( bNeg ? L"-%d.%03d" : L"%d.%03d", hundredThousands, number );
			else
				m_String.Format( bNeg ? L"-%d" : L"%d", number );

			break;
		}
		default:
		{
			if ( billions )
				m_String.Format( bNeg ? L"-%d,%03d,%03d,%03d" : L"%d,%03d,%03d,%03d", billions, hundredMillions, hundredThousands, number );
			else if ( hundredMillions )
				m_String.Format( bNeg ? L"-%d,%03d,%03d" : L"%d,%03d,%03d", hundredMillions, hundredThousands, number );
			else if ( hundredThousands )
				m_String.Format( bNeg ? L"-%d,%03d" : L"%d,%03d", hundredThousands, number );
			else
				m_String.Format( bNeg ? L"-%d" : L"%d", number );

			break;
		}
	}

	return m_String.NonConstChars();
}

WCHAR* C_UILocaliser::ConvertNumberToPosition( S64 number, const bool bCaps, const WCHAR* pSuffixModifier, const WCHAR* pSuffixModifierReset, const bool bDoNumber, const bool bDoSuffix )
{
	m_String.Clear();

	static C_TextStringID s_ZeroNumberText( "LOC_ZERO_NUMBER" );

	static C_TextStringID s_LocThCapsText( "LOC_TH_CAPS" );
	static C_TextStringID s_LocStCapsText( "LOC_ST_CAPS" );
	static C_TextStringID s_LocNdCapsText( "LOC_ND_CAPS" );
	static C_TextStringID s_LocRdCapsText( "LOC_RD_CAPS" );

	static C_TextStringID s_LocThText( "LOC_TH" );
	static C_TextStringID s_LocStText( "LOC_ST" );
	static C_TextStringID s_LocNdText( "LOC_ND" );
	static C_TextStringID s_LocRdText( "LOC_RD" );

	if ( ( number == 0 ) && bDoNumber )
		m_String.Format( s_ZeroNumberText.String() );
	else
	{
		if ( bDoNumber )
			ConvertNumberToCommas( number );

		if ( bDoSuffix )
		{
			if ( pSuffixModifier && ( wcslen( pSuffixModifier ) != 0 ) )
				m_String += pSuffixModifier;

			S64 mod = number % 10;

			if ( ( number > 10 ) && ( number < 14 ) ) // 11th, 12th and 13th
				m_String += bCaps ? s_LocThCapsText.String() : s_LocThText.String();
			else if ( mod == 1 )
				m_String += bCaps ? s_LocStCapsText.String() : s_LocStText.String();
			else if ( mod == 2 )
				m_String += bCaps ? s_LocNdCapsText.String() : s_LocNdText.String();
			else if ( mod == 3 )
				m_String += bCaps ? s_LocRdCapsText.String() : s_LocRdText.String();
			else
				m_String += bCaps ? s_LocThCapsText.String() : s_LocThText.String();

			if ( pSuffixModifierReset && ( wcslen( pSuffixModifierReset ) != 0 ) )
				m_String += pSuffixModifierReset;
		}
	}

	return m_String.NonConstChars();
}

WCHAR* C_UILocaliser::ConvertNumberToDay( S64 number )
{
	if ( number == 0 )
		m_String.Format( L"--" );
	else
	{
		ConvertNumberToCommas( number );

		S64 mod = number % 10;

		if ( ( number > 10 ) && ( number < 14 ) ) // 11th, 12th and 13th
			m_String += C_TextStringID( "LOC_TH" ).String();
		else if ( mod == 1 )
			m_String += C_TextStringID( "LOC_ST" ).String();
		else if ( mod == 2 )
			m_String += C_TextStringID( "LOC_ND" ).String();
		else if ( mod == 3 )
			m_String += C_TextStringID( "LOC_RD" ).String();
		else
			m_String += C_TextStringID( "LOC_TH" ).String();
	}

	return m_String.NonConstChars();
}

WCHAR* C_UILocaliser::ConvertNumberToGear( S64 number )
{
	if ( number < 0 )
		m_String.Format( C_TextStringID( "LOC_GEARS_R" ).String() );
	else
		ConvertNumberToCommas( number );

	return m_String.NonConstChars();
}

WCHAR* C_UILocaliser::ConvertNumberToCash( S64 number )
{
	ConvertNumberToCommas( number );

	m_String.Insert( C_TextStringID( "LOC_CASH_SYMBOL" ).String(), 0 );
	return m_String.NonConstChars();
}

WCHAR* C_UILocaliser::ConvertTextToTime( char* pTime )
{
	S32 minutes = 0, secs = 0, tenths = 0;
	S32 a = 0, b = 0;
	char temp[ 3 ];

	if ( strlen( pTime ) == 0 )
		return L"";

	while ( pTime[ a ] != ':' )
	{
		assert( b < 3 );

		if ( b == 3 )
			break;

		temp[ b++ ]= pTime[ a++ ];
	}

	assert( b < 3 );

	temp[ b ] = 0;
	minutes = atoi( temp );

	a++;
	b = 0;

	while( pTime[ a ] != '.' )
		temp[ b++ ] = pTime[ a++ ];

	temp[ b ] = 0;
	secs = atoi( temp );

	a++;
	b = 0;

	while( pTime[ a ] )
		temp[ b++ ] = pTime[ a++ ];

	temp[ b ] = 0;
	tenths = atoi( temp );
  	
	// wth is the point of this - doesn't make sense to chop it up then put back together in exactly the same format ???
	//SWNPRINTF( m_String, m_MaxStringLength, L"%d:%02d.%03d", minutes, secs, tenths );

	// maybe you meant this? - language specific from one standard format
	ConvertMSToTime( (S32)( minutes * 60000.0f + secs * 1000.0f + tenths * 100.0f ) );

	return m_String.NonConstChars();
}

const char C_UILocaliser::GetNumberSeperator()
{
	switch ( m_eLanguage )
	{
		case E_UILanguage_English:
		case E_UILanguage_Korean:
		case E_UILanguage_Japanese:
		case E_UILanguage_SimplifiedChinese:
		case E_UILanguage_TraditionalChinese:
		{
			return '.';
			break;
		}
		default:
		{
			return ',';
			break;
		}

		return ',';
	}
}

WCHAR* C_UILocaliser::ConvertMSToTime( S64 time, bool bUseSpecial, bool bPlus, bool bAlwaysShowNumber )
{
	// PR - TODO - fix up the separator properly!

	bool bNeg = false;

	if ( time < 0 )
	{
		bNeg = true;
		time = -time;
	}

	S32 mins = (S32)( time / 60000.0f );
	S32 secs = (S32)( ( time - ( mins * 60000.0f ) ) / 1000.0f );
	S32 msecs = (S32)( time - ( mins * 60000.0f ) - ( secs * 1000.0f ) );

	msecs /= 10;

	char sep = GetNumberSeperator();	// time seperator
	char start = bNeg ? '-' : ( bPlus ? '+' : 0 );

	static C_TextStringID s_TimeStringSpecialCText( "LOC_TIME_STRING_SPECIAL_C" );
	static C_TextStringID s_TimeStringCText( "LOC_TIME_STRING_C" );

	static C_TextStringID s_TimeStringSpecialText( "LOC_TIME_STRING_SPECIAL" );
	static C_TextStringID s_TimeStringText( "LOC_TIME_STRING" );

	static C_TextStringID s_TimeZeroStringSpecialText( "LOC_TIME_ZERO_STRING_SPECIAL" );
	static C_TextStringID s_TimeZeroStringText( "LOC_TIME_ZERO_STRING" );

	if ( time != 0 )
	{
		if ( start != 0 )
		{
			if ( bUseSpecial )
				m_String.Format( s_TimeStringSpecialCText.String(), start, mins, secs, /*sep, */msecs );
			else
				m_String.Format( s_TimeStringCText.String(), start, mins, secs, sep, msecs );
		}
		else
		{
			if ( bUseSpecial )
				m_String.Format( s_TimeStringSpecialText.String(), mins, secs, /*sep, */msecs );
			else
				m_String.Format( s_TimeStringText.String(), mins, secs, sep, msecs );
		}
	}
	else if ( !bAlwaysShowNumber )
	{
		if ( bUseSpecial )
			m_String.Format( s_TimeZeroStringSpecialText.String(), sep );
		else
			m_String.Format( s_TimeZeroStringText.String(), sep );
	}

	return m_String.NonConstChars();
}

WCHAR* C_UILocaliser::ConvertFloatSecondsToTime( F32 time, bool bUseSpecial, bool bPlus, bool bAlwaysShowNumber )
{
	ConvertMSToTime( (S32)( time * 1000.0f ), bUseSpecial, bPlus, bAlwaysShowNumber );

	return m_String.NonConstChars();
}

WCHAR* C_UILocaliser::ConvertFloatSecondsToTimeDHM( F32 fTime )
{
	static C_TextStringID s_TimeDHM( "LOC_TIME_DHM_D_H_M" );
	static C_TextStringID s_TimeHM( "LOC_TIME_DHM_H_M" );
	static C_TextStringID s_TimeM( "LOC_TIME_DHM_M" );

	S32 sMins = 0;
	S32 sHours = 0;
	S32 sDays = 0;	

	if ( fTime > 0.0f )
	{
		sDays = (S32)( fTime / 86400.0f );
		sHours = (S32)( ( fTime - ( sDays * 86400.0f ) ) / 3600.0f );
		sMins = (S32)( ( fTime - ( sDays * 86400.0f ) - ( sHours * 3600.0f ) ) / 60.0f );
	}

	if ( sDays > 0 )
		m_String.Format( s_TimeDHM.String(), sDays, sHours, sMins );
	else if ( sHours > 0 )
		m_String.Format( s_TimeHM.String(), sHours, sMins );
	else
		m_String.Format( s_TimeM.String(), sMins );

	return m_String.NonConstChars();
}

WCHAR* C_UILocaliser::ConvertDayAndWeekNumberToDate( S32 day, S32 weekNumber, S32 year, bool bShowDay )
{
	C_CalandarMonthDate md( C_CalandarWeekDay( day, weekNumber, 2008 + year ) );

	// check for duff sorcery data
	if ( day < 0 || weekNumber < 0 )
		return L"Invalid date";

	C_WideString32 _pDateString;
	m_String.Format( ConvertNumberToDay( md.Date() + 1 ) );  // DG - +1 needed as my stuff (rather confusingly) runs 0-6 & 0-(ml-1)

	const WCHAR* pDayString = bShowDay ? m_DayString[ day ].String() : L"";
	const WCHAR* pDateString = _pDateString; // Ps3 fix

	switch( m_eLanguage )
	{
		case E_UILanguage_Japanese:				// FALL THROUGH
		case E_UILanguage_Korean:				// FALL THROUGH
		case E_UILanguage_Hungarian:			// FALL THROUGH
		case E_UILanguage_SimplifiedChinese:
		{
			m_String.Format( L"%s %s %s", m_MonthString[ md.Month() ].String(), pDateString, pDayString );
			break;
		}
		default:
		{
			m_String.Format( L"%s %s %s", pDayString, pDateString, m_MonthString[ md.Month() ].String() );
			break;
		}
	}

	return m_String.NonConstChars();
}

const U32 C_UILocaliser::YearLength( const U32 year ) const
{
	return LeapYear( year ) ? 366 : 365;
}

const U32 C_UILocaliser::MonthLength( const U32 month, const U32 year ) const
{
	static const U8 monthLengths[ m_MonthsInYear ] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	assert( month < m_MonthsInYear );

	if ( month == 1 && LeapYear( year ) )
		return 29;

	return monthLengths[ month ];
}

// -------------------
// C_CalandarMonthDate
// -------------------
C_CalandarMonthDate::C_CalandarMonthDate( const C_CalandarWeekDay& in )
{
	*this = in;
}

void C_CalandarMonthDate::operator = ( const C_CalandarWeekDay& copy )
{
	// get days into this year (and year)
	S32 days = ( copy.WeekNum() * 7 ) + copy.DayOfWeek() - GetDayOffsetForYear( copy.Year() );

	if ( days < 0 )
	{
		m_Year = copy.Year() - 1;
		days += C_UILocaliser::Instance().YearLength( m_Year );
	}
	else
		m_Year = copy.Year();

	// find date and month from days
	m_Month = 0;

	for( S32 i = 0; ( days - C_UILocaliser::Instance().MonthLength( i, m_Year ) ) >= 0; i++ )
	{
		m_Month++;
		days -= C_UILocaliser::Instance().MonthLength( i, m_Year );
	}

	m_Date = days;
}

const bool C_CalandarMonthDate::operator > ( const C_CalandarMonthDate& in ) const
{
	if ( m_Year != in.Year() )
		return ( m_Year > in.Year() );

	if ( m_Month != in.Month() )
		return ( m_Month > in.Month() );

	return ( m_Date > in.Date() );
}

const bool C_CalandarMonthDate::operator < ( const C_CalandarMonthDate& in ) const
{
	if ( m_Year != in.Year() )
		return ( m_Year < in.Year() );

	if ( m_Month != in.Month() )
		return ( m_Month < in.Month() );

	return ( m_Date < in.Date() );
}

const bool C_CalandarMonthDate::operator == ( const C_CalandarMonthDate& in ) const
{
	if ( in.Date() != m_Date )
		return false;

	if ( in.Month() != m_Month )
		return false;

	if ( in.Year() != m_Year )
		return false;

	return true;
}

// -----------------
// C_CalandarWeekDay
// -----------------
C_CalandarWeekDay::C_CalandarWeekDay( const C_CalandarMonthDate& in )
{
	*this = in;
}

void C_CalandarWeekDay::operator = ( const C_CalandarMonthDate& copy )
{	
	S32 days = 0;

	for ( U32 i = 0; i < copy.Month(); i++ )
		days += C_UILocaliser::Instance().MonthLength( i, copy.Year() );

	days += copy.Date();

	// get start day for this year
	days += GetDayOffsetForYear( copy.Year() );

	if ( days > (S32)C_UILocaliser::Instance().YearLength( copy.Year() ) )
	{
		days -= C_UILocaliser::Instance().YearLength( copy.Year() );
		m_WeekNum = days / 7;
		m_DayOfWeek = days % 7;
		m_Year = copy.Year() + 1;
	}
	else
	{
		m_WeekNum = days / 7;
		m_DayOfWeek = days % 7;
		m_Year = copy.Year();
	}
}

}; // namespace UI
