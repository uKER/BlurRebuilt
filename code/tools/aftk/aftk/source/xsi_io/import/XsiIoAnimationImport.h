#ifndef XSI_IO_ANIMATION_IMPORT_H
#define XSI_IO_ANIMATION_IMPORT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace XsiIo
{
	class C_SceneImport;

	class AnimationImport
	{
	public:
		static Aftk::C_Status Import(
			C_SceneImport* pSceneImport,
			CSLModel* pXsiNode, 
			Aftk::C_SceneNode* pAftkSceneNode);
	};
}

#endif
