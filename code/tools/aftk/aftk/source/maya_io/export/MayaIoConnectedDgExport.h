#ifndef MAYA_IO_ANIMATION_EXPORT_H
#define MAYA_IO_ANIMATION_EXPORT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace MayaIo
{

	class C_Exporter;

	class C_ConnectedDgExport
	{
	public:
		static Aftk::C_Status Export(
			C_Exporter* pExporter,
			MDagPath& transformDagPath,
			MDagPath* pShapeDagPath,
			const Aftk::C_Scene* pAftkScene,
			const Aftk::C_SceneNode* pAftkSceneNode);

	private:
		static Aftk::C_Status ExportDirectlyConnectedPlug(
			C_Exporter* pExporter,
			MDagPath& transformDagPath,
			MDagPath* pShapeDagPath,
			const Aftk::C_Scene *pAftkScene,
			const Aftk::I_Plug* pAftkPlug);

		static Aftk::C_Status C_ConnectedDgExport::ExportCompoundPlug(
			C_Exporter* pExporter,
			MDagPath& transformDagPath,
			MDagPath* pShapeDagPath,
			const Aftk::C_Scene *pAftkScene,
			const Aftk::I_Plug* pAftkPlug,
			MObject& parentAttrObj);

		static Aftk::C_Status C_ConnectedDgExport::ExportSimplePlug(
			C_Exporter* pExporter,
			MDagPath& transformDagPath,
			MDagPath* pShapeDagPath,
			const Aftk::C_Scene *pAftkScene,
			const Aftk::I_Plug* pAftkPlug,
			MObject& parentAttrObj);

	};
}
#endif
