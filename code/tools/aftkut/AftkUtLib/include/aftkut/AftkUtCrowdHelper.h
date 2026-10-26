#ifndef AFTK_UT_CROWD_HELPER_H
#define AFTK_UT_CROWD_HELPER_H

#include <btl/containers/vector.h>

namespace Aftk
{
	class C_SceneNode;
};

namespace AftkUt
{
	namespace CrowdHelper
	{
		bool	IsBaseCrowdInstance(const Aftk::C_SceneNode* pSceneNode);

		bool	IsCrowdSkeleton(const Aftk::C_SceneNode* pSceneNode);

		Aftk::C_SceneNode*	GetAftkRoot(Aftk::C_SceneNode* pSceneNode);

		bool	CompareNameWithSceneNodeAndParents(const char* pName, const Aftk::C_SceneNode* pSceneNode);

		void	StripCrowdFlags(Aftk::C_SceneNode* pSceneNode);

		void	BuildSkeletonJointMap(Btl::Vector<const Aftk::C_SceneNode*>& joints,
					const Btl::Vector<const Aftk::C_SceneNode*>& skeletonNodes);

		void	SetCrowdInstanceIndex(Aftk::C_SceneNode* pSceneNode, const S32 index);

		S32		GetCrowdInstanceIndex(const Aftk::C_SceneNode* pSceneNode);
	};
}; // namespace AftkUt

#endif
