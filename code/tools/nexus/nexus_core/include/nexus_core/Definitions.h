#ifndef NEXUS_CORE_DEFINITIONS_H
#define NEXUS_CORE_DEFINITIONS_H

#define NEXUS_CORE_VERSION "2.20"

struct lua_State;

namespace Nexus
{
	class Progress;
	struct C_SelectedNodeInfo;
	enum E_SelectionMode;

	typedef Ut::UtVector<Ut::DynamicVector< std::string> >			StringArray;
	typedef Ut::UtMap<Aftk::C_SceneNode*, C_SelectedNodeInfo>		AftkSceneNodeMap;

	typedef Tu::Status(*OnChangedCallback)(Nexus::Progress* pProgress);

	typedef void(*OnSelectCallback)(E_SelectionMode mode, const C_SelectedNodeInfo& selection);

	enum E_SelectionMode
	{
		E_SelectionMode_Object,
		E_SelectionMode_Face,
		E_SelectionMode_Point,

		E_SelectionMode_Count
	};
};

#endif
