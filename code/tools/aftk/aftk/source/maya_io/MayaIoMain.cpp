#include "stdafx.h"
#include <maya/MCommandResult.h>
#include <maya/MCommandMessage.h>

#include "MayaIoStringTable.h"

//#pragma message("TODO:REMOVE OLD STRING PROC")
#include <core/utility/stringproc.h>

namespace MayaIo
{
	std::string RemoveFileNameInstanceNumber(const std::string& str, Aftk::C_Status& status)
	{
		//1) Is the last character of the file name an end brace?
		if (str[str.length()-1] == '}')
		{
			//2) Search backwards until we find a matching brace.
			std::size_t pos = str.rfind('{');
			if (pos == std::string::npos)
			{
				//Could not remove curly braces.
				AFTK_DO_BEGIN << "Could not remove curly braces." AFTK_DO_ERROR_END_0;
				return str;
			}

			//Remove the curly braces.
			status = Aftk::C_Status::E_Status_Success;
			std::size_t charsToRemove = str.length() - pos;
			return str.substr(0, str.length() - charsToRemove);
		}

		status = Aftk::C_Status::E_Status_Success;
		return str;
	}
}

using namespace MayaIo;

Aftk::C_Status C_Main::Read(
	const std::string& fileName, 
	Aftk::C_Scene *pAftkScene, 
	const char* pOptions,
	Aftk::I_Progress* pProgressCallback)
{
	Aftk::C_Status status;
	MStatus mStatus;
	MItDag itDag2(MItDag::kBreadthFirst, MFn::kTransform, &mStatus);

	std::string fixedFileName = fileName;
	Tu::SearchAndReplace(fixedFileName, "\\", "/");

	{
		
		std::string optionsString = pOptions;
		std::transform (optionsString.begin(),optionsString.end(), optionsString.begin(), tolower);
		MString syncOption = MString("fileOpenSyncRefLoadSetting");

		int sync = 0;
		if (optionsString.find("syncreferences") != std::string::npos)
		{
			std::string command("syncReferences(\"" + fixedFileName + "\")");
			mStatus = MGlobal::executeCommand(MString(command.c_str()));
			if (!mStatus)
			{
				Aftk::C_Status aftkStatus(Aftk::C_Status::E_Status_Failure);
				aftkStatus << "Failed to execute syncReferences. [" << mStatus.errorString().asChar() << "].";
			}
		}

		std::string command(
			"biz_mb_loadSceneAndReloadRefs(\"" + 
			fixedFileName +
			"\", \"\")");
		mStatus = MGlobal::executeCommand(MString(command.c_str()));
		if (!mStatus)
		{
			Aftk::C_Status aftkStatus(Aftk::C_Status::E_Status_Failure);
			aftkStatus << "Failed to execute biz_mb_loadSceneAndReloadRefs. [" << mStatus.errorString().asChar() << "].";
			return aftkStatus;
		}
	}
	{
		std::string command("biz_batchModePreExport()");
		mStatus = MGlobal::executeCommand(MString(command.c_str()));
		if (!mStatus)
		{
			Aftk::C_Status aftkStatus(Aftk::C_Status::E_Status_Failure);
			aftkStatus << "Failed execute biz_batchModePreExport. [" << mStatus.errorString().asChar() << "].";
			return aftkStatus;
		}
	}

	status = ReadExternalScene(
		pAftkScene,
		pOptions,
		pProgressCallback);

	return status;
}

Aftk::C_Status C_Main::Write(
	const std::string &fileName, 
	U32 version, 
	const Aftk::C_Scene *pAftkScene,
	const char* pOptions,
	Aftk::I_Progress* pProgressCallback)
{
	MFileIO::newFile(true);

	Aftk::C_Status status;
	status = WriteExternalScene(
		pAftkScene,
		pOptions,
		pProgressCallback);
	if (!status) return status;

	std::string fixedFileName = fileName;
	Tu::SearchAndReplace(fixedFileName, "\\", "/");

	MStatus mStatus;
	mStatus = MFileIO::saveAs(fixedFileName.c_str(), 0, true);
	if (!mStatus) 
	{
		Aftk::C_Status aftkStatus(Aftk::C_Status::E_Status_Failure);
		aftkStatus << "Failed to save file. [" << mStatus.errorString().asChar() << "].";
		return aftkStatus;
	}

	MFileIO::newFile(true);
	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_Main::ReadExternalScene(
	Aftk::C_Scene *pAftkScene,
	const char* pOptions,
	Aftk::I_Progress* pProgressCallback)
{
	DisableAutoKey();
	C_Importer importer;

	Aftk::C_Status status = importer.Import(
		pAftkScene,
		pOptions,
		pProgressCallback);
	if (!status) 
	{
		Aftk::C_Status aftkStatus(Aftk::C_Status::E_Status_Failure);
		aftkStatus << "Maya->Aftk conversion failed. [" << status.GetErrorString() << "].";
		RestoreAutoKey();
		return aftkStatus;
	}
	
	RestoreAutoKey();
	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_Main::WriteExternalScene(
	const Aftk::C_Scene *pAftkScene,
	const char* pOptions,
	Aftk::I_Progress* pProgressCallback)
{
	DisableAutoKey();

	Aftk::C_Status status;
	{
		C_Exporter exporter;
		status = exporter.Export(
			pAftkScene,
			pProgressCallback);
		if (!status) 
		{
			Aftk::C_Status aftkStatus(Aftk::C_Status::E_Status_Failure);
			aftkStatus << "Aftk->Maya conversion failed. [" << status.GetErrorString() << "].";
			RestoreAutoKey();
			return aftkStatus;
		}
	}
	
	RestoreAutoKey();
	return Aftk::C_Status::E_Status_Success;
}

void C_Main::DisableAutoKey()
{
	MStatus mStatus;
	MCommandResult result;
	mStatus = MGlobal::executeCommand("autoKeyframe -query -state;", result, false, false);
	if (mStatus)
	{
		int res = 0;
		mStatus = result.getResult(res);
		if (res == 0)
			m_autoKey = false;
		else
			m_autoKey = true;
	}
	else
	{
		m_autoKey = false;
	}

	mStatus = MGlobal::executeCommand("autoKeyframe -state false;", false, false);
}

void C_Main::RestoreAutoKey()
{
	MStatus mStatus;
	if (m_autoKey)
	{
		mStatus = MGlobal::executeCommand("autoKeyframe -state true;", false, false);
	}
}



