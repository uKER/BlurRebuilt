#include <stdafx.h>
#include <Portal/RoomGridCell.h>

using namespace Portal;


// Default c'tor for grid cell.
C_RoomGridCell::C_RoomGridCell() :
	m_iStartIndex(0),
	m_iEndIndex(0)
{
	return;
}

// -----------------------------------------------------------------

// D'tor.
C_RoomGridCell::~C_RoomGridCell()
{
	// Placeholder method.
	return;
}

// -----------------------------------------------------------------

// Return the end index into the sub-space list.
NOALIAS const U32 C_RoomGridCell::GetEndIndex() const
{
	return m_iEndIndex;
}

// -----------------------------------------------------------------

// Return the start index into the sub-space list.
NOALIAS const U32 C_RoomGridCell::GetStartIndex() const
{
	return m_iStartIndex;
}

// -----------------------------------------------------------------

// Return the number of sub-spaces that this grid cell contains.
NOALIAS const U32 C_RoomGridCell::GetSubSpaceCount() const
{
	return m_iEndIndex - m_iStartIndex;
}

// -----------------------------------------------------------------
