#ifndef AFTK_ANIMATION_CURVE_NODE_H
#define AFTK_ANIMATION_CURVE_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Curve
{
	enum E_TangentType;
	enum E_InfinityType;
}

namespace Aftk
{
	class C_CurveData;
	class C_CurveEval;
	
	///Description: An animation curve is where information such as keyframe data is 
	///stored. This data comprises of a number of scalars representing the value of 
	///variable at a given x. 
	class C_AnimationCurveNode : public I_EvaluatableNode
	{
	public:
		///\brief Constructor.
		///
		C_AnimationCurveNode(C_Scene* pScene);

		virtual C_Node* Clone(C_Scene* pScene) const;

		///\brief Copy constructor.
		///
		///@param rhs The curve to copy.
		C_AnimationCurveNode(const C_AnimationCurveNode& rhs);

		///\brief Destructor.
		///
		virtual ~C_AnimationCurveNode();

		///\brief Asignment operator.
		///
		///@param rhs The curve to copy.
		///@return A reference to this node.
		const C_AnimationCurveNode& operator=(const C_AnimationCurveNode& rhs);

		///\brief Adds a key to this curve.
		///
		///All keys within a curve must be of the same type.
		///A return value of -1 indicates a failure.
		///@param key The key to add.
		///@return The index of the new key.
		S32 AddKey(const C_LinearKey& key);

		///\brief Adds a key to this curve.
		///
		///All keys within a curve must be of the same type.
		///A return value of -1 indicates a failure.
		///@param key The key to add.
		///@return The index of the new key.
		S32 AddKey(const C_HermiteKey& key);

		bool SetKey(U32 keyIndex, const C_LinearKey& key);

		bool SetKey(U32 keyIndex, const C_HermiteKey& key);

		///\brief Returns a key from the curve.
		///
		///@param keyIndex The index of the key to fetch.
		///@param key The key.
		bool GetKey(C_LinearKey& key, U32 keyIndex) const;

		///\brief Returns a key from the curve.
		///
		///@param keyIndex The index of the key to fetch.
		///@param key The key.
		bool GetKey(C_HermiteKey& key, U32 keyIndex) const;

		///\brief Returns the number of keys in this curve.
		///
		///@return The number of keys in this curve.
		U32 GetNumKeys() const;

		///\brief Find the lowest value of x for which there is a key on this curve.
		///
		///@return The lowest value of x.
		F32 GetStartX() const;

		///\brief Find the highest value of x for which there is a key on this curve.
		///
		///@return The highest value of x.
		F32 GetEndX() const;

		///\brief Find the keys adjacent to a given value of x.
		///
		///If an exact match is made, i.e. if there is a key for x, then keyIndex1 is the index of
		///the exactly matching key, and keyIndex2 is -1. If x is beyond the end of the curve, then
		///keyIndex1 is the final key, and keyIndex2 is -1. If x is beyond the start of the curve, then
		///keyIndex1 is the first key, and keyIndex2 is -1. For all other cases keyIndex1 and keyIndex2 are
		///the indices of they keys to the left and right of x respectively.
		///@param x The query value.
		///@param keyIndex1 the left key index.
		///@param keyIndex2 the right key index.
		bool GetAdjacentKeyIndices(S32& keyIndex1, S32& keyIndex2, F32 x) const;

		///\brief Evaluates the curve at the current scene time.
		///
		///@return result
		virtual bool Evaluate(F32& value) const;

		///\brief Evaluates the curve at the current scene time.
		///
		///@param value Attribute value.
		virtual bool Evaluate(F64& value) const;		
		
		///\brief Evaluates the curve at the current scene time.
		///
		///@param value Attribute value.
		virtual bool Evaluate(S32& value) const;
		
		///\brief Evaluates the curve at the current scene time.
		///
		///@param value Attribute value.		
		virtual bool Evaluate(bool& value) const;

		///\brief Returns adjacent keys at the current scene time.
		///
		///@return result
		virtual bool GetAdjacentKeys(C_BaseKey& key1, C_BaseKey& key2, F32 x) const;

		///\brief Returns adjacent keys at the current scene time.
		///
		///@return result
		virtual bool GetAdjacentKeys(C_LinearKey& key1, C_LinearKey& key2, F32 x) const;

		///\brief Returns adjacent keys at the current scene time.
		///
		///@return result
		virtual bool GetAdjacentKeys(C_HermiteKey& key1, C_HermiteKey& key2, F32 x) const;

		///\brief Returns adjacent keys at the current scene time.
		///
		///@return result
		virtual bool GetAdjacentKeys(C_BaseKey& key1, C_BaseKey& key2) const;

		///\brief Returns adjacent keys at the current scene time.
		///
		///@return result
		virtual bool GetAdjacentKeys(C_LinearKey& key1, C_LinearKey& key2) const;

		///\brief Returns adjacent keys at the current scene time.
		///
		///@return result
		virtual bool GetAdjacentKeys(C_HermiteKey& key1, C_HermiteKey& key2) const;

		///\brief Evaluates the curve for a given value of x.
		///
		///@param x The variable to evaluate the curve for.
		///@return result
		bool Evaluate(F32& value, F32 x) const;

		C_NullPlug& GetInputPlug();

		const C_NullPlug& GetInputPlug() const;

		void SetIsTimeInput(bool isTimeInput);

		bool IsTimeInput() const;

		E_AnimCurveType GetCurveType() const;

		virtual E_ObjType GetType() const;

		virtual bool HasType(E_ObjType type) const;

		///\brief Generate hash of data in current class and hierachy
		///
		///@return 32b hash value
		virtual U32 GetHash(E_ObjType classToAbortTest = E_ObjType_Unknown) const;

		virtual bool IsUsed() const;

		///\brief Clears the contents of this curve to their initial state.
		///
		virtual void Clear();

		///\brief Returns the static plug at the given index.
		///
		///@param instance The plug index.
		///@return The plug.
		I_Plug* GetStaticPlugPtr(U32 instance)
		{
			return const_cast<I_Plug*>(((const C_Node*)this)->GetStaticPlugPtr(instance));
		}

		///\brief Returns the static plug at the given index.
		///
		///@param instance The plug index.
		///@return The plug.
		virtual const I_Plug* GetStaticPlugPtr(U32 instance) const;

		///\brief Returns the number of static plugs on this node.
		///
		///@return The number of dynamic plugs.
		virtual U32	GetNumStaticPlugs() const;

		void SetPreInfinity(E_CurveInfinityType type);

		void SetPostInfinity(E_CurveInfinityType type);

		E_CurveInfinityType GetPreInfinity() const;

		E_CurveInfinityType GetPostInfinity() const;

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		void ClearThis();

		void OneTimeInit();

		void CreateCurve(E_AnimCurveType type);

		void DestroyCurve();

		template<class STORAGE>
		void BinarySearchForKeyPair(
			S32& index1,
			S32& index2,
			const STORAGE& storage, 
			F32 x) const;

		bool EvaluateLinear(F32& value, F32 x) const;

		bool EvaluateHermite(F32& value, F32 x) const;

		static void OnInputPlugChanged(void* pThis);

		E_CurveTangentType		CurveTangentToAftkTangent(Curve::E_TangentType type) const;
		Curve::E_TangentType	AftkTangentToCurveTangent(E_CurveTangentType type) const;

		E_CurveInfinityType		CurveInfinityToAftkInfinity(Curve::E_InfinityType type) const;
		Curve::E_InfinityType	AftkInfinityToCurveInfinity(E_CurveInfinityType type) const;


		C_NullPlug				m_inputPlug;

		bool					m_isTimeInput;
		E_AnimCurveType			m_curveType;
		E_CurveInfinityType		m_preInfinity;
		E_CurveInfinityType		m_postInfinity;

		void*					m_pCurveData;
		void*					m_pCurveEval;

		friend class C_NativeAnimationCurveNode;
	#endif
	};

	#include "AftkAnimationCurveNode.inl"
}

#endif
