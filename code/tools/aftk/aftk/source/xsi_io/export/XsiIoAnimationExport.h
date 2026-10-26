#ifndef XSI_IO_ANIMATION_EXPORT_H
#define XSI_IO_ANIMATION_EXPORT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace XsiIo
{
	class C_SceneExport;

	class C_AnimationExport
	{
	public:

		static Aftk::C_Status Export(
			C_SceneExport* pExporter,
			CSLModel* pXsiNode,
			const Aftk::C_Scene* pAftkScene,
			const Aftk::C_SceneNode* pAftkSceneNode);

	};
}


#endif
