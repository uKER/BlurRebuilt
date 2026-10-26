#include <stdafx.h>
#include <Portal/BuilderRoomContainmentSubSpace.h>

using namespace Portal;


// Default c'tor.
C_BuilderRoomContainmentSubSpace::C_BuilderRoomContainmentSubSpace(U32 iStart, U32 iEnd) :
	m_iStartIndex(iStart),
	m_iEndIndex(iEnd)
{
	return;
}

// -----------------------------------------------------------------

// D'tor.
C_BuilderRoomContainmentSubSpace::~C_BuilderRoomContainmentSubSpace()
{
	return;
}

// -----------------------------------------------------------------

// Return the start index.
const U32 C_BuilderRoomContainmentSubSpace::GetStartIndex() const
{
	return m_iStartIndex;
}

// -----------------------------------------------------------------

// Return the end index.
const U32 C_BuilderRoomContainmentSubSpace::GetEndIndex() const
{
	return m_iEndIndex;
}
