#include "stdafx.h"
#include <system/xml/XmlDom.h>
#include <Aftk/Aftk.h>
#include <AftkUt/AftkUt.h>

#include "Meta.h"

namespace Commands
{

bool ReadWorkFile(C_Options& options)
{
	options.inputs.Clear();

	Sys::C_XmlDom dom;

	if (!dom.Read(options.mergeWorkFile.Chars()))
	{
		LOGERROR(g_log, "Failed to read workfile %s", options.mergeWorkFile.Chars());
		return false;
	}

	Sys::C_XmlDomBaseNode* pInputs = Sys::C_XmlDomUtility::FindXmlNodeByName(dom.GetRootNodePtr(), "inputs");
	if (!pInputs) 
	{
		LOGERROR(g_log, "Failed to find \"inputs\" block in xml");
		return false;
	}

	for (S32 i = 0; i < pInputs->GetChildren().Size(); ++i)
	{
		Sys::C_XmlDomBaseNode* pInput = pInputs->GetChildren()[i];
		if (!pInput) continue;

		Sys::C_FilePath inputFilePath = 
			((Sys::C_XmlDomElement*)(pInput))->GetAttributes().GetAttributeByName("filepath")->value;

		options.inputs.PushBack(inputFilePath);
	}


	return true;
}

E_ReturnCode Merge(C_Options& options)
{
	if (options.mergeWorkFile.Length() != 0)
		if (!ReadWorkFile(options))
			return E_ReturnCode_Fatal;

	Aftk::E_FileType outputType;
	Aftk::C_Global::GetFileType(options.output.Chars(), outputType);
	if (outputType != Aftk::E_FileType_A3d)
	{
		LOGERROR(g_log, "Merge output file %s must be of type A3D", options.output.Chars());
		return E_ReturnCode_Fatal;
	}

	Aftk::C_Scene scene;
	for (S32 i = 0; i < options.inputs.Size(); ++i)
	{
		const Sys::C_FilePath& input = options.inputs[i];
		Aftk::E_FileType inputType;
		Aftk::C_Global::GetFileType(input.Chars(), inputType);
		if (inputType != Aftk::E_FileType_A3d)
		{
			LOGERROR(g_log, "Merge input file %s must be of type A3D", input.Chars());
			return E_ReturnCode_Fatal;
		}
	
		LOGINFO(g_log, "Importing %i/%i: %s", i+1, options.inputs.Size(), input.Chars());

		Aftk::C_Scene importScene;
		if (!importScene.Read(input.Chars()))
		{
			LOGERROR(g_log, "Failed to read scene %s", input.Chars());
			return E_ReturnCode_Fatal;
		}

		if (!AftkUt::C_Import::Import(&scene, &importScene, 0))
		{
			LOGERROR(g_log, "Failed to import scene %s", input.Chars());
			return E_ReturnCode_Fatal;
		}
	}

	LOGINFO(g_log, "Writing output %s...", options.output.Chars());
	if (!scene.Write(options.output.Chars()))
	{
		LOGERROR(g_log, "Failed to write scene %s", options.output.Chars());
		return E_ReturnCode_Fatal;
	}

	LOGINFO(g_log, "Merging successful.");
	return E_ReturnCode_Success;
}

}
