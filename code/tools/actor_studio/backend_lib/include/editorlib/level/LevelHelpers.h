#ifndef EDITOR_LIB_LEVEL_HELPERS_H
#define EDITOR_LIB_LEVEL_HELPERS_H

#pragma once

namespace EditorLib
{
	namespace LevelHelpers
	{
		enum E_DeleteSceneNode
		{
			E_DeleteSceneNode_ReparentChildrenAbsolute,
			E_DeleteSceneNode_ReparentChildren,
			E_DeleteSceneNode_DeleteChildren
		};

		//Creation and cloning.
		I_DgNode*	CreateNode(
			C_XtLevel* pTargetLevel, const char* typeName, const char* name = 0);

		I_DgNode*	CloneNode(
			C_XtLevel* pTargetLevel, I_DgNode* pSrc);

		bool DestroyNode(
			I_DgNode* pNode, 
			E_DeleteSceneNode childBehaviour = E_DeleteSceneNode_ReparentChildrenAbsolute, 
			bool deleteOverride = true);

		//Mass renaming
		void RenameSelection(const C_Selection& selection);

		Btl::String GetNextFreeName(const Btl::String& name);

		//Export content to another level.
		XtId		ExportNode(XtId targetLevelId, I_DgNode* pSourceNode);
		void		ExportNodeLayers(XtId targetLevelId, XtId targetNodeId, I_DgNode* pSourceNode);
	}
}

#endif
