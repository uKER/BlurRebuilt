#include <Render Lib/stdafx.h>
#include <Render Lib/GVertexWeights.h>

using namespace SRD;

GVertexWeights::GVertexWeights()
{
	Clear();
}

void GVertexWeights::Clear()
{
	m_numWeights = 0;
	for (U32 i = 0; i < MAX_BONES; ++i)
	{
		m_indices[i] = 0;
		m_weights[i] = 0.f;
	}
}

void GVertexWeights::AddWeight(F32 weight, U32 index)
{
	// Insert into weight list
	bool inserted = false;
	for (S32 i = 0; i < m_numWeights; ++i)
	{
		if (weight > m_weights[i])
		{
			// Shuffle up to make room for the new weight.
			int endIndex = m_numWeights;
			if (m_numWeights >= MAX_BONES)
			{
				// Overwrite the last (and smallest) weight in the list.
				endIndex = m_numWeights - 1;
			}

			for (S32 j = endIndex; j > i; --j)
			{
				m_weights[j] = m_weights[j-1];
				m_indices[j] = m_indices[j-1];
			}

			// Insert new weight.
			m_weights[i] = weight;
			m_indices[i] = index;
			inserted = true;
			return;
		}
	}

	if (m_numWeights < MAX_BONES)
	{
		m_weights[m_numWeights] = weight;
		m_indices[m_numWeights] = index;
		++m_numWeights;
	}
}

void GVertexWeights::Normalise()
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


U32 GVertexWeights::NumWeights() const
{
	return m_numWeights;
}

U32 GVertexWeights::BoneIndex(U32 index) const
{
	assert(index < m_numWeights);
	return m_indices[index];
}


F32 GVertexWeights::Weight(U32 index) const
{
	assert(index >= 0 && index < m_numWeights);
	return m_weights[index];
}
