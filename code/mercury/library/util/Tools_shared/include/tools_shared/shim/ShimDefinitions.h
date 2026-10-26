#ifndef MERCURY_SHIM_DEFINITIONS_H
#define MERCURY_SHIM_DEFINITIONS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Tools
{
	#define ALIEN_BRAIN_FOLDER	"alienbrainwork"

	namespace Shim
	{
		enum E_ExportInfoMask
		{
			E_ExportInfoMask_Location		= 0x0000000f,
			E_ExportInfoMask_DataSource		= 0x000000f0
		};

		enum E_ExportInfo
		{
			E_ExportInfo_LocationUnknown	= 0x00000000,
			E_ExportInfo_LocationLocal		= 0x00000001,
			E_ExportInfo_LocationNetwork	= 0x00000002,

			E_ExportInfo_DataSourceUnknown	= 0x00000000,
			E_ExportInfo_DataSourceLocal	= 0x00000010,
			E_ExportInfo_DataSourceNetwork	= 0x00000020,
			E_ExportInfo_DataSourceSCM		= 0x00000030
		};
	}
}

#endif
