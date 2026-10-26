#ifndef AFTK_GROUP_LOD_LEVEL_INFO_H
#define AFTK_GROUP_LOD_LEVEL_INFO_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_GroupLodLevelInfo
	{
	public:
		///\brief Constructor
		///
		C_GroupLodLevelInfo()
		{
			m_threshold = 0;
			m_pSceneNode = 0;
		}

		///\brief Copy constructor
		///
		///@param rhs The class to copy.
		C_GroupLodLevelInfo(const C_GroupLodLevelInfo& rhs)
		{
			*this = rhs;
		}
		
		///\brief Destructor.
		///
		~C_GroupLodLevelInfo()
		{
		}

		///\brief Asignment operator
		///
		///@param rhs The class to copy.
		///@return A reference to this class.
		const C_GroupLodLevelInfo& operator=(const C_GroupLodLevelInfo& rhs)
		{
			m_threshold = rhs.m_threshold;
			m_pSceneNode = rhs.m_pSceneNode;
			return *this;
		}

		///\brief Set the scene node that this lod level info is associated with.
		///
		///@param pSceneNode The scene node.
		void SetSceneNodePtr(C_SceneNode* pSceneNode)
		{
			m_pSceneNode = pSceneNode;
		}

		///\brief Set the threshold at which we switch to the next level of detail.
		///
		///@param threshold The LOD threshold.
		void SetThreshold(F32 threshold)
		{
			m_threshold = threshold;
		}

		///\brief Returns the scene node associated with this level of detail.
		///
		///@return The scene node.
		const C_SceneNode* GetSceneNodePtr() const
		{
			return m_pSceneNode;
		}

		///\brief Returns the scene node associated with this level of detail.
		///
		///@return The scene node.
		C_SceneNode* GetSceneNodePtr()
		{
			return m_pSceneNode;
		}

		///\brief Returns the threshold at which we switch to the next level of detail.
		///
		///@return The LOD threshold.
		F32 GetThreshold() const
		{
			return m_threshold;
		}

	private:
		F32				m_threshold;
		C_SceneNode*		m_pSceneNode;
	};
}

#endif
