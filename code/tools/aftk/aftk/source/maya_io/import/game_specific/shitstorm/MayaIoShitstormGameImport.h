#ifndef MAYA_IO_SHITSTORM_GAME_IMPORT_H
#define MAYA_IO_SHITSTORM_GAME_IMPORT_H


#ifdef _MSC_VER_
#pragma once
#endif

namespace MayaIo
{
	class C_ShitstormGameImport : public I_GameImport
	{
		virtual ~C_ShitstormGameImport();

		Aftk::C_Status PreProcessScene(
			C_Importer *pImporter,
			const C_ImportOptions* pOptions);

		Aftk::C_Status PostProcessScene(
			C_Importer* pImporter,
			const C_ImportOptions* pOptions);

		bool PreserveChildJointTransforms(
			C_Importer* pImporter,
			const MDagPath& path) const;
	};
}

#endif

