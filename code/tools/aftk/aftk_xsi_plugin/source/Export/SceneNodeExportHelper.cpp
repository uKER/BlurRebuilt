#include "XSIA3DExportPCH.h"
#include "ExportOptions.h"
#include "SceneNodeExportHelper.h"
#include "ISceneNodeExport.h"
#include "SceneNodeExport.h"
#include "SceneMeshNodeExport.h"

namespace XSITrans
{

Aftk::C_SceneNode* C_SceneNodeExportHelper::Export(
	Aftk::C_Scene& aftkScene,
	const C_ExportOptions& options,
	XSI::X3DObject xsiNode)
{
	Aftk::C_SceneNode* pSceneNode;

	XSI::Primitive prim = xsiNode.GetActivePrimitive();
	if (!prim.IsValid())
	{
		C_SceneNodeExport exporter;
		pSceneNode = exporter.Export(
			aftkScene,
			options,
			xsiNode);
	}
	else
	{
		XSI::Geometry geo = prim.GetGeometry();
		if (geo.IsA(XSI::siPolygonMeshID))
		{
			C_SceneMeshNodeExport exporter;
			pSceneNode = exporter.Export(
				aftkScene,
				options,
				xsiNode);
		}
		else
		{
			C_SceneNodeExport exporter;
			pSceneNode = exporter.Export(
				aftkScene,
				options,
				xsiNode);
		}
	}

	return pSceneNode;
}

}
