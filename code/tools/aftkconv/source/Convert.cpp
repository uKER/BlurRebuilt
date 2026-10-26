#include "stdafx.h"
#include "Convert.h"
#include "Definitions.h"
#include "Maya.h"
#include "Meta.h"
#include <AftkConv/Version.h>


#include <Tools_Shared/Utility/AssetMeta.h>
#include <Aftk/Aftk.h>

namespace Commands
{

E_ReturnCode Convert(C_Options& options)
{
	if (options.inputs.Size() != 1)
	{
		LOGINFO(g_log, "1 input must be specified, %i provided.", options.inputs.Size());
		return E_ReturnCode_Fatal;
	}

	const Sys::C_FilePath& input = options.inputs[0];

	
	if (!options.forceConversion && !Tools::MetaMismatch(
		options.output.Chars(), 
		input.Chars(),
		AFTK_CONV_CURRENT_VERSION))
	{
		LOGINFO(g_log, "Metadata match - no need for conversion.");
		return E_ReturnCode_Success;
	}
	else
		LOGINFO(g_log, "Metadata mismatch - conversion required.");

	//Boot Maya if needed.
	Aftk::E_FileType inputType, outputType;
	Aftk::C_Global::GetFileType(input.Chars(), inputType);
	Aftk::C_Global::GetFileType(options.output.Chars(), outputType);
	if ((inputType == Aftk::E_FileType_Mb || inputType == Aftk::E_FileType_Ma ||
		outputType == Aftk::E_FileType_Mb || outputType == Aftk::E_FileType_Ma)
		&&
		!InitialiseMaya())
		return E_ReturnCode_Fatal;

	//Convert
	{
		Aftk::C_Scene scene;
		LOGINFO(g_log, "Reading input %s with args \"%s\"...", input.Chars(), options.inputArgs.Chars());
		if (!scene.Read(input.Chars(), inputType, options.inputArgs.Chars()))
		{
			LOGERROR(g_log, "Failed to read scene %s", input.Chars());
			return E_ReturnCode_Fatal;
		}

		scene.SetRootFilePath(input);

		LOGINFO(g_log, "Writing output %s with args \"%s\"...", options.output.Chars(), options.outputArgs.Chars());
		if (!scene.Write(options.output.Chars(), outputType, 0, options.outputArgs.Chars()))
		{
			LOGERROR(g_log, "Failed to write scene %s", options.output.Chars());
			return E_ReturnCode_Fatal;
		}
	}
	if (!SaveMeta(input.Chars(), options.output.Chars()))
		return E_ReturnCode_Fatal;

	LOGINFO(g_log, "Conversion successful.");
	return E_ReturnCode_Success;
}

}
