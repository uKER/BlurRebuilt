#ifndef AFTK_UT_MERGE_MESHES_H
#define AFTK_UT_MERGE_MESHES_H

namespace AftkUt
{
	class C_MergeMeshes
	{
	public:
		static Aftk::C_Status Merge(
			Aftk::C_Scene* pScene,
			const Ut::Vector<Aftk::C_SceneMeshNode*>& toMerge,
			Aftk::I_Progress* pProgress = 0);

	private:
	};
}

#endif
