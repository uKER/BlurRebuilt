#ifndef NEXUS_CORE_PICK_RESULT_H
#define NEXUS_CORE_PICK_RESULT_H

struct lua_State;

namespace Nexus
{
	struct C_PickResult
	{
		Aftk::C_SceneNode* pSceneNode;
		U32 lodLevel;
		S32 selectedComponent;
		S32 selectedSubComponent;
	};
}

#endif