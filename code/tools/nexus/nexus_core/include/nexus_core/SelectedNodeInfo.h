#ifndef NEXUS_CORE_SELECTED_NODE_INFO_H
#define NEXUS_CORE_SELECTED_NODE_INFO_H

namespace Nexus
{
	struct C_ComponentSelection
	{
		C_ComponentSelection()
		{
		}

		C_ComponentSelection(U32 _lodLevel, U32 _componentIndex)
		{
			lodLevel = _lodLevel;
			componentIndex = _componentIndex;
		}

		bool operator ==(const C_ComponentSelection& rhs) const
		{
			if (lodLevel == rhs.lodLevel && 
				componentIndex == rhs.componentIndex)
				return true;

			return false;
		}

		bool operator < (const C_ComponentSelection& rhs) const
		{
			if (componentIndex < rhs.componentIndex)
				return true;

			return false;
		}

		S32 lodLevel;
		S32 componentIndex;
	};

	struct C_SelectedNodeInfo
	{
		C_SelectedNodeInfo()
		{
			pSceneNode = 0;
		}

		C_SelectedNodeInfo(Aftk::C_SceneNode* p)
		{
			pSceneNode = p;
		}

		C_SelectedNodeInfo(C_PickResult& pickResult)
		{
			assert(pickResult.lodLevel != -1 && pickResult.selectedComponent != -1);
			pSceneNode = pickResult.pSceneNode;
			if (pickResult.selectedComponent >= 0)
			{
				selectedComponents.Insert(
					C_ComponentSelection(
						pickResult.lodLevel, 
						pickResult.selectedComponent));
			}
		}

		C_SelectedNodeInfo(const C_SelectedNodeInfo& rhs)
		{
			*this = rhs;
		}

		const C_SelectedNodeInfo& operator=(const C_SelectedNodeInfo& rhs)
		{
			pSceneNode = rhs.pSceneNode;
			selectedComponents = rhs.selectedComponents;

			return *this;
		}

		Aftk::C_SceneNode* pSceneNode;
		Ut::UtSet<C_ComponentSelection> selectedComponents;
	};
}

#endif