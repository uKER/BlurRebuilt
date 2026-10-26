#include <stdafx.h>

using namespace Aftk;

C_VertexWeight::C_VertexWeight()
{
	Clear();
}

void C_VertexWeight::Clear()
{
	m_numWeights = 0;
	for (U32 i = 0; i < MAX_INFLUENCE_NODES; ++i)
	{
		m_indices[i] = 0;
		m_weights[i] = 0.f;
	}
}

void C_VertexWeight::AddWeight(F32 weight, U32 influenceNodeIndex)
{
	// Insert into weight list
	bool inserted = false;
	for (S32 i = 0; i < m_numWeights; ++i)
	{
		if (m_indices[i] == influenceNodeIndex)
		{
			//We already having weighting for this.
			return;
		}
	}
	for (S32 i = 0; i < m_numWeights; ++i)
	{
		if (weight > m_weights[i])
		{
			// Shuffle up to make room for the new weight.
			int endIndex = m_numWeights;
			if (m_numWeights >= MAX_INFLUENCE_NODES)
			{
				// Overwrite the last (and smallest) weight in the list.
				endIndex = m_numWeights - 1;
			}

			for (S32 j = endIndex; j > i; --j)
			{
				AFTK_ASSERT((j < MAX_INFLUENCE_NODES+1) && (j >= 0));
				m_weights[j] = m_weights[j-1];
				m_indices[j] = m_indices[j-1];
			}

			AFTK_ASSERT((i < MAX_INFLUENCE_NODES) && (i >= 0));
			// Insert new weight.
			m_weights[i] = weight;
			m_indices[i] = influenceNodeIndex;
			inserted = true;
			if (m_numWeights < MAX_INFLUENCE_NODES)
			{
				m_numWeights++;
			}
			return;
		}
	}

	if (m_numWeights < MAX_INFLUENCE_NODES)
	{
		m_weights[m_numWeights] = weight;
		m_indices[m_numWeights] = influenceNodeIndex;
		m_numWeights++;
	}
}

void C_VertexWeight::Normalise()
{	
	F32 weightSum = 0.f;
	for (U32 i = 0; i < m_numWeights; ++i)
	{
		weightSum += m_weights[i];
	}

	for (U32 i = 0; i < m_numWeights; ++i)
	{
		m_weights[i] /= weightSum;
	}
}


U32 C_VertexWeight::GetNumWeights() const
{
	return m_numWeights;
}

U32 C_VertexWeight::GetInfluenceNodeIndex(U32 index) const
{
	AFTK_ASSERT(index < m_numWeights);
	return m_indices[index];
}


F32 C_VertexWeight::GetWeight(U32 index) const
{
	AFTK_ASSERT(index >= 0 && index < m_numWeights);
	return m_weights[index];
}
