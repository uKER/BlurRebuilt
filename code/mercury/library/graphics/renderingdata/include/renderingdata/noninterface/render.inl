// RenderNode
inline RenderNode::RenderNode(const RenderNode& renderNode) : CullNode(renderNode)
{
	m_object = Btl::Clone(renderNode.m_object);
}

inline RenderNode& RenderNode::operator = (const RenderNode& renderNode)
{
	static_cast<CullNode&>(*this) = renderNode;
	m_object = Btl::Clone(renderNode.m_object);
	return *this;
}

inline NodeType RenderNode::GetNodeType() const
{
	return NODE_TYPE_RENDER;
}

inline bool RenderNode::IsNode(NodeType t) const
{
	if (t == NODE_TYPE_RENDER) return true;
	else return CullNode::IsNode(t);
}

inline RenderNode* RenderNode::Clone() const
{
	return new RenderNode(*this);
}

inline void RenderNode::SetRenderObject(Btl::AutoPointer<RenderObject> p)
{
	m_object = p;
}

inline const RenderObject* RenderNode::GetRenderObjectPtr() const
{
	return (const RenderObject*) Pointer(m_object);
}

inline RenderObject* RenderNode::GetRenderObjectPtr()
{
	return (RenderObject*) Pointer(m_object);
}

inline const RenderNode* RenderNode::GetPointer(const Node* p)
{
	if (p && p->IsNode(NODE_TYPE_RENDER))
		return (const RenderNode*) p;
	else
		return 0;
}

inline RenderNode* RenderNode::GetPointer(Node* p)
{
	if (p && p->IsNode(NODE_TYPE_RENDER))
		return (RenderNode*) p;
	else
		return 0;
}

// Render Object
inline RenderObjectType RenderObject::GetObjectType() const
{
	return RENDER_OBJECT_UNKNOWN;
}

inline RenderObject* RenderObject::Clone() const
{
	return new RenderObject(*this);
}

