#ifndef SCENE_GRAPH_RENDER_PASS_H
#define SCENE_GRAPH_RENDER_PASS_H

#ifdef _MSC_VER
#pragma once
#endif

#if !RENDERING_SPU_HACK
#include "Display/Device/StencilState.h"
#endif

namespace SceneGraph
{

class I_VisibleNodeListSorter;
struct RenderStates;

enum E_DrawOrder
{
	E_DrawOrder_PrimitivesInstances,
	E_DrawOrder_InstancesPrimitives
};

class ALIGN16 C_RenderPass
{
public:
	C_RenderPass();
	~C_RenderPass();

	U32								GetFlags() const { return m_Flags; }
	S32								GetIndex() const { return m_Index; }
	U32								GetVolatileInfoFlags() const { return m_VolatileInfoFlags; }

	U32								GetRenderingDataPass() const { return m_RenderingDataPass; }
	
	void							SetSubtechniqueHandle(const U32 subtechnique) { m_Subtechnique = subtechnique; }
	U32								GetSubtechniqueHandle() const { return m_Subtechnique; }

	void							SetSubtechniqueMask(const U32 subtechniqueMask) { m_SubtechniqueMask = ~subtechniqueMask; }
	U32								GetSubtechniqueMask() const { return m_SubtechniqueMask; }

	void							SetSorter(I_VisibleNodeListSorter* pSorter);
	const I_VisibleNodeListSorter*	GetSorter() const { return m_pSorter; }

	void							SetRenderStates(const RenderStates* pRenderStates) { BIZ_ASSERT_SPEED(pRenderStates); m_pRenderStates = pRenderStates; }
	const RenderStates*				GetRenderStates() const { return m_pRenderStates; }

	void							EnableVolatileInfoFlag(const U32 volatileInfoFlag) { m_VolatileInfoFlags |= volatileInfoFlag; }
	void							DisableVoltaileInfoFlag(const U32 volatileInfoFlag) { m_VolatileInfoFlags &= ~volatileInfoFlag; }

	void							SetDrawOrder(E_DrawOrder drawOrder) { m_DrawOrder = drawOrder; }
	const E_DrawOrder				GetDrawOrder() const { return m_DrawOrder; }

	friend class C_SceneGraph;

private:
	U32								m_Flags;
	S32								m_Index;
	U32								m_VolatileInfoFlags;	// what volatile info needs to be generated to go with the nodes in this pass
	E_DrawOrder						m_DrawOrder;

	// TODO: remove this and replace with more generic render states
	U32								m_RenderingDataPass;
	U32								m_Subtechnique;
	U32								m_SubtechniqueMask;

	const RenderStates*				m_pRenderStates;

	I_VisibleNodeListSorter*		m_pSorter;
};

} // namespace SceneGraph

#endif // SCENE_GRAPH_H
