#ifndef MAYA_IO_MATERIAL_EXPORT_H
#define MAYA_IO_MATERIAL_EXPORT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace MayaIo
{
	class C_Exporter;

	class C_MaterialExport
	{
	public:
		static Aftk::C_Status Export(	
			C_Exporter* pExporter,
			const Aftk::C_Scene* pAftkScene,
			const Aftk::C_MaterialNode* pAftkMaterialNode,
			MObject* pMayaMaterialObject,
			MObject* pMayaMaterialShadingGroupObject);
	};
}

#endif

