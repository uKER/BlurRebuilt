
#include "stdafx.h"

#include <system/misc/StringUtil.h>
#include <tools_shared/PlatformHelpers.h>
#include <tools_shared/file_copy/FileCopyList.h>

#include <converter_shared/helpers/ExportHelpers.h>

namespace ExportHelpers
{

Sys::I_File* WriteExportFile( const Util::C_String& outputFilename, const Tools::E_Platform platform, Tools::FileCopy::C_FileCopyList* pFileCopyList )
{
	bool success = true;

	LOGDEBUG( Tools::g_log, "Creating file '%s'", outputFilename.Chars());

	//Create output folder.
	const Util::C_String outputDirectory = Util::ExtractDirectory( outputFilename );
	success = Sys::C_FileSystem::Instance().CreateDirectory( outputDirectory );
	if (!success)
	{
		LOGWARN( Tools::g_log, "Failed to create output directory '%s'", outputDirectory.Chars());
		return NULL;
	}

	Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open(
		outputFilename.Chars(), 
		Sys::E_FileAccess_Write);

	if (!pFile)
	{
		LOGWARN( Tools::g_log, "Failed to create output file '%s'", outputFilename.Chars());
		return NULL;
	}

	const bool endianSwap = Tools::C_PlatformHelper::ShouldEndianSwap( platform );
	pFile->SetEndianSwapFlag(endianSwap);

	pFileCopyList->AddFileCopyItem( platform, Tools::FileCopy::C_FileCopyItem((const char*)outputFilename.Chars(), "") );

	return pFile;
}


} // namespace ExportHelpers