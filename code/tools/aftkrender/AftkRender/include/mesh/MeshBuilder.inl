

inline bool C_MeshBuilder::SkinPartition::AddPolygon(
	U32 polyIndex, 
	Ut::UtVector< Ut::DynamicVector<const Aftk::C_SceneNode*> >&influencers)
{
	Ut::UtVector< Ut::DynamicVector<const Aftk::C_SceneNode*> > differenceSet;

	for (S32 i=0; i<influencers.Size(); ++i)
	{
		bool found = false;
		for (S32 j=0; j<influenceNodes.Size(); ++j)
			if (influencers[i] == influenceNodes[j])
				found = true;

		if (!found)
			differenceSet.PushBack(influencers[i]);
	}

	if (differenceSet.Size() + influenceNodes.Size() > C_MeshBuilder::MAX_MATRIX_PALETTE_SIZE)
		return false;

	for (S32 i=0; i<differenceSet.Size(); ++i)
		influenceNodes.PushBack(differenceSet[i]);

	polygons.PushBack(polyIndex);
	return true;
}


inline C_MeshBuilder::VertexWeight::VertexWeight()
{
	memset(weights, 0, sizeof(weights));
	memset(nodes, 0, sizeof(nodes));
}

inline void C_MeshBuilder::VertexWeight::Add(
	F32 weight,
	const Aftk::C_SceneNode* pNode)
{
	for (U32 i=0; i<MAX_VERTEX_INFLUENCERS; ++i)
	{
		if (weight>weights[i])
		{
			for (U32 j=MAX_VERTEX_INFLUENCERS-1; j>i; --j)
			{
				weights[j] = weights[j-1];
				nodes[j]  = nodes[j-1];
			}
			weights[i] = weight;
			nodes[i]  = pNode;
			break;
		}
	}
}