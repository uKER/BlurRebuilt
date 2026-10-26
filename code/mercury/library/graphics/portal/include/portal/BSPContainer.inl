// Default c'tor.
__forceinline C_BSPContainer::C_BSPContainer() : 
	m_iNodeCount(0),
	m_pElements(NULL)
{
	return;
}

// -----------------------------------------------------------------

// D'tor.
__forceinline C_BSPContainer::~C_BSPContainer()
{
	if(m_pElements)
		BizAlignedFree(m_pElements);
	return;
}

// -----------------------------------------------------------------

#ifndef PLATFORM_SPU
// Deserialise a BSP container from the stream.
template <typename _T>
void C_BSPContainer::Deserialise(_T& rkStream)
{
	U32 iVersion = 0;
	rkStream >> iVersion;
	BIZ_WARNING_MSG(iVersion == 0, "Code written to deserialise RegionBSP version 0 only!");
	rkStream >> m_iNodeCount;

	rkStream >> m_RootRegion;	// Root region.

	// Load the tree nodes...
	C_BSPSplitPlane* pNodeData = (C_BSPSplitPlane*)BizAlignedMalloc(m_iNodeCount*sizeof(C_BSPSplitPlane), 16);
	BIZ_WARNING_MSG(pNodeData, "No memory allocated!");										// Sanity check -- no memory allocated.
	BIZ_WARNING_MSG((U32)pNodeData == ((U32)pNodeData&~0xf), "Memory not 16byte aligned.");	// Sanity check -- memory not 16byte aligned.
	for(U32 iCurrentNode=0; iCurrentNode<m_iNodeCount; ++iCurrentNode)
		rkStream >> pNodeData[iCurrentNode];

	m_pElements = pNodeData;
	return;
}
#endif // #ifndef PLATFORM_SPU

// -----------------------------------------------------------------

// Return number of nodes in the tree.
__forceinline NOALIAS const U32 C_BSPContainer::GetNodeCount() const __restrict
{
	return m_iNodeCount;
}

// -----------------------------------------------------------------

// Return pointer to the nodes vector.
__forceinline NOALIAS const C_BSPSplitPlane* C_BSPContainer::GetNodes() const __restrict
{
	return m_pElements;
}

// -----------------------------------------------------------------

#ifdef PLATFORM_SPU
// Set the elements pointer for the tree.
__forceinline NOALIAS void C_BSPContainer::SetNodePtr(C_BSPSplitPlane* pNodes) __restrict
{
	m_pElements = pNodes;
	return;
}
#endif // #ifdef PLATFORM_SPU
