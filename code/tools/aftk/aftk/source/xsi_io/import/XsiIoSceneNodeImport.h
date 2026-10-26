#ifndef XSI_IO_SCENE_NODE_IMPORT_H
#define XSI_IO_SCENE_NODE_IMPORT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace XsiIo
{
	class C_Main;

	class C_SceneNodeImport
	{
	public:
		static Aftk::C_Status Import(
			C_SceneImport* pSceneImport,
			CSLModel* pXsiNode, 
			Aftk::C_SceneNode* pAftkSceneNode);

	private:
		static Aftk::C_Status XsiCustomPSetToAftkDynamicPlug(
			C_SceneImport* pSceneImport,
			Aftk::C_SceneNode* pAftkSceneNode,
			CSLCustomPSet *pXsiParamSet,
			Aftk::C_DynamicPlug *pAftkDynamicPlug);
	};
}

#endif
