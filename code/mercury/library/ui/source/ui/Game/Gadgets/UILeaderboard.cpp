#include "stdafx.h"

#include "ui/Game/Gadgets/UILeaderboard.h"

#include "ui/Game/UILua.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

namespace UI
{

// ---------------------
// C_UILeaderboardColumn
// ---------------------
void C_UILeaderboardColumn::AddData( const WCHAR* pwData )
{
	assert( m_ColumnDescriptor.Active() && ( m_ColumnDescriptor.DataType() == E_UIDataType_WCHAR ) );
	m_Data.PushBack( C_UILeaderboardData() );
	C_UILeaderboardData& data = m_Data.Back();
	WideStrCopy( data.m_String, COLUMN_MAX_STRING_SIZE, pwData );
}

void C_UILeaderboardColumn::AddData( const S64 sData )
{
	assert( m_ColumnDescriptor.Active() && ( m_ColumnDescriptor.DataType() == E_UIDataType_S64 ) );
	m_Data.PushBack( C_UILeaderboardData() );
	C_UILeaderboardData& data = m_Data.Back();
	data.m_sNumber = sData;
}

void C_UILeaderboardColumn::AddData( const U64 uData )
{
	assert( m_ColumnDescriptor.Active() && ( m_ColumnDescriptor.DataType() == E_UIDataType_U64 ) );
	m_Data.PushBack( C_UILeaderboardData() );
	C_UILeaderboardData& data = m_Data.Back();
	data.m_uNumber = uData;
}

void C_UILeaderboardColumn::AddData( const F64 fData )
{
	assert( m_ColumnDescriptor.Active() && ( m_ColumnDescriptor.DataType() == E_UIDataType_F64 ) );
	m_Data.PushBack( C_UILeaderboardData() );
	C_UILeaderboardData& data = m_Data.Back();
	data.m_fNumber = fData;
}

bool C_UILeaderboardColumn::IsSortable()
{
	return ( 
		( m_ColumnDescriptor.DataType() == E_UIDataType_S64 ) ||
		( m_ColumnDescriptor.DataType() == E_UIDataType_U64 ) ||
		( m_ColumnDescriptor.DataType() == E_UIDataType_F64 )
	);
}

F64	C_UILeaderboardColumn::GetSortableData( const U32 uRow )
{
	// everything is just a float - hmmmm ...

	if ( m_ColumnDescriptor.DataType() == E_UIDataType_S64 )
		return (F64)GetDataS64( uRow );

	if ( m_ColumnDescriptor.DataType() == E_UIDataType_U64 )
		return (F64)GetDataU64( uRow );

	if ( m_ColumnDescriptor.DataType() == E_UIDataType_F64 )
		return (F64)GetDataF64( uRow );

	return 0.0f;
}

// ---------------
// C_UILeaderboard
// ---------------
void C_UILeaderboard::Init()
{
	m_uColumnCount = 0;
	m_uRowCount = 0;

	m_pName[ 0 ] = '\0';

	m_fRowHeight = 0.0f;

	m_sPrimarySortColumn = -1;
	m_sSecondarySortColumn = -1;

	m_bSortDescendingPrimary = true;
	m_bSortDescendingSecondary = true;
}

void C_UILeaderboard::Setup( lua_State* pState )
{
	if( !pState )
	{
		LOGWARN( g_Log, DBSTR "This gadget cannot be creasted without lua");
		return;
	}

	// leaderboard name
	const char *string;
	if ( Lua::DataMemberType( pState, "name" ) == LUA_TSTRING )
	{
		Lua::GetDataMember( pState, "name", string );
		STRCPY( m_pName, m_uMaxNameLength, string );
	}

	// row height
	if ( Lua::DataMemberType( pState, "row_height" ) == LUA_TNUMBER )
		Lua::GetDataMember( pState, "name", m_fRowHeight );

	// sort columns
	if ( Lua::DataMemberType( pState, "sort_column_primary" ) == LUA_TNUMBER )
		Lua::GetDataMember( pState, "sort_column_primary", m_sPrimarySortColumn );

	if ( Lua::DataMemberType( pState, "sort_column_secondary" ) == LUA_TNUMBER )
	{
		Lua::GetDataMember( pState, "sort_column_secondary", m_sSecondarySortColumn );
		m_sSecondarySortColumn--;
	}

	if ( m_sPrimarySortColumn < 0 )
		m_sPrimarySortColumn = -1;
	if ( m_sSecondarySortColumn < 0 )
		m_sSecondarySortColumn = -1;

	if ( Lua::DataMemberType( pState, "sort_descending_primary" ) == LUA_TBOOLEAN )
		Lua::GetDataMember( pState, "sort_descending_primary", m_bSortDescendingPrimary );
	if ( Lua::DataMemberType( pState, "sort_descending_secondary" ) == LUA_TBOOLEAN )
		Lua::GetDataMember( pState, "sort_descending_secondary", m_bSortDescendingSecondary );

	// columns descriptions
	if ( Lua::DataMemberType( pState, "columns" ) == LUA_TTABLE )
	{
		lua_pushstring( pState, "columns" );
		lua_gettable( pState, -2 );

		m_uColumnCount = (U32)luaL_getn( pState, -1 );

		if ( m_uColumnCount > m_uMaxColumns )
		{
			LOGWARN( g_Log, DBSTR "Too many columns. MAX is ( %d ), you have ( %d )! Resetting to ( %d ).", m_uMaxColumns, m_uColumnCount, m_uMaxColumns );
			m_uColumnCount = m_uMaxColumns;
		}

		// grab each column

		for ( U32 i = 1; i <= m_uColumnCount; i++ )
		{
			m_ColumnData.PushBack( C_UILeaderboardColumn() );	// new column

			lua_rawgeti( pState, -1, i ); // push on index [ i ]
			
			if ( lua_type( pState, -1 ) == LUA_TTABLE )
			{
				C_UILeaderboardColumnDescriptor& columnDescriptor = m_ColumnData[ i - 1 ].GetColumnDescriptor();

				columnDescriptor.Active( true );

				if ( Lua::DataMemberType( pState, "active" ) == LUA_TBOOLEAN )
				{
					bool active;
					Lua::GetDataMember( pState, "active", active );

					columnDescriptor.Active( active );
				}
				if ( Lua::DataMemberType( pState, "data_type" ) == LUA_TNUMBER )
				{
					S32 data_type;
					Lua::GetDataMember( pState, "data_type", data_type );

					columnDescriptor.DataType( (E_UIDataType)data_type );
				}
				if ( Lua::DataMemberType( pState, "column_type" ) == LUA_TNUMBER )
				{
					S32 column_type;
					Lua::GetDataMember( pState, "column_type", column_type );

					columnDescriptor.ColumnType( column_type );
				}

				if ( Lua::DataMemberType( pState, "title" ) == LUA_TNUMBER )
				{
					U32 title;
					Lua::GetDataMember( pState, "title", title );

					columnDescriptor.Title( C_TextStringID( title ) );
				}
				if ( Lua::DataMemberType( pState, "layer" ) == LUA_TNUMBER )
				{
					U8 layer;
					Lua::GetDataMember( pState, "layer", layer );

					columnDescriptor.Layer( (LayerIndex)layer );
				}

				Maths::HVec3 offset( 0.0f, 0.0f, 0.0f );

				if( Lua::DataMemberType( pState, "offsetx" ) == LUA_TNUMBER )
				{
					F32 x;
					Lua::GetDataMember( pState, "offsetx", x );
					
					offset.X( x );
				}
				if( Lua::DataMemberType( pState, "offsety" ) == LUA_TNUMBER )
				{
					F32 y;
					Lua::GetDataMember( pState, "offsety", y );

					offset.X( y );
				}
				if( Lua::DataMemberType( pState, "offsety" ) == LUA_TNUMBER )
				{
					F32 z;
					Lua::GetDataMember( pState, "offsety", z );

					offset.X( z );
				}
				columnDescriptor.Offset( offset );

				if ( Lua::DataMemberType( pState, "justify" ) == LUA_TNUMBER )
				{
					S32 justify;
					Lua::GetDataMember( pState, "justify", justify );

					columnDescriptor.Justify( (E_Justify)justify );
				}
				if ( Lua::DataMemberType( pState, "wrap_size" ) == LUA_TNUMBER )
				{
					F32 wrap_size;

					Lua::GetDataMember( pState, "wrap_size", wrap_size );
					columnDescriptor.WrapSizeX( wrap_size );
				}
				if ( Lua::DataMemberType( pState, "colour" ) == LUA_TSTRING )
				{
					const char* colour;
					Lua::GetDataMember( pState, "colour", colour );
					columnDescriptor.Colour().Set( colour );
				}

				columnDescriptor.FontStyle().Setup( pState );
			}
			else
				LOGWARN( g_Log, DBSTR "Column table ( %d ) is INVALID", i );

			lua_pop( pState, 1 );	// pop index [ i ] off
		}

		lua_pop( pState, 1 ); // pop the columns table off

	}
	else
		LOGERROR( g_Log, DBSTR "You need to define some columns!" );
}

void C_UILeaderboard::ClearData()
{
	for ( U32 i = 0; i < m_uColumnCount; i++ )
		m_ColumnData[ i ].ClearData();
}

void C_UILeaderboard::Sort()
{
	if ( ( m_sPrimarySortColumn == -1 ) || !m_ColumnData[ m_sPrimarySortColumn ].IsSortable() )
		return;

	bool bSecondaryValid = true;

	if ( ( m_sSecondarySortColumn == -1 ) || !m_ColumnData[ m_sSecondarySortColumn ].IsSortable() )
		bSecondaryValid = false;

	C_UILeaderboardColumn& p = m_ColumnData[ m_sPrimarySortColumn ];
	C_UILeaderboardColumn& s = bSecondaryValid ? m_ColumnData[ m_sSecondarySortColumn ] : m_ColumnData[ m_sPrimarySortColumn ];

	// bah - just a selection sort

	for ( U32 i = 0; i < m_uRowCount; i++ )
		m_SortedOrder[ i ] = i;

	for ( U32 i = 0; i < m_uRowCount; i++ )
	{
		U32 uMax = i;

		for ( U32 j = i; j < m_uRowCount; j++ )
		{
			if ( m_bSortDescendingPrimary ? 
				( p.GetSortableData( m_SortedOrder[ j ] ) > p.GetSortableData( m_SortedOrder[ uMax ] ) ) : 
				( p.GetSortableData( m_SortedOrder[ j ] ) < p.GetSortableData( m_SortedOrder[ uMax ] ) ) )
			{
				uMax = j;
			}
			else if ( bSecondaryValid && ( p.GetSortableData( m_SortedOrder[ j ] ) == p.GetSortableData( m_SortedOrder[ uMax ] ) ) )
			{
				if ( m_bSortDescendingSecondary ? 
					( s.GetSortableData( m_SortedOrder[ j ] ) > s.GetSortableData( m_SortedOrder[ uMax ] ) ) : 
					( s.GetSortableData( m_SortedOrder[ j ] ) < s.GetSortableData( m_SortedOrder[ uMax ] ) ) )
				{
					uMax = j;
				}
			}
		}

		if ( uMax != i )
		{
			U32 uTemp = m_SortedOrder[ i ];
			m_SortedOrder[ i ] = m_SortedOrder[ uMax ];
			m_SortedOrder[ uMax ] = uTemp;
		}
	}
}

};	// namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on) 
#endif
