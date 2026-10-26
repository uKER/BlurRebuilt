#ifndef XSI_IO_SCENE_IK_NODE_IMPORT_H
#define XSI_IO_SCENE_IK_NODE_IMPORT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace XsiIo
{
	class C_Main;

	class C_SceneIkNodeImport
	{
	public:
		static Aftk::C_Status Import(
			C_SceneImport* pSceneImport,
			CSLModel* pXsiNode, 
			Aftk::C_SceneNode* pAftkSceneNode);
	};
}

#endif
