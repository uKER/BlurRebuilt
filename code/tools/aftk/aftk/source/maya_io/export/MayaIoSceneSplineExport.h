#ifndef MAYA_IO_SCENE_SPLINE_EXPORT_H
#define MAYA_IO_SCENE_SPLINE_EXPORT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace MayaIo
{
	class C_Exporter;

	class C_SceneSplineExport : public C_SceneNodeExport
	{
	public:
		static Aftk::C_Status Export(
			bool create,
			C_Exporter* pExporter,
			MObject& parentTransformNodeObject,
			MObject& mayaNewTransformObject,
			MObject& mayaNewShapeObject,
			const Aftk::C_Scene* pAftkScene,
			const Aftk::C_SceneNode* pAftkSceneNode);
	};

}

#endif

