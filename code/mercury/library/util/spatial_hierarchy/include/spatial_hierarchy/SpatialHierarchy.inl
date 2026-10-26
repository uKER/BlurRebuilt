#ifndef __SPATIAL_HIERARCHY_INL__
#define __SPATIAL_HIERARCHY_INL__

namespace SD
{

template<typename Archive>
void SpatialHierarchy::write(Archive &ar)
{
	ar << m_bIgnoreInstances;

	// save instance kd trees
	ar << m_KdTrees.Size();
	for (int i=0; i<m_KdTrees.Size(); i++)
		m_KdTrees[i].write(ar);

	// save merged kd tree
	ar << m_MainKdTrees.Size();
	for (int i=0; i<m_MainKdTrees.Size(); i++)
		m_MainKdTrees[i].write(ar);

	// read active trees
	ar << m_iActiveMainKdTrees;

	// save objects
	ar << m_Objects.Size();
	for (int i=0; i<m_Objects.Size(); i++)
		m_Objects[i].write(ar);

	// save alpha textures
	int n = m_pAlphaTextureList->Size();
	ar << n;
	if (n)
	{
		for (int i=0; i<n; i++)
			ar << m_pAlphaTextureList->At(i);
	}

	// save layer on/off switches
	ar << m_nLayers;
	if (m_nLayers)
		ar.writeBinary(&m_pbLayer[0], m_nLayers);

	KdTreeTemplate<ObjectPrim>::write(ar);
}

template<typename Archive>
void SpatialHierarchy::read(Archive &ar)
{
	MEMORY_TRACKER_PUSH("mercury.spatialhierarchy.client");

	ar >> m_bIgnoreInstances;

	// read instance kd trees
	int n;
	ar >> n;
	m_KdTrees.Resize(n);
	for (int i=0; i<n; i++)
	{
		m_KdTrees[i].read(ar, "mercury.spatialhierarchy.client.kdtree.baseinstances");
		m_KdTrees[i].m_pSpatialHierarchy = this;
	}

	// read merged kd trees
	ar >> n;
	m_MainKdTrees.Resize(n);
	for (int i=0; i<n; i++)
	{
		char buffer[512];
		sprintf(buffer, "mercury.spatialhierarchy.client.kdtree.layertrees.layer%d", i);
		m_MainKdTrees[i].read(ar, buffer);
		m_MainKdTrees[i].m_pSpatialHierarchy = this;
	}

	// read active trees
	ar >> m_iActiveMainKdTrees;

	// read objects
	MEMORY_TRACKER_PUSH("mercury.spatialhierarchy.client.kdtree.instanceobjects");
	ar >> n;
	m_Objects.Resize(n);
	for (int i=0; i<n; i++)
		m_Objects[i].read(ar);
	if (n)
		m_pTris = &m_Objects[0];
	MEMORY_TRACKER_POP("mercury.spatialhierarchy.client.kdtree.instanceobjects");

	MEMORY_TRACKER_PUSH("mercury.spatialhierarchy.client.kdtree.alphatextures");
	// read alpha textures
	ar >> n;
	if (n)
	{
		m_pAlphaTextureList = new Ut::Vector<AlphaTextureInfo>;
		m_pAlphaTextureList->Resize(n);
		for (int i=0; i<n; i++)
			ar >> m_pAlphaTextureList->At(i);
	}
	else
		m_pAlphaTextureList = NULL;
	MEMORY_TRACKER_POP("mercury.spatialhierarchy.client.kdtree.alphatextures");

	// read layer on/off switches
	ar >> m_nLayers;
	m_pbLayer.Resize(m_nLayers);
	if (m_nLayers)
		ar.readBinary((void*)&m_pbLayer[0], m_nLayers);

	m_bAllocatedBySerialization = true;

	MEMORY_TRACKER_PUSH("mercury.spatialhierarchy.client.kdtree.instancetree");
	KdTreeTemplate<ObjectPrim>::read(ar);
	MEMORY_TRACKER_POP("mercury.spatialhierarchy.client.kdtree.instancetree");

	MEMORY_TRACKER_POP("mercury.spatialhierarchy.client");
}

}

#endif
