// RenderListNode
inline RenderListNode::RenderListNode()
{
}

inline RenderListNode::~RenderListNode()
{
}

inline NodeType RenderListNode::GetNodeType() const
{
	return NODE_TYPE_RENDER_LIST;
}

inline bool RenderListNode::IsNode(NodeType t) const
{
	if (t == NODE_TYPE_RENDER_LIST) return true;
	else return CullNode::IsNode(t);
}

inline RenderListNode* RenderListNode::Clone() const
{
#if !PLATFORM_SPU
	return new RenderListNode(*this);
#endif
}

inline const RenderListNode* RenderListNode::GetPointer(const Node* p)
{
	if (p && p->IsNode(NODE_TYPE_RENDER_LIST))
		return (const RenderListNode*) p;
	else
		return 0;
}

inline RenderListNode* RenderListNode::GetPointer(Node* p)
{
	if (p && p->IsNode(NODE_TYPE_RENDER_LIST))
		return (RenderListNode*) p;
	else
		return 0;
}

// primitives
inline S32 RenderListNode::GetNumPrimitives() const
{
	return m_renderData.primitives.Size();
}

inline const RenderListNode::Primitive&	RenderListNode::GetPrimitive(const S32 index) const
{
	return m_renderData.primitives[index];
}

inline const Bounds& RenderListNode::GetPrimitiveBounds(const S32 index) const
{
	return m_sceneGraphData.primitiveBounds[index];
}

inline const Bounds* RenderListNode::GetPrimitiveBounds() const
{
	return m_sceneGraphData.primitiveBounds.First();
}

inline const U8* RenderListNode::GetPrimitivePvs(S32 cell) const
{
	return m_sceneGraphData.primitivePvs.GetPvsData(cell);
}

// instances
inline S32 RenderListNode::GetNumInstances() const
{
	return m_renderData.instances.Size();
}

inline const RenderListNode::Instance&	RenderListNode::GetInstance(const S32 index) const
{
	return m_renderData.instances[index];
}

inline const Bounds& RenderListNode::GetInstanceBounds(const S32 index) const
{
	return m_sceneGraphData.instanceBounds[index];
}

inline const Bounds* RenderListNode::GetInstanceBounds() const
{
	return m_sceneGraphData.instanceBounds.First();
}

inline const U8* RenderListNode::GetInstancePvs(S32 cell) const
{
#if !PLATFORM_SPU
	return m_sceneGraphData.instancePvs.GetPvsData(cell);
#endif
}

inline S32 RenderListNode::GetNumBasePrimitives() const
{
	return m_renderData.basePrimitives.Size();
}

// instance batch
inline S32 RenderListNode::GetNumInstanceBatches() const
{
	return m_renderData.instanceBatches.Size();
}

inline const RenderListNode::InstanceBatch&	RenderListNode::GetInstanceBatch(const S32 index) const
{
	return m_renderData.instanceBatches[index];
}

inline const Bounds& RenderListNode::GetInstanceBatchBounds(const S32 index) const
{
	return m_sceneGraphData.instanceBatchBounds[index];
}

inline const Bounds* RenderListNode::GetInstanceBatchBounds() const
{
	return m_sceneGraphData.instanceBatchBounds.First();
}

inline const RenderListNode::RenderPassInfo& RenderListNode::GetRenderPassInfo(RenderPass renderPass) const
{
	return m_sceneGraphData.renderPassInfo[renderPass];
}

inline const S32 RenderListNode::GetRenderPassInfoSize() const
{
	return m_sceneGraphData.renderPassInfo.Size();
}

#if !PLATFORM_SPU
inline S32 RenderListNode::GetNumIndexBuffers() const
{
	return m_streamerData.indexBufferSource.Size();
}

inline S32 RenderListNode::GetNumVertexBuffers() const
{
	return m_streamerData.vertexBufferSource.Size();
}

inline S32 RenderListNode::GetNumTextures() const
{
	return m_streamerData.textureSource.Size();
}

inline ResourceIndex RenderListNode::GetIndexBufferSource(S32 i) const
{
	return m_streamerData.indexBufferSource[i];
}

inline ResourceIndex RenderListNode::GetVertexBufferSource(S32 i) const
{
	return m_streamerData.vertexBufferSource[i];
}

inline ResourceIndex RenderListNode::GetTextureSource(S32 i) const
{
	return m_streamerData.textureSource[i];
}

/// Get a bit vector representing visiblity of primitives with respect to portals.
inline const Btl::BitVector& RenderListNode::GetPortalPrimitiveVisibilityBitVector() const
{
	return m_sceneGraphData.portalVisiblePrimitives;
}

/// Get a bit vector representing visiblity of instances with respect to portals.
inline const Btl::BitVector& RenderListNode::GetPortalInstanceVisibilityBitVector() const
{
	return m_sceneGraphData.portalVisibleInstances;
}
#endif


