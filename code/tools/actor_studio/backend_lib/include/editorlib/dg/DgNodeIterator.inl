namespace EditorLib
{

C_DgNodeConstIterator::C_DgNodeConstIterator(const C_DgNodeConstIterator& rhs)
{
	*this = rhs;
}

const C_DgNodeConstIterator& C_DgNodeConstIterator::operator=(const C_DgNodeConstIterator& rhs)
{
	m_pDgNodes = rhs.m_pDgNodes;
	m_it = rhs.m_it;
	return *this;
}

C_DgNodeConstIterator::operator const I_DgNode*() const
{
	if (m_it != m_pDgNodes->End())
		return m_it->GetValue();
	return 0;
}

const I_DgNode* C_DgNodeConstIterator::operator->()  const
{
	return *this;
}

const I_DgNode* C_DgNodeConstIterator::operator ++()
{
	++m_it;
	return m_it->GetValue();
}

const I_DgNode* C_DgNodeConstIterator::operator ++(int) 
{
	return ++(*this);
}

C_DgNodeConstIterator::C_DgNodeConstIterator()
{
}

C_DgNodeConstIterator::C_DgNodeConstIterator(const DgIdPtrMap* pDgNodes)
{
	m_pDgNodes = pDgNodes;
	assert(m_pDgNodes);
	m_it = m_pDgNodes->Begin();
}



C_DgNodeIterator::C_DgNodeIterator(const C_DgNodeIterator& rhs)
{
	*this = rhs;
}

const C_DgNodeIterator& C_DgNodeIterator::operator=(const C_DgNodeIterator& rhs)
{
	C_DgNodeConstIterator::operator =(rhs);
	return *this;
}

C_DgNodeIterator::operator I_DgNode*()
{
	if (m_it != m_pDgNodes->End())
		return const_cast<I_DgNode*>(m_it->GetValue());
	return 0;
}

I_DgNode* C_DgNodeIterator::operator->() 
{
	return *this;
}

I_DgNode* C_DgNodeIterator::operator ++()
{
	return const_cast<I_DgNode*>(C_DgNodeConstIterator::operator ++());
}

I_DgNode* C_DgNodeIterator::operator ++(int) 
{
	return ++(*this);
} 

C_DgNodeIterator::C_DgNodeIterator(const DgIdPtrMap* pDgNodes)
:
C_DgNodeConstIterator(pDgNodes)
{
}

}
