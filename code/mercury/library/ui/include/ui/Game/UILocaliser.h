#ifndef UILOCALISER_H
#define UILOCALISER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <system/time/time.h>
#include <btl/Containers/containers.h>

#include "ui/Graphics/Localisation/loc.h"
#include "ui/Util/WideString.h"

namespace UI
{

/// @addtogroup Localisation
/// @{

#define	 LOC_MASK( x )		( 1 << x )

/// the UI languages
enum E_UILanguage
{
	E_UILanguage_Other = 0,				///< en

	E_UILanguage_English,				///< en
	E_UILanguage_French,				///< fr
	E_UILanguage_German,				///< de
	E_UILanguage_Italian,				///< it
	E_UILanguage_Spanish,				///< es
	E_UILanguage_Hungarian,				///< hu
	E_UILanguage_Russian,				///< ru
	E_UILanguage_Polish,				///< pl
	E_UILanguage_Czech,					///< cs

	E_UILanguage_SimplifiedChinese,		///< sc
	E_UILanguage_TraditionalChinese,	///< tc
	E_UILanguage_Japanese,				///< ja
	E_UILanguage_Korean,				///< ko
	
	E_UILanguage_Count,
};

/// the UI regions
enum E_UIRegion
{
	E_UIRegion_Europe = 0,				///< @a
	E_UIRegion_NorthAmerica,			///< @a

	E_UIRegion_Other,					///< @a

	E_UIRegion_Count,
};

// -------------
// C_UILocaliser
// -------------
/// the "mighty" UI localiser - formats different depending on the language and region set
/// the localiser depends on at most all of the following strings defined in Sorcery (modify as appropriate)
/// LOC_DAY_1 - LOC_DAY_7 => "Monday" - "Sunday"
/// LOC_MONTH_1 - LOC_MONTH_12 => "January" - "December"
/// LOC_TIME_AND_DATE => "%d/%d/%d @ %02d:%02d"
/// LOC_TIME_AND_DATE_DASHES => "%d-%d-%d @ %02d:%02d"
/// LOC_TIME_AND_DATE_DOTS => "%d.%d.%d @ %02d:%02d"
/// LOC_TIME_DOTS_AND_DATE => "%d/%d/%d @ %02d.%02d"
/// LOC_TH => "th"
/// LOC_ST => "st"
/// LOC_ND => "nd"
/// LOC_RD => "rd"
/// LOC_GEARS_R => "R"
/// LOC_CASH_SYMBOL => "£"
/// LOC_TIME_STRING => "%d:%02d%c%02d"
/// LOC_TIME_STRING_SPECIAL => "%d:%02d~~/s50~~~~/y-1~~ %c%02d~~/y1~~~~/s200~~" (for example)
/// LOC_ZERO_NUMBER => "--"
/// LOC_TIME_ZERO_STRING_SPECIAL => "--:--~~/s50~~~~/y-1~~ %c--~~/y1~~~~/s200~~" (for example)
/// LOC_TIME_ZERO_STRING => "--:--%c--"
class C_UILocaliser
{
public:
	/// singleton
	static C_UILocaliser&	Instance()		{ return m_sInstance; }

public:
	/// default - english language with europe region
	C_UILocaliser() 
		: m_eLanguage( E_UILanguage_English ), 
		m_eRegion( E_UIRegion_Europe )
	{
		//
	}

public:
	void		Init();																														///< @a
	void		DetermineLanguageAndRegion( const S32 sSupportedLanguages, const E_UILanguage eDefaultLanguage = E_UILanguage_English );	///< bitwise supported language
	const char*	GetLanguageCode();		
	const char	GetNumberSeperator();

	WCHAR*		ConvertSystemTimeToFileName( const Time::C_DecomposedTime& time );											///< D-M-Y @ h-m-s
	WCHAR*		ConvertSystemTimeToDate( const Time::C_DecomposedTime& time, const bool bFileSystemSafe = false );			///< @a
	WCHAR*		ConvertSystemTimeToDateOnly( const Time::C_DecomposedTime& time, const bool bFileSystemSafe = false );		///< @a
	WCHAR*		ConvertSystemTimeToTimeOnly( const Time::C_DecomposedTime& time, const bool bFileSystemSafe = false );		///< @a

	WCHAR*		ConvertNumberToCommas( S64 number );																									///< @a
	WCHAR*		ConvertNumberToPosition( S64 number, const bool bCaps = false, const WCHAR* pSuffixModifier = NULL, const WCHAR* pSuffixModifierReset = NULL, const bool bDoNumber = true, const bool bDoSuffix = true );	///< 1st, 2nd, 3rd etc
	WCHAR*		ConvertNumberToDay( S64 number );																										///< 1st, 2nd, 3rd etc
	WCHAR*		ConvertNumberToGear( S64 number );																										///< ( < 0 ) => "R"
	WCHAR*		ConvertNumberToCash( S64 number );																										///< e.g. £1,234,567

	WCHAR*		ConvertTextToTime( char* pTime );	/// "%d:%02d.%03d" => localised time
	WCHAR*		ConvertMSToTime( S64 time, bool bUseSpecial = true, bool bPlus = false, bool bAlwaysShowNumber = false );			///< @a
	WCHAR*		ConvertFloatSecondsToTime( F32 time, bool bUseSpecial = true, bool bPlus = false, bool bAlwaysShowNumber = false );	///< @a
	WCHAR*		ConvertFloatSecondsToTimeDHM( F32 fTime );																			///< 0d 0h 0m

	F32			ConvertMPHtoKPH( float mph )							{ return mph * 1.609344f; }	///< 1 mph = 1.609344 kph
	
	WCHAR*		ConvertDayAndWeekNumberToDate( S32 day, S32 weekNumber, S32 year, bool bShowDay = true );	///< => "Wednesday 26th March"

	const U32	YearLength( const U32 year ) const;							///< @a
	const U32	MonthLength( const U32 month, const U32 year ) const;		///< @a

	const U32	MonthsInYear() const									{ return m_MonthsInYear; }	///< @a
	const U32	DaysInWeek() const										{ return m_DaysInWeek; }	///< @a
	const U32	WeeksInYear() const										{ return m_WeeksInYear; }	///< @a

	UI::C_TextStringID&		MonthString( const U32 month ) 				{ return m_MonthString[ month ]; }	///< @a
	UI::C_TextStringID&		DayString( const U32 day ) 					{ return m_DayString[ day ]; }		///< @a


	E_UIRegion				ReadRegion();																					////< Reads region from hardware
	E_UILanguage			ReadLanguage( E_UIRegion region, E_UILanguage eDefaultLanguage = E_UILanguage_English );		////< Reads language from hardware

	void					Language( const E_UILanguage eLanguage )	{ m_eLanguage = eLanguage; }		///< @a
	E_UILanguage			Language() const							{ return m_eLanguage; }				///< @a
	void					Region( const E_UIRegion eRegion )			{ m_eRegion = eRegion; }			///< @a
	E_UIRegion				Region() const								{ return m_eRegion; }				///< @a

private:
	static const U32			m_DaysInWeek	= 7;
	static const U32			m_MonthsInYear	= 12;
	static const U32			m_WeeksInYear	= 52;

	static const U32			m_MaxStringLength	= 64;

	// static instance of the localiser
	static C_UILocaliser		m_sInstance;

private:
	Btl::Vector< C_TextStringID, Btl::VectorParameters_Array< m_MonthsInYear > >	m_MonthString;
	Btl::Vector< C_TextStringID, Btl::VectorParameters_Array< m_DaysInWeek > >		m_DayString;

	C_WideString<m_MaxStringLength>		m_String;
	
	E_UILanguage						m_eLanguage;
	E_UIRegion							m_eRegion;
};

// calendar helper classes

class C_CalandarWeekDay;

// -------------------
// C_CalandarMonthDate
// -------------------
/// holder for day, month and year
class C_CalandarMonthDate
{
public:
	/// validate - year >= 2007 (for now)
	C_CalandarMonthDate( const U32 date, const U32 month, const U32 year )
		: m_Date( date ),
		m_Month( month ),
		m_Year( year )
	{
		// validate - months are 0-11, dates are 0-ml
		assert( m_Month < 12 );
		assert( m_Date < C_UILocaliser::Instance().MonthLength( month, year ) );
		assert( year >= 2007 );
	}

	/// construct from a C_CalandarWeekDay
	C_CalandarMonthDate( const C_CalandarWeekDay& in );

public:
	void		operator = ( const C_CalandarWeekDay& copy );				///< @a
	const bool	operator == ( const C_CalandarMonthDate& in ) const;		///< @a
	const bool	operator > ( const C_CalandarMonthDate& in ) const;			///< @a
	const bool	operator < ( const C_CalandarMonthDate& in ) const;			///< @a

	const U32 	Date() const			{ return m_Date; }					///< @a
	const U32 	Month() const			{ return m_Month; }					///< @a
	const U32 	Year() const			{ return m_Year; }					///< @a

private:
	U32			m_Date;
	U32			m_Month;
	U32			m_Year;
};

// -----------------
// C_CalandarWeekDay
// -----------------
/// holder for day of week, week number and year
class C_CalandarWeekDay
{
public:
	/// validate - year >= 2007 (for now)
	C_CalandarWeekDay( const U32 dow, const U32 weeknum, const U32 year )
		: m_DayOfWeek( dow ),
		m_WeekNum( weeknum ),
		m_Year( year )
	{
		// validate
		assert( weeknum <= 52 ); // you can get 52 weeks and 1 day (two in a leap year)
		assert( dow < 7 );
		assert( year >= 2007 );
	}

	/// construct from a C_CalandarMonthDate
	C_CalandarWeekDay( const C_CalandarMonthDate& in );

public:
	void		operator = ( const C_CalandarMonthDate& copy );		///< @a

	const U32 	DayOfWeek() const		{ return m_DayOfWeek; }		///< @a
	const U32 	WeekNum() const			{ return m_WeekNum; }		///< @a
	const U32 	Year() const			{ return m_Year; }			///< @a

private:
	U32			m_DayOfWeek;
	U32			m_WeekNum;
	U32			m_Year;
};

/// @}

}; // namespace UI

#endif	// UILOCALISER_H
