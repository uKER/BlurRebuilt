#include "stdafx.h"
#include "../Aftk/Ftk.h"

#include "XsiIoUtility.h"

#include "XsiIoMain.h"
#include "import/XsiIoImportOptions.h"
#include "import/XsiIoSceneImport.h"
#include "export/XsiIoSceneExport.h"

using namespace XsiIo;

Aftk::C_Status C_Main::Write(
	const std::string &fileName, 
	U32 version, 
	const Aftk::C_Scene *pAftkScene, 
	const char* pOptions,
	Aftk::I_Progress* pProgressCallback) 
{
	CSLScene scene;
	SI_Error error;

	if (pProgressCallback)
	{
		if (pProgressCallback->OnProgress(0))
		{
			return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Aftk::C_Scene creation aborted.");
		}
		pProgressCallback->OnPrintProgress(std::string("Writing ") + fileName + std::string("..."));
	}

	try
	{
		error = scene.Create( (char*)fileName.c_str(), 3, 60 );
	}
	catch( ... )
	{
		return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Softimage XSI writer crashed during file creation.");
	}

	if ( error == SI_SUCCESS )
	{
		C_SceneExport exporter;
		exporter.Export( pAftkScene, &scene, pProgressCallback );
		error = scene.Write();
		error = scene.Close();
	}
	else
	{
		return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Could not create file");
	}

	if (pProgressCallback)
	{
		bool onCancel = pProgressCallback->OnProgress(100);
		if (onCancel)
		{
			return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Aftk::C_Scene creation aborted.");
		}
		pProgressCallback->OnPrintProgress("File created successfully.");
	}

	return Aftk::C_Status::E_Status_Success;
	//return Aftk::C_Status::E_Status_NotImplemented;
}
	
Aftk::C_Status C_Main::Read(
	const std::string& fileName,
	Aftk::C_Scene *pAftkScene, 
	const char* pOptions,
	Aftk::I_Progress* pProgressCallback)
{
	Aftk::C_Status status;

	C_ImportOptions readOptions;
/*	for (U32 iOption = 0; iOption < importParameters.GetNumAttributes(); iOption++)
	{
		const Aftk::C_GenericAttribute& attr = importParameters.GetAttribute(iOption);
		if (!strcmp(attr.GetName().c_str(), "cleanuvs"))
		{
			attr.GetValue(readOptions.cleanUVs);
		}
	}
*/
	if (pProgressCallback)
	{
		if (pProgressCallback->OnProgress(0))
		{
			return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Aftk::C_Scene loading aborted.");
		}
		pProgressCallback->OnPrintProgress(std::string("Reading ") + fileName + std::string("..."));
	}

	SI_Error result = SI_SUCCESS; 

	CSLScene scene;
	SI_Error error;

	try
	{
		error = scene.Open(const_cast<char*>(fileName.c_str()));
	}
	catch(...)
	{
		return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Softimage XSI reader crashed during file loading. File is corrupt.");
	}

	if (error == SI_SUCCESS)
	{
		scene.Read();
		C_SceneImport importer;
		status = importer.Import(
			&scene,
			pAftkScene,
			&readOptions,
			pProgressCallback);
		if (!status) return status;
	}
	else
	{
		return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "File not found or could not open file");
	}

	if (pProgressCallback)
	{
		bool onCancel = pProgressCallback->OnProgress(100);
		if (onCancel)
		{
			return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Aftk::C_Scene reading aborted.");
		}
		pProgressCallback->OnPrintProgress("File read successfully.");
	}

	return Aftk::C_Status::E_Status_Success;
} 
