#ifndef RENDER_MESH2_ARGS_H
#define RENDER_MESH2_ARGS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Render
{

//! C_Mesh2Param

struct C_Mesh2Param
{
	C_Mesh2Param();

	U32			m_eaMeshPtr;
	U32			m_eaRingBufferPtr;
	U32			m_eaRingBufferCurOffsetPtr;
	U32			m_eaJumpToSelfPtr;
	U32			m_JumpToSelfReturnOffset;
};

} // namespace Render

#endif // RENDER_MESH2_ARGS_H
