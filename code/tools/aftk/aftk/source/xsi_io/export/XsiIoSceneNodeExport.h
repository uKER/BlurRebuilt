#ifndef XSI_IO_SCENE_NODE_EXPORT_H
#define XSI_IO_SCENE_NODE_EXPORT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace XsiIo
{
	class C_SceneExport;

	class C_SceneNodeExport
	{
	public:

		static Aftk::C_Status Export(
			C_SceneExport* pExporter,
			CSLModel* pXsiNode,
			const Aftk::C_Scene* pAftkScene,
			const Aftk::C_SceneNode* pAftkSceneNode);
	
	private:
		static Aftk::C_Status AftkDynamicPlugToXsiCustomPSet(
			C_SceneExport* pExporter,
			CSLModel *pXsiNode,
			const Aftk::C_SceneNode* pAftkSceneNode,
			CSLCustomPSet *pXsiParamSet,
			const Aftk::I_Plug* pAftkDynamicPlug);

		static Aftk::C_Status ExportParameter(
			C_SceneExport* pExporter,
			CSLModel *pXsiNode,
			const Aftk::C_SceneNode* pAftkSceneNode,
			CSLCustomPSet *pXsiParamSet,
			CSLVariantParameter *pXsiParam,
			const Aftk::I_Plug* pAftkDynamicPlug);

	};
}


#endif
