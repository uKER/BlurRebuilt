#pragma once

namespace SRD
{
	class GVertexWeights
	{
	public:
		GVertexWeights();

		void Clear();
		void AddWeight(F32 weight, U32 boneIndex);
		void Normalise();

		U32 NumWeights() const;
		U32 BoneIndex(U32 index) const;

		F32 Weight(U32 index) const;

	private:
		static const U32 MAX_BONES = 4;
		F32 m_weights[MAX_BONES];
		//Bone (aka joint) indices
		U8 m_indices[MAX_BONES];
		U8 m_numWeights;
	};
}
