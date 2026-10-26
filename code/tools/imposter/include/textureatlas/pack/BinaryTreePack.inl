template<typename Node>
C_TreePackNode<Node>::C_TreePackNode(const ImageSize& size, U32 border)
: m_pNode(0)
, m_bounds(0, 0, size.x, size.y)
, m_border(border)
{
	memset(&m_pChildren, 0, sizeof(C_TreePackNode<Node>*)*2);
}

template<typename Node>
C_TreePackNode<Node>::C_TreePackNode(const ImageRect& bounds, U32 border)
: m_pNode(0)
, m_bounds(bounds)
, m_border(border)
{
	memset(&m_pChildren, 0, sizeof(C_TreePackNode<Node>*)*2);
}


template<typename Node>
C_TreePackNode<Node>::~C_TreePackNode()
{
	Clear();
}

template<typename Node>
void C_TreePackNode<Node>::Clear()
{
	SAFE_DELETE_PTR(m_pChildren[0]);
	SAFE_DELETE_PTR(m_pChildren[1]);

	m_pNode = 0;
}

template<typename Node>
U32 C_TreePackNode<Node>::GetBorderSize() const
{
	return m_border;
}

template<typename Node>
const ImageRect& C_TreePackNode<Node>::GetBounds() const
{
	return m_bounds;
}

template<typename Node>
const C_TreePackNode<Node>* C_TreePackNode<Node>::Insert(const Node* pNode)
{
	//return if our space has already been allocated, or is too small for the image
	if (m_pNode || !pNode || !pNode->size.x || !pNode->size.y)
		return 0;

	// Recurse to child nodes if not a leaf
	if (m_pChildren[0]!=0 && m_pChildren[1]!=0)
	{
		const C_TreePackNode<Node> *pResult;

		pResult = m_pChildren[0]->Insert(pNode);
		if (!pResult)
			pResult = m_pChildren[1]->Insert(pNode);

		return pResult;
	}

	//check image can fit
	if (pNode->size.x>m_bounds.w || pNode->size.y>m_bounds.h)
		return 0;


	//perfect fit?
	if (pNode->size.x==m_bounds.w && pNode->size.y==m_bounds.h)
	{
		m_pNode = pNode;
		return this;
	}

	//spawn new child tree nodes
	ImageSize delta(m_bounds.w-pNode->size.x, m_bounds.h-pNode->size.y);
	if (delta.x >= delta.y)
	{
		//vertical split
		m_pChildren[0] = new C_TreePackNode<Node>(
			ImageRect(m_bounds.x, m_bounds.y, pNode->size.x, m_bounds.h), m_border);

		m_pChildren[1] = new C_TreePackNode<Node>(
			ImageRect(m_bounds.x+pNode->size.x+m_border, m_bounds.y, m_bounds.w-pNode->size.x-m_border, m_bounds.h), m_border);
	}
	else
	{
		//horizontal split
		m_pChildren[0] = new C_TreePackNode<Node>(
			ImageRect(m_bounds.x, m_bounds.y, m_bounds.w, pNode->size.y), m_border);

		m_pChildren[1] = new C_TreePackNode<Node>(
			ImageRect(m_bounds.x, m_bounds.y+pNode->size.y+m_border, m_bounds.w, m_bounds.h-pNode->size.y-m_border), m_border);
	}

	return m_pChildren[0]->Insert(pNode);
}
