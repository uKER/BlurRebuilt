#include "stdafx.h"
#include "../Aftk/Ftk.h"
#include "FbxIoMain.h"
#include "FbxIoUtility.h"
#include "import/FbxIoImportOptions.h"
#include "import/FbxIoSceneImport.h"
#include <core/utility/stringproc.h>
#include <fbxsdk.h>

using namespace FbxIo;

Aftk::C_Status C_Main::Write(
	const std::string &fileName, 
	U32 version, 
	const Aftk::C_Scene *pAftkScene, 
	const char* pOptions,
	Aftk::I_Progress* pProgressCallback) 
{
	return Aftk::C_Status::E_Status_NotImplemented;
}

Aftk::C_Status C_Main::Read(
	const std::string& fileName,
	Aftk::C_Scene *pAftkScene, 
	const char* pOptions,
	Aftk::I_Progress* pProgressCallback)
{
	Aftk::C_Status status;
	
	if (pProgressCallback)
	{
		if (pProgressCallback->OnProgress(0))
		{
			return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Aftk::C_Scene loading aborted.");
		}
		pProgressCallback->OnPrintProgress(std::string("Reading ") + fileName + std::string("..."));
	}

	//Convert string parameters to AFTK ImportOptions
	C_ImportOptions readOptions;
	if (pOptions)
	{
		std::string optionsStr = pOptions;
		status = GetParameters(readOptions, optionsStr);
		if (!status) return status;
	}
	
	//Create FBX memory manager and scene
	KFbxSdkManager* pSdkManager = KFbxSdkManager::Create();

    if (!pSdkManager)
    {
        return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Unable to create the FBX SDK manager.");
    }

    KFbxScene* pScene = KFbxScene::Create(pSdkManager,"");

	//Create FBX import options
	KFbxStreamOptionsFbxReader* pImportOptions = KFbxStreamOptionsFbxReader::Create(pSdkManager, "");

	// Create FBX importer
	KFbxImporter* pImporter = KFbxImporter::Create(pSdkManager,"");

	int fileFormat = -1;

	// Detect the file format of the file to be imported
	if (!pSdkManager->GetIOPluginRegistry()->DetectFileFormat(fileName.c_str(), fileFormat))
    {
        // Unrecognizable file format.
        // Try to fall back to SDK's native file format (an FBX binary file).
        fileFormat = pSdkManager->GetIOPluginRegistry()->GetNativeReaderFormat();
    }
    pImporter->SetFileFormat(fileFormat);

	// Initialize the importer by providing a filename.
    bool importStatus = pImporter->Initialize(fileName.c_str());

	if( !importStatus )  // Problem with the file to be imported
    {
		std::string err = "Call to KFbxImporter::Initialize() failed. Cause: ";
		err += pImporter->GetLastErrorString();
		return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, err);
	}

	
	int takes = pImporter->GetTakeCount();
	std::string takeName = pImporter->GetCurrentTakeName();
	
	if (pImporter->IsFBX())
    {
        // Import options determine what kind of data is to be imported.
        // The default is true, but here we set the options explictly.
        pImportOptions->SetOption(KFBXSTREAMOPT_FBX_MATERIAL,        true);
        pImportOptions->SetOption(KFBXSTREAMOPT_FBX_TEXTURE,         true);
        pImportOptions->SetOption(KFBXSTREAMOPT_FBX_LINK,            true);
        pImportOptions->SetOption(KFBXSTREAMOPT_FBX_SHAPE,           true);
        pImportOptions->SetOption(KFBXSTREAMOPT_FBX_GOBO,            true);
        pImportOptions->SetOption(KFBXSTREAMOPT_FBX_ANIMATION,       true);
        pImportOptions->SetOption(KFBXSTREAMOPT_FBX_GLOBAL_SETTINGS, true);
    }

    // Import the scene.
    importStatus = pImporter->Import(pScene, pImportOptions);

	if(!importStatus)
	{
		if(pImporter->GetLastErrorID() == KFbxIO::ePASSWORD_ERROR)
		{
			pImportOptions->SetOption(KFBXSTREAMOPT_FBX_PASSWORD, readOptions.password);
			pImportOptions->SetOption(KFBXSTREAMOPT_FBX_PASSWORD_ENABLE, true);

			//attempt to import again with the supplied password
			importStatus = pImporter->Import(pScene, pImportOptions);

			if(!importStatus && pImporter->GetLastErrorID() == KFbxIO::ePASSWORD_ERROR)
			{
				return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Incorrect password, file not imported."); 
			}
		} else 
		{
			return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Failed to import the FBX file. Unknown error."); 	
		}

	} 

	C_SceneImport importer;
	status = importer.Import(
		pScene,
		pAftkScene,
		pImporter,
		&readOptions,
		pProgressCallback);
	if (!status) return status;
	
	
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

Aftk::C_Status C_Main::GetParameters(C_ImportOptions& options, const std::string& optionsStr)
{
	std::string arguments(optionsStr);
	Tu::SearchAndReplace(arguments, "|", ";");
	if (arguments.size() > 0)
	{
		char delimiter[] = ";";
		char* token;
		
		//Get the first token:
		char* buffer = new char[arguments.length()+1];
		strcpy(buffer, arguments.c_str());
		token = strtok(buffer, delimiter);
		while( token != NULL )
		{
			size_t tokenLength = strlen(token);
			if (tokenLength > 1)
			{
				std::string tokenStd(token);
				if (tokenStd.find("takeName=") != std::string::npos)
				{
					if (!GetOptionValueString(token, options.takeName))
						return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Failed to parse takeName argument.");
				}
				else if (tokenStd.find("password=") != std::string::npos)
				{
					if (!GetOptionValueString(token, options.password))
						return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Failed to parse password argument.");
				}
				else if (tokenStd.find("targetFPS=") != std::string::npos)
				{
					if (!GetOptionValueU32(token, options.targetFPS))
						return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Failed to parse targetFPS argument.");
				}
			}

			//Get next token:
			token = strtok(NULL, delimiter);
		}
		delete[] buffer;
	}

	return Aftk::C_Status::E_Status_Success;
}