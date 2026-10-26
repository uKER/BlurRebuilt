#ifndef EDITOR_DG_PROP_NODE_CONTEXT_HELPERS_H
#define EDITOR_DG_PROP_NODE_CONTEXT_HELPERS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	namespace PropContextHelpers
	{
		const EditorLib::C_SceneNode* GetTopLevelParent(
			const EditorLib::C_DgXtPropNodeBase& component);

		EditorLib::C_SceneNode* GetTopLevelParent(
			EditorLib::C_DgXtPropNodeBase& component);
	}
}

#endif
