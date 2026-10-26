#ifndef SCENE_GRAPH_RENDERNODE_INL
#define SCENE_GRAPH_RENDERNODE_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace SceneGraph
{

// C_PassCull

INLINE const U32 C_PassCull::RenderPassEnabled(const U32 renderPassFlag) const
{
	return m_RenderPassFlags & renderPassFlag;
}

INLINE const U32 C_PassCull::RenderPasses() const
{
	return m_RenderPassFlags;
}

INLINE const U32 C_PassCull::RenderFlagEnabled(const U32 renderFlag) const
{
	return m_RenderFlags & renderFlag;
}

INLINE const U32 C_PassCull::RenderFlags() const
{
	return m_RenderFlags;
}

INLINE void C_PassCull::EnableRenderPass(const U32 passFlag)
{
	m_RenderPassFlags |= passFlag;
}

INLINE void C_PassCull::DisableRenderPass(const U32 passFlag)
{
	m_RenderPassFlags &= ~passFlag;
}

INLINE void C_PassCull::EnableRenderFlag(const U32 renderFlag)
{
	m_RenderFlags |= renderFlag;
}

INLINE void C_PassCull::DisableRenderFlag(const U32 renderFlag)
{
	m_RenderFlags &= ~renderFlag;
}

} // namespace SceneGraph

#endif // SCENE_GRAPH_RENDERNODE_INL
