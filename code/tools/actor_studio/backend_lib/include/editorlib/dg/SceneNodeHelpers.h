#ifndef EDITOR_LIB_SCENE_NODE_HELPERS_H
#define EDITOR_LIB_SCENE_NODE_HELPERS_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{
	class C_SceneNodeHelpers
	{
	public:
		///\brief Removes a node from the hierarchy, and reparents any children it might have.
		///
		///Note that pNode is NOT deleted.
		///
		///@param pNode The node to extract.
		///@param updateChildMatrices If true, children get their parents matrix.
		static void ExtractSceneNodeAndReparentChildren(C_SceneNode* pNode, bool updateChildMatrices);

		///\brief Reparents the given node.
		///
		///@param pNode The node to reparent.
		///@param pNewParent The new parent node
		static bool ReparentSceneNode(C_SceneNode* pNode, C_SceneNode* pNewParent, bool absoluteTransform);

		///\brief Deletes the given node and reparents its children.
		///
		///@param pNode The node to delete.
		///@param updateChildMatrices If true, children get their parents matrix.
		static bool DeleteSceneNodeAndReparentChildren(C_SceneNode* pNode, bool updateChildMatrices, bool deleteOverride);

		///\brief Delete the given node, and all nodes further down the hierarchy.
		///
		///@param pNode The node to begin the deletion from.
		static bool DeleteSceneNodeAndChildren(C_SceneNode* pNode, bool deleteOverride);

		///\brief Returns how deeply nested we are.
		static U32 GetDepth(C_SceneNode* pNode);

	private:
		static void _DeleteSceneNodeAndChildrenRecurse(C_SceneNode* pNode, bool deleteOverride);
	};
}

#endif
