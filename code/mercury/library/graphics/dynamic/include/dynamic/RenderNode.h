#ifndef DYNAMIC_RENDER_NODE_H
#define DYNAMIC_RENDER_NODE_H

#include <SceneGraph/RenderNode.h>

namespace DynamicEffect
{

class C_RenderNode;
class I_DynamicEffect;

class STOREALIGN16 RenderNodePtrs : public Btl::Vector<const C_RenderNode*, Btl::VectorParameters_Static<10> > {};

// composite render node is many render nodes combined into a single draw call, contains all the necessary info to either draw the effect from prebuilt data
// or get at the actual effect data in order to create the effect on the fly
struct C_CompositeRenderNode : public SceneGraph::I_RenderNode
{
	C_CompositeRenderNode() : SceneGraph::I_RenderNode(), pEffect(NULL), flags(0), iVertexBuffer(0xFFFFFFFF), nNumElements(0) {}
	~C_CompositeRenderNode() {}

	void	OnRender(SceneGraph::C_Context& context) const {}
	void	DebugRender(const SceneGraph::C_VisibleNode& node) const;

	const I_DynamicEffect*			pEffect;
	MathsStorage::Float2			lodFade;
	U32								flags;				// other batch breaking stuff, eg lights, shadows, etc
	U32								iVertexBuffer;		// block index used if batch uses prebuilt vertex data
	U32								nNumElements;

	RenderNodePtrs					blocks;				// blocks and particle num needed if batch uses BeginEndVertices/XPS/DrawPrimitiveUP etc

	friend class C_Renderer;
};

typedef Btl::Vector<C_CompositeRenderNode>	CompositeRenderNodes;

};
#endif // DYNAMIC_RENDER_NODE_TYPE_H
