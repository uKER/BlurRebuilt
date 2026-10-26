#ifndef MAYA_IO_DG_DISTANCE_DIM_IMPORT_H
#define MAYA_IO_DG_DISTANCE_DIM_IMPORT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace MayaIo
{
	class C_Importer;

	class C_DgDistanceDimImport : public C_DgNodeImport
	{
	public:
		static Aftk::C_Status Import(
			C_Importer* pImporter,
			Aftk::C_Scene* pAftkScene,
			MFnDependencyNode& fnDependencyNode,
			Aftk::C_Node* pAftkNode);

	private:
	};
}

#endif
