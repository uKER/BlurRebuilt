#include "Stdafx.h"

#include "UITestLeaderboardNEW.h"

#include <ui/Game/UILocaliser.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI 
#pragma optimize("", off) 
#endif

namespace UI
{

void UITestLeaderboardNEW::Init()
{
	m_Leaderboard.Init();

	UIStaticText::Init();
}

void UITestLeaderboardNEW::Setup( lua_State* pState )
{
	m_Leaderboard.Setup( pState );

	// our lua stuff here

	UIStaticText::Setup( pState );

	// fill the leaderboard with some test data
	// -- this would come from the game somewhere

	m_Leaderboard.AddDataU64( 0, 0 );
	m_Leaderboard.AddDataS64( 1, 1 );
	m_Leaderboard.AddDataS64( 2, 5 );
	m_Leaderboard.EndData();

	m_Leaderboard.AddDataU64( 0, 0 );
	m_Leaderboard.AddDataS64( 1, 3 );
	m_Leaderboard.AddDataS64( 2, 6 );
	m_Leaderboard.EndData();

	m_Leaderboard.AddDataU64( 0, 0 );
	m_Leaderboard.AddDataS64( 1, 4 );
	m_Leaderboard.AddDataS64( 2, 7 );
	m_Leaderboard.EndData();

	m_Leaderboard.AddDataU64( 0, 0 );
	m_Leaderboard.AddDataS64( 1, 3 );
	m_Leaderboard.AddDataS64( 2, 8 );
	m_Leaderboard.EndData();

	m_Leaderboard.Sort();
}

void UITestLeaderboardNEW::Render()
{
	UIStaticText::Render();

	// each row
	for ( U32 i = 0; i < m_Leaderboard.RowCount(); i++ )
	{
		// each column
		for ( U32 j = 0; j < m_Leaderboard.ColumnCount(); j++ )
		{
			C_UILeaderboardColumnDescriptor& columnDescriptor = m_Leaderboard.GetColumnDescriptor( j );

			if ( columnDescriptor.ColumnType() == 0 )
			{
				// position
				const WCHAR* pString = C_UILocaliser::Instance().ConvertNumberToPosition( i + 1 );

				gFont.DrawText( columnDescriptor.Layer(),
					Maths::HVec3( m_Position.x, m_Position.y + ( i * m_Leaderboard.RowHeight() ), m_Position.z ) + columnDescriptor.Offset(),
					columnDescriptor.Justify(), pString, columnDescriptor.Colour(), 
					m_Size.y, columnDescriptor.FontStyle(), columnDescriptor.WrapSizeX(), m_Panel );
			}
			else if ( columnDescriptor.ColumnType() == 1 )
			{
				const WCHAR* pString = C_UILocaliser::Instance().ConvertNumberToCommas( m_Leaderboard.GetSortedDataS64( j, i ) );

				gFont.DrawText( columnDescriptor.Layer(),
					Maths::HVec3( m_Position.x, m_Position.y + ( i * m_Leaderboard.RowHeight() ), m_Position.z ) + columnDescriptor.Offset(),
					columnDescriptor.Justify(), pString, columnDescriptor.Colour(), 
					m_Size.y, columnDescriptor.FontStyle(), columnDescriptor.WrapSizeX(), m_Panel );
			}
			else if ( columnDescriptor.ColumnType() == 2 )
			{
				// score
				const WCHAR* pString = C_UILocaliser::Instance().ConvertNumberToCommas( m_Leaderboard.GetSortedDataS64( j, i ) );

				gFont.DrawText( columnDescriptor.Layer(),
					Maths::HVec3( m_Position.x, m_Position.y + ( i * m_Leaderboard.RowHeight() ), m_Position.z ) + columnDescriptor.Offset(),
					columnDescriptor.Justify(), pString, columnDescriptor.Colour(), 
					m_Size.y, columnDescriptor.FontStyle(), columnDescriptor.WrapSizeX(), m_Panel );
			}
		}
	}
}

};	// namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on) 
#endif
