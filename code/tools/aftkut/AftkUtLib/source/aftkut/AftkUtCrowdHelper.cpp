#include "stdafx.h"

#include <AftkUt/AftkUtCrowdHelper.h>
#include <AftkUt/AftkUt.h>

namespace AftkUt
{
	namespace CrowdHelper
	{
		bool IsBaseCrowdInstance(const Aftk::C_SceneNode* pSceneNode)
		{
			AftkUt::Query::C_Flags flags = AftkUt::Query::GetConventionFlags(pSceneNode);
			if (flags.asCrowd)
				return true;

			return false;
		}

		bool IsCrowdSkeleton(const Aftk::C_SceneNode* pSceneNode)
		{
			bool status = false;

			C_NodeFlagParser nodeFlags(*pSceneNode, &status);

			if (!status)
				return false;

			if (_stricmp(nodeFlags.GetFlagGroupCategory().c_str(), "Crowd") == 0)
			{
				if (_stricmp(nodeFlags.GetFlagGroupName().c_str(), "Skeleton") == 0)
					return true;
			}

			return false;
		}

		Aftk::C_SceneNode* GetAftkRoot(Aftk::C_SceneNode* pSceneNode)
		{
			if (!pSceneNode || pSceneNode->GetName() == "AftkUtImportRoot")
				return NULL; // Gone too far :(

			if (pSceneNode->GetName() != "AftkRoot")
				return GetAftkRoot(pSceneNode->GetParentPtr());
			else
				return pSceneNode;
		}

		bool CompareNameWithSceneNodeAndParents(const char* pName, const Aftk::C_SceneNode* pSceneNode)
		{
			if (!pName || !pSceneNode)
				return false;
			else
			{
				if(_stricmp(pName, pSceneNode->GetName().c_str()) == 0)
					return true;
				else
					return CompareNameWithSceneNodeAndParents(pName, pSceneNode->GetParentPtr());
			}
		}

		void StripCrowdFlags(Aftk::C_SceneNode* pSceneNode)
		{
			if (!pSceneNode)
				return;

			bool status = false;

			C_NodeFlagParser nodeFlags(*pSceneNode, &status);

			if (!status)
				volatile S32 breakhere = 1;

			AftkUt::C_NodeFlagParser::ClearAllFlags(*pSceneNode);

			/*if (_stricmp(nodeFlags.GetFlagGroupCategory().c_str(), "Crowd") == 0)
				nodeFlags.SetFlagGroupCategory("");*/

			/*for (U32 i = 0, count = pSceneNode->GetNumInstancesOfThisNode(); i < count; ++i)
				StripCrowdFlags(pSceneNode->GetInstanceOfThisNode(i));*/
		}

		void GetUniqueJointList(const Aftk::C_SceneNode* pSceneNode, Btl::Vector<const Aftk::C_SceneNode*>& joints)
		{
			if (!pSceneNode)
				return;
			
			if (pSceneNode->GetType() == Aftk::E_ObjType_SceneJoint)
			{
				bool bFound = false;

				for (S32 i = 0, count = joints.Size(); i < count; ++i)
				{
					if (_stricmp(joints[i]->GetName().c_str(), pSceneNode->GetName().c_str()) == 0)
					{
						bFound = true;
						break;
					}
				}

				if (!bFound)
					joints.PushBack(pSceneNode);
			}

			for (S32 i = 0, count = pSceneNode->GetNumChildren(); i < count; ++i)
				GetUniqueJointList(pSceneNode->GetChildPtr(i), joints);
		}

		void BuildSkeletonJointMap(Btl::Vector<const Aftk::C_SceneNode*>& joints,
			const Btl::Vector<const Aftk::C_SceneNode*>& skeletonNodes)
		{
			joints.Clear();

			for (S32 i = 0, count = skeletonNodes.Size(); i < count; ++i)
			{
				const Aftk::C_SceneNode* pRoot = skeletonNodes[i];

				if (pRoot && pRoot->GetNumChildren())
				{
					GetUniqueJointList(pRoot, joints);
				}
			}
		}

		void SetCrowdInstanceIndex(Aftk::C_SceneNode* pSceneNode, const S32 index)
		{
			if (!pSceneNode)
				return;

			C_NodeFlagParser nodeFlags(*pSceneNode);

			nodeFlags.SetParameterValue("CrowdInstanceIndex", index);
		}

		S32 GetCrowdInstanceIndex(const Aftk::C_SceneNode* pSceneNode)
		{
			if (!pSceneNode)
				return -1;

			bool status = false;

			C_NodeFlagParser nodeFlags(*pSceneNode, &status);

			if (!status)
				return -1;

			S32 crowdInstanceIndex = -1;

			if (nodeFlags.GetParameterValue("CrowdInstanceIndex", crowdInstanceIndex))
				return crowdInstanceIndex;

			return pSceneNode->GetUniqueId();
		}
	} // namespace CrowdHelper
} // namespace AftkUt
