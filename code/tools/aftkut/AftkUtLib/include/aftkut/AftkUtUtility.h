#ifndef AFTK_UT_UTILITY_H
#define AFTK_UT_UTILITY_H

namespace AftkUt
{
	class C_Utility
	{
	public:
		static Aftk::C_Status ReparentInstance(
			Aftk::C_SceneNode* pInstance, 
			Aftk::C_SceneNode* pInstanceSource);


		static Aftk::C_SceneNode* GetInstanceSourceNodePtr(Aftk::C_SceneNode* pNode);

		static const Aftk::C_SceneNode* GetInstanceSourceNodePtr(const Aftk::C_SceneNode* pNode);
	};
}

#endif
