#ifdef _MSC_VER_
#pragma once
#endif

#ifndef AFTK_CLIP_CURVE_MAPPING_H
#define AFTK_CLIP_CURVE_MAPPING_H

namespace Aftk
{
	class C_SceneNode;

	class C_ClipCurveMapping
	{
	public:
		///\brief Constructor.
		///
		C_ClipCurveMapping()
		{
			m_pAnimatedNode = 0;
		}

		///\brief Copy constructor.
		///
		///@param rhs The clip curve mapping to copy.
		C_ClipCurveMapping(const C_ClipCurveMapping& rhs)
		{
			*this = rhs;
		}

		///\brief Asignment operator.
		///
		///@param rhs The clip curve mapping to copy.
		///@return A reference to this node.
		const C_ClipCurveMapping& operator=(const C_ClipCurveMapping& rhs)
		{
			m_pAnimatedNode = rhs.m_pAnimatedNode;
			m_curveIndices = rhs.m_curveIndices;

			return *this;
		}

		///\brief Returns the scene node that this mapping is associated with.
		///
		///@return The scene node.
		C_SceneNode* GetSceneNodePtr()
		{
			return m_pAnimatedNode;
		}

		///\brief Returns the scene node that this mapping is associated with.
		///
		///@return The scene node.
		const C_SceneNode* GetSceneNodePtr() const
		{
			return m_pAnimatedNode;
		}

		///\brief Returns the number of curve indices.
		///
		///@return The number of curve indices.
		U32 GetNumCurves() const
		{
			return (U32)m_curveIndices.Size();
		}

		///\brief Returns the nth curve index.
		///
		///@param curveInstance The index of the curve index to return.
		///@return The curve index.
		E_AnimAttribute GetCurveAttribute(U32 curveInstance) const
		{
			return m_curveIndices[curveInstance];
		}

		///\brief Set the scene node that this mapping is associated with.
		///
		///@param pSceneNode The scene node.
		void SetSceneNodePtr(C_SceneNode* pSceneNode)
		{
			m_pAnimatedNode = pSceneNode;
		}

		///\brief Add a curve index to this mapping.
		///
		///@param curveIndex The new curve index.
		///@return The index of this curve in our curve index array.
		U32 AddCurveIndex(E_AnimAttribute curveIndex)
		{
			m_curveIndices.PushBack(curveIndex);
			return (U32)m_curveIndices.Size()-1;
		}

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	private:
		C_SceneNode*	m_pAnimatedNode;

		Ut::UtVector< Ut::DynamicVector<E_AnimAttribute> >	m_curveIndices;

		friend class C_NativeClipCurveMapping;
	#endif
	};
}

#endif
