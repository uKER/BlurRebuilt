#ifndef __KD_TREE_TEMPLATE_H__
#define __KD_TREE_TEMPLATE_H__

#ifndef DONT_USE_SIMD
#include "spatial_hierarchy/xmmoperators.h"
#endif

/// The main Kd-Tree template class
/// This template contains the common kd-tree tracing functionality.
/// It is instantiated with the primitive type that is contained in the tree (e.g. triangles)
/// The primitive type MUST implement the following interface:
///
///  for colliding with a single ray:
///    bool TriAccel::Intersect(const SD::Ray &ray, SD::RayTraceResult &result, void *pTree)
///  where
///     ray		  = the ray to be traced against the primitive
///     result    = the result of the intersection test
///     pTree     = pointer to the instantiation of the KdTreeTemplate template
///     (returns) = true if an intersection occurred
///
///  for colliding with 4 rays ray:
///    bool TriAccel::IntersectSIMD(SIMDRay &ray, SIMDResult &result, __m128 &inmask, void *pTree)
///  where
///     ray		  = the 4 rays to be traced against the primitive
///     result    = the 4 results of the intersection tests
///     mask      = a 128-bit mask, 32-bits for each ray, which can be 0 if that particular ray is to be ignored
///     pTree     = pointer to the instantiation of the KdTreeTemplate template
///     (returns) = true if an intersection occurred
///
template <typename T>
class KdTreeTemplate
{
public:
	/// Constructor
	KdTreeTemplate();
	// Destructor
	~KdTreeTemplate();

	/// Trace a single ray against the Kd-Tree.
	/// If a primitive is hit, it's result is returned in 'result' (callback function is called Intersect and IntersectSIMD)
	bool	Trace(const SD::Ray &ray, SD::RayTraceResult &result) const;

	/// Serialize a spatial hierarchy to an MFC ost::Engine class for loading/saving.
	template<typename Archive>
	void	write(Archive &ar);
	template<typename Archive>
	void	read(Archive &ar);
#ifndef DONT_USE_SIMD
	/// Trace 4 rays against the Kd-Tree
	void	TraceSIMD(SIMDRay &ray, SIMDResult &result) const;
#endif
	/// Sets the epsilon value
	void	SetEpsilon(float fEpsilon) const
	{
		m_xfEpsilon = fEpsilon;
	}

	/// Copy the contents (nodes and mailboxes) of a tree builder class and a primitive list
	void	CopyTreeBuilderContents(KdBuilder *pBuilder, T *pPrims);

	// Trace a bounding box against the Kd-Tree (callback function is called Intersect)
	void	Trace(const Box3 &bbox) const;

	// draw the entire tree as a bunch of bounding boxes
	void	Draw() const;

private:
	// private single-ray functions
	void	IntersectLeaf(int iPrimitive, const SD::Ray &ray, SD::RayTraceResult &result) const;
	bool	IntersectLeafS(int iPrimitive, const SD::Ray &ray, SD::RayTraceResult &result) const;
#ifndef DONT_USE_SIMD
	// private SIMD functions
	void	ClipRayVsBBoxSIMD(const __m128 *vRayOrg, const __m128 *vInvRayDir, const int *iRaydir, __m128 *pfNear, __m128 *pfFar) const;
	void	IntersectLeafSIMD(int iPrimitive, SIMDRay &ray, SIMDResult &result, __m128 mask) const;
#endif
	// private bbox trace function
	void	TraceLeaf(int iPrimitive) const;
	// obsolete
	bool	DirectionSignsMatch(SD::RayBatch &batch) const;
	// clear the contents of the mailbox hash table
	void	ClearMailboxHashTable() const;

public:
	//
	T		*m_pTris;					/// list of primitives

	Box3	m_bbox;						/// bounding box of this KD-tree

	KdNode				*m_pNodes;		/// Kd-tree nodes
	int					m_nNodes;
	MailboxVector		*m_pMailboxes;	/// Kd-tree leafnode primitive index lists

	bool				m_bAllocatedBySerialization;	/// indicates whether the tree was created by deserialization

	mutable float		m_xfEpsilon;

	mutable void		*m_pParent;

	mutable int			*m_pMailboxHashTable;
};

// serialize/deserialize a kd tree
template <typename T>
template <typename Archive>
void KdTreeTemplate<T>::write(Archive &ar)
{
	// save bounding box
	ar << m_bbox.vmin << m_bbox.vmax;

	// save kd nodes
	ar << m_nNodes;
	for (int i=0; i<m_nNodes; i++)
		ar << m_pNodes[i].iFlagDimAndOffset << m_pNodes[i].fSplitCoordinate;

	// save mailboxes
	int n = m_pMailboxes->Size();
	ar << n;
	if (n)
	{
		for (int i=0; i<n; i++)
			ar << m_pMailboxes->At(i).iPrimitive;
	}
}
template <typename T>
template <typename Archive>
void KdTreeTemplate<T>::read(Archive &ar)
{
	// read bounding box
	ar >> m_bbox.vmin >> m_bbox.vmax;

	// read kd nodes
	ar >> m_nNodes;
	m_pNodes = (KdNode*)BizAlignedMalloc(sizeof(KdNode) * m_nNodes, 128);
	for (int i=0; i<m_nNodes; i++)
		ar >> m_pNodes[i].iFlagDimAndOffset >> m_pNodes[i].fSplitCoordinate;

	// read mailboxes
	int nMailboxes;
	ar >> nMailboxes;
	m_pMailboxes = new MailboxVector;
	m_pMailboxes->Resize(nMailboxes);
	if (nMailboxes)
	{
		for (int i=0; i<nMailboxes; i++)
			ar >> m_pMailboxes->At(i).iPrimitive;
	}

	m_bAllocatedBySerialization = true;
}

#include "KdTreeTemplate.inl"

#endif
