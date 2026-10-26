#ifndef UILEADERBOARD_H
#define UILEADERBOARD_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/Game/Gadgets/Buttons.h"
#include "ui/Graphics/Font/FontStyle.h"

#include <btl/Containers/containers.h>

#define COLUMN_MAX_STRING_SIZE		128		// PS3's fault - doesn't like static const's in unions

namespace UI
{

/// @addtogroup Leaderboards
/// @{

/// data types that leaderboards support
enum E_UIDataType
{
	E_UIDataType_None = 0,		///< @a

	E_UIDataType_WCHAR,			///< @a
	E_UIDataType_S64,			///< @a
	E_UIDataType_U64,			///< @a
	E_UIDataType_F64,			///< @a

	E_UIDataType_Count,
};

// -------------------------------
// C_UILeaderboardColumnDescriptor
// -------------------------------
/// holds common data for a leaderboard's column
class C_UILeaderboardColumnDescriptor
{
public:
	C_UILeaderboardColumnDescriptor()
		: m_bActive( false ),
		m_eDataType( E_UIDataType_None ),
		m_sColumnType( 0 ),
		m_Layer( 0 ),
		m_Offset( 0.0f, 0.0f, 0.0f ),
		m_eJustify( E_Justify_TopLeft ),
		m_fWrapSizeX( 0.0f ),
		m_Colour( 0xffffffff ),
		m_Title( "CMN_NOWT" )
	{
		//
	}

public:
	bool				Active()								{ return m_bActive; }			///< @a
	void				Active( const bool bActive )			{ m_bActive = bActive; }		///< @a

	E_UIDataType		DataType()								{ return m_eDataType; }			///< @a
	void				DataType( const E_UIDataType eType )	{ m_eDataType = eType; }		///< @a

	S32					ColumnType()							{ return m_sColumnType; }		///< @a
	void				ColumnType( const S32 sType )			{ m_sColumnType = sType; }		///< @a

	C_TextStringID&		Title()									{ return m_Title; }				///< @a
	void				Title( const C_TextStringID& id )		{ m_Title = id; }				///< @a

	LayerIndex			Layer()									{ return m_Layer; }				///< @a
	void				Layer( const LayerIndex layer )			{ m_Layer = layer; }			///< @a

	Maths::HVec3&		Offset()								{ return m_Offset; }			///< @a
	void				Offset( Maths::HVec3& offset )			{ m_Offset = offset; }			///< @a

	E_Justify			Justify()								{ return m_eJustify; }			///< @a
	void				Justify( const E_Justify justify )		{ m_eJustify = justify; }		///< @a

	F32					WrapSizeX()								{ return m_fWrapSizeX; }		///< @a
	void				WrapSizeX( const F32 fWrapSizeX )		{ m_fWrapSizeX = fWrapSizeX; }	///< @a

	C_UIColour&			Colour()								{ return m_Colour; }			///< @a
	void				Colour( const C_UIColour& colour )		{ m_Colour = colour; }			///< @a

	C_FontStylePasses&	FontStyle()										{ return m_Style; }			///< @a
	void				FontStyle( const C_FontStylePasses& style )		{ m_Style = style; }		///< @a

private:
	bool				m_bActive;
	E_UIDataType		m_eDataType;		// what type of data we hold - WCHAR, S32 etc ...
	S32					m_sColumnType;		// game-specific - e.g. position, arrow, difficulty etc
	C_TextStringID		m_Title;
	LayerIndex			m_Layer;
	Maths::HVec3		m_Offset;
	E_Justify			m_eJustify;
	F32					m_fWrapSizeX;		// for strings
	C_UIColour			m_Colour;
	C_FontStylePasses	m_Style;
};

// -------------------
// C_UILeaderboardData
// -------------------
/// union type of what a specific entry in a leaderboard can be
union C_UILeaderboardData
{
	WCHAR				m_String[ COLUMN_MAX_STRING_SIZE ];
	S64					m_sNumber;
	U64					m_uNumber;
	F64					m_fNumber;
};

// ---------------------
// C_UILeaderboardColumn
// ---------------------
/// storage class for column data
class C_UILeaderboardColumn
{
public:
	C_UILeaderboardColumn()			{  }
	~C_UILeaderboardColumn()		{ ClearData(); }

public:
	void				ClearData()								{ m_Data.Clear(); }		///< clear data list

	void				AddData( const WCHAR* pwData );			///< @a
	void				AddData( const S64 sData );				///< @a
	void				AddData( const U64 uData );				///< @a
	void				AddData( const F64 fData );				///< @a

	WCHAR*				GetDataWCHAR( const U32 uRow )			{ assert( m_ColumnDescriptor.DataType() == E_UIDataType_WCHAR ); return m_Data[ uRow ].m_String; }	///< @a
	S64&				GetDataS64( const U32 uRow ) 			{ assert( m_ColumnDescriptor.DataType() == E_UIDataType_S64 ); return m_Data[ uRow ].m_sNumber; }	///< @a
	U64&				GetDataU64( const U32 uRow ) 			{ assert( m_ColumnDescriptor.DataType() == E_UIDataType_U64 ); return m_Data[ uRow ].m_uNumber; }	///< @a
	F64&				GetDataF64( const U32 uRow ) 			{ assert( m_ColumnDescriptor.DataType() == E_UIDataType_F64 ); return m_Data[ uRow ].m_fNumber; }	///< @a

	C_UILeaderboardColumnDescriptor&	GetColumnDescriptor()	{ return m_ColumnDescriptor; }	///< @a

	bool				IsSortable();							///< returns true if the column's data type is a number
	F64					GetSortableData( const U32 uRow );		///< convert's all numbers to float for comparison

public:
	static const U32																m_uMaxRows = 101;	// enough for friends list

private:
	C_UILeaderboardColumnDescriptor													m_ColumnDescriptor;
	Btl::Vector< C_UILeaderboardData, Btl::VectorParameters_Static< m_uMaxRows > >	m_Data;
};

// ---------------
// C_UILeaderboard
// ---------------
/// A storage class for leaderboard data - has a Setup() to grab column defs and common data from LUA
class C_UILeaderboard
{
public:
	void 				Init();			///< @a
	void 				Setup( lua_State* pState );		///< grab column data from LUA

	void				ClearData();	///< clear data list - but keep column defs

	void				AddDataWCHAR( const U32 uColumn, const WCHAR* pwData )		{ m_ColumnData[ uColumn ].AddData( pwData ); }	///< @a
	void				AddDataS64( const U32 uColumn, const S64 sData )			{ m_ColumnData[ uColumn ].AddData( sData ); }	///< @a
	void				AddDataU64( const U32 uColumn, const U64 uData )			{ m_ColumnData[ uColumn ].AddData( uData ); }	///< @a
	void				AddDataF64( const U32 uColumn, const F64 fData )			{ m_ColumnData[ uColumn ].AddData( fData ); }	///< @a

	void				EndData()													{ m_uRowCount++; m_SortedOrder.PushBack( m_uRowCount - 1 ); }	///< @a

	WCHAR*				GetDataWCHAR( const U32 uColumn, const U32 uRow )			{ return m_ColumnData[ uColumn ].GetDataWCHAR( uRow ); }	///< @a
	S64&				GetDataS64( const U32 uColumn, const U32 uRow )				{ return m_ColumnData[ uColumn ].GetDataS64( uRow ); }		///< @a
	U64&				GetDataU64( const U32 uColumn, const U32 uRow )				{ return m_ColumnData[ uColumn ].GetDataU64( uRow ); }		///< @a
	F64&				GetDataF64( const U32 uColumn, const U32 uRow )				{ return m_ColumnData[ uColumn ].GetDataF64( uRow ); }		///< @a

	WCHAR*				GetSortedDataWCHAR( const U32 uColumn, const U32 uRow )		{ return m_ColumnData[ uColumn ].GetDataWCHAR( m_SortedOrder[ uRow ] ); }	///< @a
	S64&				GetSortedDataS64( const U32 uColumn, const U32 uRow )		{ return m_ColumnData[ uColumn ].GetDataS64( m_SortedOrder[ uRow ] ); }		///< @a
	U64&				GetSortedDataU64( const U32 uColumn, const U32 uRow )		{ return m_ColumnData[ uColumn ].GetDataU64( m_SortedOrder[ uRow ] ); }		///< @a
	F64&				GetSortedDataF64( const U32 uColumn, const U32 uRow )		{ return m_ColumnData[ uColumn ].GetDataF64( m_SortedOrder[ uRow ] ); }		///< @a

	void				Sort();		///< sort depending on "sort_column_primary", "sort_column_secondary", "sort_descending_primary" and "sort_descending_secondary" taken from LUA

	C_UILeaderboardColumnDescriptor&	GetColumnDescriptor( const U32 uColumn )	{ return m_ColumnData[ uColumn ].GetColumnDescriptor(); }	///< @a

	U32					ColumnCount()												{ return m_uColumnCount; }	///< @a
	U32					RowCount()													{ return m_uRowCount; }		///< @a

	char*				Name()														{ return m_pName; }			///< @a

	F32					RowHeight()													{ return m_fRowHeight; }	///< @a
	
protected:
	static const U32	m_uMaxColumns = 10;
	Btl::Vector< C_UILeaderboardColumn, Btl::VectorParameters_Static< m_uMaxColumns > >		m_ColumnData;

	U32					m_uColumnCount;
	U32					m_uRowCount;

	static const U32	m_uMaxNameLength = 64;
	char				m_pName[ m_uMaxNameLength ];

	F32					m_fRowHeight;

	S32					m_sPrimarySortColumn;
	S32					m_sSecondarySortColumn;

	bool				m_bSortDescendingPrimary;
	bool				m_bSortDescendingSecondary;

	Btl::Vector< U32, Btl::VectorParameters_Static< C_UILeaderboardColumn::m_uMaxRows >	>	m_SortedOrder;
};

/// @}

};	// namespace UI

#endif // UILEADERBOARD_H
