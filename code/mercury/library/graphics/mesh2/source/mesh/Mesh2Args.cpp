#include "stdafx.h"
#include "Mesh2/Mesh/Mesh2Args.h"

namespace Render
{

// C_Mesh2Param

C_Mesh2Param::C_Mesh2Param()
{
	m_eaMeshPtr						= 0;
	m_eaRingBufferPtr				= 0;
	m_eaRingBufferCurOffsetPtr		= 0;
	m_eaJumpToSelfPtr				= 0;
	m_JumpToSelfReturnOffset		= 0;
}

} // namespace Render
