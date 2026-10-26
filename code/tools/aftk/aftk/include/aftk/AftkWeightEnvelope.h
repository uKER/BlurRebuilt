#ifndef AFTK_WEIGHT_ENVELOPE_H
#define AFTK_WEIGHT_ENVELOPE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	///A weighting envelope class. A weighting envelope contains a set
	///of weights that are associated with a particular 'influence node'. An
	///influence object is an object, such as a bone, to which vertices are 
	///weighted.
	class C_WeightEnvelope
	{
	public:
		///\brief Constructor.
		///
		C_WeightEnvelope();

		///\brief Copy constructor.
		///
		///@param rhs The envelope to copy.
		C_WeightEnvelope(const C_WeightEnvelope& rhs);

		///\brief Destructor.
		///
		virtual ~C_WeightEnvelope();

		///\brief Asignment operator.
		///
		///@param rhs The envelope to copy.
		///@return A reference to this envelope.
		const C_WeightEnvelope& operator=(const C_WeightEnvelope& rhs);

		///\brief Adds a weight to the array of weights contained by this class.
		///
		///@param weight The weight to add.
		///@return The index of this weight in the array of weights.
		U32	AddWeight(const C_Weight<F32>& weight);

		///\brief Sets the initial capacity of the weight array.
		///
		///Presizing this array will increase performance.
		///
		///@param capacity The capacity of the weight array.
		void SetWeightInitialCapacity(U32 capacity);

		///\brief Overwrites a particular weight.
		///
		///@param instance The index of the weight to overwrite.
		///@param weight The weight to copy.
		void SetWeight(U32 instance, const C_Weight<F32>& weight);

		///\brief Sets a pointer to the node to which influences are weighted vertices
		///
		///This object would typically be a null or bone node, but can be any scene node.
		///
		///@param pInfluenceNode The influence node pointer.
		void SetInfluenceNodePtr(C_SceneNode* pInfluenceNode);

		///\brief Returns a pointer to the node which influences the weighted vertices.
		///
		///@return The influence node pointer.
		C_SceneNode* GetInfluenceNodePtr();

		///\brief Returns a pointer to the node which influences the weighted vertices.
		///
		///@return The influence node pointer.
		const C_SceneNode* GetInfluenceNodePtr() const;

		///\brief Returns the number of weights contained.
		///
		///@return The number of weights.
		U32	GetNumWeights() const;

		///\brief Returns the data type of the weights contained within this envelope.
		///
		///@return data type enum.
		E_GenericAttributeType GetDataType() const;

		///\brief Returns the weight at the given index.
		///
		///@param instance The index of the weight to return.
		///@param weight The weight.
		void GetWeight(U32 instance, C_Weight<F32>& weight);

		///\brief Returns the weight at the given index.
		///
		///@param instance The index of the  weight to return.
		///@param weight The weight.
		void GetWeight(U32 instance, C_Weight<F32>& weight) const;

		///\brief Clears this envelope to their initial state.
		///
		void Clear();

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	private:
		C_SceneNode* m_pInfluenceNode;

		//The type of weight we can handle is for the moment limited to type F32.
		//If required in the future this can be extended to being generic, in the same manner as the
		//animation curves.
		Ut::UtVector< Ut::DynamicVector<C_Weight<F32> > >	m_weights;

		E_GenericAttributeType m_dataType;

		friend class C_SceneMeshNode;
		friend class C_NativeFtk;
		friend class C_NativeWeightEnvelope;
	#endif
	};
}
#endif
