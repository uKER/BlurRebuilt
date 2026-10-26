#ifndef XSI_IO_IMPORT_OPTIONS_H
#define XSI_IO_IMPORT_OPTIONS_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace XsiIo
{
	class C_ImportOptions
	{
	public:
		C_ImportOptions()
		{
			cleanUVs = false;
		}

		bool cleanUVs;
	};
}

#endif
