#ifndef AFTK_UT_REPLACE_WITH_LOD_GROUP_H
#define AFTK_UT_REPLACE_WITH_LOD_GROUP_H

namespace AftkUt
{
	class C_ReplaceWithLodGroup
	{
	public:
		/// Replace the given node with a lod group, complete with children that instance the
		/// children of the nominated source lod group.
		static Aftk::C_SceneLodGroupNode* ReplaceWithLodGroup(
			Aftk::C_SceneNode* pToReplace,
			const Aftk::C_SceneLodGroupNode* pLodGroup);
	};
}

#endif
