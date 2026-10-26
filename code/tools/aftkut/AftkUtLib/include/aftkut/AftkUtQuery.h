#ifndef AFTK_UT_QUERY_H
#define AFTK_UT_QUERY_H

namespace AftkUt
{
	namespace Query
	{
		union C_Flags
		{
			C_Flags() : data(0) {}

			U32 data;
			struct
			{
				bool baseInstance : 1;
				bool doNotExport : 1;
				bool asDynamic : 1 ;
				bool asActor : 1 ;
				bool collisionPrimitive : 1;
				bool isConstructionNode : 1;
				bool asCrowd : 1 ;
				bool decal : 1;
				bool isBillboard : 1;
			};
		};

		bool IsInstance(const Aftk::C_SceneNode* pSceneNode);
		bool IsBaseInstance(const Aftk::C_SceneNode* pSceneNode);
		bool IsBaseCrowdInstance(const Aftk::C_SceneNode* pSceneNode);
		bool IsCrowdSkeletonNode(const Aftk::C_SceneNode* pSceneNode);
		bool IsCrowdInstance(const Aftk::C_SceneNode* pSceneNode);
		bool IsFlagged(const Aftk::C_SceneNode* pSceneNode);

		C_Flags GetConventionFlags(const Aftk::C_SceneNode* pSceneNode);

		Aftk::C_SceneNode* GetTopLevelSourceNodePtr(const Aftk::C_SceneNode* pInstance);
	}
}

#endif
