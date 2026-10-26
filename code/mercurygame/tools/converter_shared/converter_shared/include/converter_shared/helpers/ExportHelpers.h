#ifndef CONVERTER_SHARED_EXPORT_HELPERS_H
#define CONVERTER_SHARED_EXPORT_HELPERS_H

#ifdef _MSC_VER
#pragma once
#endif

// forward decl
namespace Tools
{
	enum E_Platform;

	namespace FileCopy
	{
		class C_FileCopyList;
	};
};

namespace ExportHelpers
{

Sys::I_File* WriteExportFile( const Util::C_String& outputFilename, const Util::E_Platform platform, Tools::FileCopy::C_FileCopyList* pFileCopyList );
void SetExportFileProperties(const Util::C_String& filename, const Util::E_Platform platform);

} // namespace ExportHelpers

#endif
