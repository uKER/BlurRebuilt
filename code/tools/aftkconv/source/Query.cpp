#include "stdafx.h"
#include "Query.h"
#include "Definitions.h"
#include "Meta.h"
#include <AftkConv/Version.h>

#include <Tools_Shared/Utility/AssetMeta.h>

namespace Commands
{

E_ReturnCode Query(C_Options& options)
{
	if (options.inputs.Size() != 1)
	{
		LOGINFO(g_log, "1 input must be specified, %i provided.", options.inputs.Size());
		return E_ReturnCode_Fatal;
	}

	const Sys::C_FilePath& input = options.inputs[0];
	
	if (!Sys::GetDefaultFileSystem()->FileExists(options.output))
	{
		LOGINFO(g_log, "Output missing - conversion required..");
		return E_ReturnCode_MetaMismatch;
	}
	
	if (!options.forceConversion && !Tools::MetaMismatch(
		options.output.Chars(), 
		input.Chars(),
		AFTK_CONV_CURRENT_VERSION))
	{
		LOGINFO(g_log, "Metadata match - no need for conversion.");
		return E_ReturnCode_Success;
	}

	LOGINFO(g_log, "Metadata mismatch - conversion required.");
	return E_ReturnCode_MetaMismatch;
}

}

