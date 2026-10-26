#ifndef MAYA_IO_LIMIT_MIXIN_IMPORT_H
#define MAYA_IO_LIMIT_MIXIN_IMPORT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace MayaIo
{
	class C_Importer;

	class C_LimitMixinImport
	{
	public:
		static Aftk::C_Status Import(
			C_Importer* pImport,
			const MDagPath &dagPath,
			Aftk::C_LimitMixin* pLimitMixin,
			const C_ImportOptions* pOptions);
	};
}

#endif
