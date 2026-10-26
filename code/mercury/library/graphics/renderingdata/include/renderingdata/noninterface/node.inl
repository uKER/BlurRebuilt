// Node
inline Node::Node() : m_pScene(0)
{
}

inline void Node::SetName(const Btl::String& s)
{
#if !PLATFORM_SPU
	m_name = s;
#endif
}

inline const Btl::String& Node::GetName() const
{
#if !PLATFORM_SPU
	return m_name;
#endif
}

inline NodeType Node::GetNodeType() const
{
	return NODE_TYPE_EMPTY;
}

inline bool Node::IsNode(NodeType t) const
{
	if (t == NODE_TYPE_EMPTY) return true;
	else return false;
}

inline Node* Node::Clone() const
{
#if !PLATFORM_SPU
	return new Node(*this);
#endif
}

inline S32 Node::GetNumSubNodes() const
{
	return m_subNodes.Size();
}

inline ResourceIndex Node::GetSubNode(S32 i) const
{
	return m_subNodes[i];
}

inline void Node::CleanSubNodes()
{
	Btl::Remove(m_subNodes, ResourceIndex::Null());
}

inline Scene* Node::GetScenePtr() const
{
	return m_pScene;
}

inline const ResourceIndex& Node::GetLocation() const
{
	return m_location;
}

inline void Node::SetParent(const ResourceIndex& i)
{
	m_parent = i;
}

inline void Node::SetLocation(S32 block, S32 index)
{
	assert(index > -1);
	m_location = ResourceIndex::BlockIndex(block, index);
}

inline ResourceIndex Node::GetParent() const
{
	return m_parent;
}

// Cull Node
inline CullNode::CullNode()
{
	m_bounds.SetUniversal();
	m_groupIndex = -1;
}

inline NodeType CullNode::GetNodeType() const
{
	return NODE_TYPE_CULL;
}

inline bool CullNode::IsNode(NodeType t) const
{
	if (t == NODE_TYPE_CULL) return true;
	else return Node::IsNode(t);
}

inline CullNode* CullNode::Clone() const
{
#if !PLATFORM_SPU
	return new CullNode(*this);
#endif
}

inline const Bounds& CullNode::GetBounds() const
{
	return m_bounds;
}

inline Bounds& CullNode::GetBounds()
{
	return m_bounds;
}

inline void CullNode::SetBounds(const Bounds& bounds)
{
	m_bounds = bounds;
}

inline const Btl::BitVector& CullNode::GetPvsBits() const
{
	return m_pvsBits;
}

inline Btl::BitVector& CullNode::GetPvsBits()
{
	return m_pvsBits;
}

inline const Btl::BitVector& CullNode::GetPortalBits() const
{
	return m_portalBits;
}

inline Btl::BitVector& CullNode::GetPortalBits()
{
	return m_portalBits;
}

inline S32 CullNode::GetGroupIndex() const
{
	return m_groupIndex;
}

inline void CullNode::SetGroupIndex(S32 i)
{
	m_groupIndex = i;
}

// Root Node
inline RootNode::RootNode()
{
	SetLocation(-1, 0);
}

inline NodeType RootNode::GetNodeType() const
{
	return NODE_TYPE_ROOT;
}

inline bool RootNode::IsNode(NodeType t) const
{
	if (t == NODE_TYPE_ROOT) return true;
	else return CullNode::IsNode(t);
}

inline RootNode* RootNode::Clone() const
{
#if !PLATFORM_SPU
	return new RootNode(*this);
#endif
}

// Group Node
inline NodeType GroupNode::GetNodeType() const
{
	return NODE_TYPE_GROUP;
}

inline bool GroupNode::IsNode(NodeType t) const
{
	if (t == NODE_TYPE_GROUP) return true;
	else return CullNode::IsNode(t);
}

inline GroupNode* GroupNode::Clone() const
{
#if !PLATFORM_SPU
	return new GroupNode(*this);
#endif
}

// Lod Node
inline NodeType LodNode::GetNodeType() const
{
	return NODE_TYPE_LOD;
}

inline bool LodNode::IsNode(NodeType t) const
{
	if (t == NODE_TYPE_LOD) return true;
	else return CullNode::IsNode(t);
}

inline LodNode* LodNode::Clone() const
{
#if !PLATFORM_SPU
	return new LodNode(*this);
#endif
}

