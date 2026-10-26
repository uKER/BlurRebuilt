#ifndef __SERIALIZEKDTREE_H__
#define __SERIALIZEKDTREE_H__

// serialize/deserialize the triangle structure
template<typename Archive>
Archive& operator << (Archive& ar, TriAccel &t)
{
	ar << t.n_u << t.n_v << t.n_d << t.m_k;// << t.m_vNormal << t.m_vCentroid;
	ar << t.b_nu << t.b_nv << t.b_d;
	ar << t.c_nu << t.c_nv << t.c_d;

	return ar;
}

template<typename Archive>
Archive& operator >> (Archive& ar, TriAccel &t)
{
	ar >> t.n_u >> t.n_v >> t.n_d >> t.m_k;// >> t.m_vNormal >> t.m_vCentroid;
	ar >> t.b_nu >> t.b_nv >> t.b_d;
	ar >> t.c_nu >> t.c_nv >> t.c_d;

	return ar;
}

// serialize/deserialize the extra triangle info structure
template<typename Archive>
Archive& operator >> (Archive& ar, OptionalTriInfo &t)
{
	ar >> t.iAlphaTex >> t.m_uv[0] >> t.m_uv[1] >> t.m_uv[2];
	return ar;
}

template<typename Archive>
Archive& operator << (Archive& ar, OptionalTriInfo &t)
{
	ar << t.iAlphaTex << t.m_uv[0] << t.m_uv[1] << t.m_uv[2];
	return ar;
}


// serialize/deserialize a kd tree
template<typename Archive>
void KdTree::write(Archive &ar)
{
	// save triangles
	ar << m_nTris;
	for (int i=0; i<m_nTris; i++)
		ar << m_pTris[i];

	// save optional tri info
	ar << m_nOptionalTriInfo;
	if (m_nOptionalTriInfo)
	{
		for (int i=0; i<m_nOptionalTriInfo; i++)
			ar << m_pOptionalInfo[i];
	}

	ar << m_iTreeLayer << m_iFirstGlobalPrimIdx << m_iLodLevel;

	KdTreeTemplate<TriAccel>::write(ar);
}

template<typename Archive>
void KdTree::read(Archive &ar, char *psz)
{
	// read triangles
	MEMORY_TRACKER_PUSH_DYNAMIC("%s.triangles", psz);
	ar >> m_nTris;
	m_pTris = (TriAccel*)BizAlignedMalloc(sizeof(TriAccel) * m_nTris, 128);
	for (int i=0; i<m_nTris; i++)
		ar >> m_pTris[i];
	MEMORY_TRACKER_POP_DYNAMIC("%s.triangles", psz);

	// read optional tri info
	MEMORY_TRACKER_PUSH_DYNAMIC("%s.optionalinfo", psz);
	ar >> m_nOptionalTriInfo;
	if (m_nOptionalTriInfo)
	{
		m_pOptionalInfo = new OptionalTriInfo[m_nOptionalTriInfo];
		for (int i=0; i<m_nOptionalTriInfo; i++)
			ar >> m_pOptionalInfo[i];
	}
	else
		m_pOptionalInfo = NULL;
	MEMORY_TRACKER_POP_DYNAMIC("%s.optionalinfo", psz);

	MEMORY_TRACKER_PUSH_DYNAMIC("%s.nodes", psz);
	ar >> m_iTreeLayer >> m_iFirstGlobalPrimIdx >> m_iLodLevel;

	m_bAllocatedBySerialization = true;

	KdTreeTemplate<TriAccel>::read(ar);
	MEMORY_TRACKER_POP_DYNAMIC("%s.nodes", psz);
}

#endif
