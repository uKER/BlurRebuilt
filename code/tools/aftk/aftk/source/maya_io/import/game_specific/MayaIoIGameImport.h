#ifndef MAYA_IO_IGAME_IMPORT_H
#define MAYA_IO_IGAME_IMPORT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace MayaIo
{
	class I_GameImport
	{
	public:
		virtual Aftk::C_Status PreProcessScene(
			C_Importer* pImporter,
			const C_ImportOptions* pOptions) = 0;

		virtual Aftk::C_Status PostProcessScene(
			C_Importer* pImporter,
			const C_ImportOptions* pOptions) = 0;

		virtual bool PreserveChildJointTransforms(
			C_Importer* pImporter,
			const MDagPath& path) const = 0;
	};
}

#endif
