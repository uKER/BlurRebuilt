#ifndef SCENE_GRAPH_CULLNODE_INL
#define SCENE_GRAPH_CULLNODE_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace SceneGraph
{

INLINE const bool C_BoundsCull::CullFlagEnabled(const U32 cullFlag) const
{
	return (m_CullFlags & cullFlag) != 0;
}

INLINE void C_BoundsCull::EnableCullFlag(const U32 cullFlag)
{
	m_CullFlags |= cullFlag;
}

INLINE void C_BoundsCull::DisableCullFlag(const U32 cullFlag)
{
	m_CullFlags &= ~cullFlag;
}

} // namespace SceneGraph

#endif // SCENE_GRAPH_CULLNODE_INL
