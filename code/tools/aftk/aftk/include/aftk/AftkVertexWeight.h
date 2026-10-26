#ifndef AFTK_VERTEX_WEIGHT_H
#define AFTK_VERTEX_WEIGHT_H

#ifdef _MSC_VER_
#pragma once
#endif


namespace Aftk
{
	class C_VertexWeight
	{
	public:
		///\brief Constructor.
		///
		C_VertexWeight();

		///\brief Clears the contents of this class to their initial state.
		///
		void Clear();

		///\brief Adds a weight to this vertex weight container.
		///
		///Only the four most influential weights are held.
		///@param weight The new weight.
		///@param influenceNodeIndex The influence node for this weight.
		void AddWeight(F32 weight, U32 influenceNodeIndex);

		///\brief Normalises the sum of the weights contained.
		///
		void Normalise();

		///\brief Returns the number of weights held.
		///
		///@return The number of weights.
		U32 GetNumWeights() const;

		///\brief Returns the influence node index for a given weight index.
		///
		///@param index C_Weight index.
		///@return The influence node index.
		U32 GetInfluenceNodeIndex(U32 index) const;

		///\brief Returns a vertex weight value.
		///
		///@param index The weight index.
		///@return The weight.
		F32 GetWeight(U32 index) const;

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	private:
		static const U32 MAX_INFLUENCE_NODES = 4;
		F32 m_weights[MAX_INFLUENCE_NODES];
		//Bone (aka joint) indices
		U8 m_indices[MAX_INFLUENCE_NODES];
		U8 m_numWeights;
	#endif
	};

	typedef Ut::UtVector< Ut::DynamicVector<C_VertexWeight> >				VertexWeightArray;
	typedef Ut::UtVector< Ut::DynamicVector<C_VertexWeight> >				VertexWeightPtrArray;
}

#endif
