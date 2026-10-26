#include "stdafx.h"

#include <tools_shared/PlatformHelpers.h>
#include <tools_shared/file_copy/FileCopy.h>

#include <tools_shared/shim/ShimDefinitions.h>
#include <tools_shared/executer/ExecuterHelpers.h>
#include <tools_shared/shim/ShimOptions.h>
#include <tools_shared/shim/ShimFileCopyXmlExport.h>


namespace Tools
{
namespace Shim
{

Util::C_String C_FileCopyXmlExport::BuildXmlFileListName(
	const Util::C_String& inputFilePath,
	const Util::C_String& suffix)
{
	Util::C_String directory, filename, extension;
	Util::DeconstructFileString(inputFilePath, directory, filename, extension);
	return directory + filename + suffix;
}

void C_FileCopyXmlExport::ClearXmlFileList(
	const Util::C_String& inputFilePath,
	const Util::C_String& revision)
{
	Util::C_String copyToGameFilePath = BuildXmlFileListName(inputFilePath, Util::C_String("_") + revision + "_copytogame.xml");
	if (Sys::C_FileSystem::Instance().FileExists(copyToGameFilePath))
	{
		LOGINFO( g_log, "Clearing XML file export list: %s\n", copyToGameFilePath.Chars());
		Sys::C_FileSystem::Instance().DeleteFile(copyToGameFilePath);
	}
	Util::C_String copyToWorkingFilePath = BuildXmlFileListName(inputFilePath, Util::C_String("_") + revision + "_copytoworking.xml");
	if (Sys::C_FileSystem::Instance().FileExists(copyToWorkingFilePath))
	{
		LOGINFO( g_log, "Clearing XML file export list: %s\n", copyToWorkingFilePath.Chars());
		Sys::C_FileSystem::Instance().DeleteFile(copyToWorkingFilePath);
	}
}

bool C_FileCopyXmlExport::ExportXmlFileList(
	const Util::C_String& rootTempPath,
	Tools::FileCopy::C_FileCopyList& uberCopyList,
	const Util::C_String& inputFilePath,
	const CopyLocations& copyLocations,
	const Util::C_String& launchComputerName,
	const Util::C_String& revision,
	Util::C_String* pCopyToWorking,
	Util::C_String* pCopyToImage)
{
	//Create output folder if needed.
	{
		Util::C_String outputDir = Util::ExtractDirectory(inputFilePath);
		if (!Sys::C_FileSystem::Instance().FileExists(outputDir))
			Sys::C_FileSystem::Instance().CreateDirectory(outputDir);
	}

	//Create copy-to-game xml file
	Util::C_String copyToGameXmlFilePath;
	{
		for (S32 platform=0; platform<Util::E_Platform_Count; ++platform)
		{
			Tools::FileCopy::FileCopyItemArray& platformListItems =
				uberCopyList.GetFileCopyItemArray((Util::E_Platform)platform);

			for (S32 i=0; i<platformListItems.Size(); ++i)
			{
				if (platform == (S32)Util::E_Platform_360)
				{
					platformListItems[i].m_FileCopyDestination =
						Util::FormatWindowsFileString(platformListItems[i].m_FileCopySource).IReplace(
									Util::FormatWindowsFileString(copyLocations[platform].workingPath), 
									Util::FormatWindowsFileString(copyLocations[platform].copyPath));
				}
				else
				{
					platformListItems[i].m_FileCopyDestination =
						Util::ConvertLocalPathToABNetworkPath(
							Util::FormatWindowsFileString(platformListItems[i].m_FileCopySource).IReplace(
									Util::FormatWindowsFileString(copyLocations[platform].workingPath), 
									Util::FormatWindowsFileString(copyLocations[platform].copyPath)), 
									launchComputerName);
				}
			}
		}

		FileCopy::C_FileCopyListXml xmlList;
		xmlList.ReadFromFileCopyList(uberCopyList, copyLocations);

		copyToGameXmlFilePath = BuildXmlFileListName(
			inputFilePath, 
			Util::C_String("_") + revision + "_copytogame.xml");
		LOGINFO( g_log, "Creating XML file export list: %s\n", copyToGameXmlFilePath.Chars());

		FileCopy::XmlWrite(xmlList, copyToGameXmlFilePath.Chars());
		if (pCopyToImage)
			*pCopyToImage = copyToGameXmlFilePath;
	}

	//Create copy-to-working xml file
	Util::C_String copyToWorkingXmlFilePath;
	{
		{
			FileCopy::C_FileCopyList copyToWorkingList;
			for (S32 platform=0; platform<Util::E_Platform_Count; ++platform)
			{
				FileCopy::FileCopyItemArray& platformListItems =
					uberCopyList.GetFileCopyItemArray((Util::E_Platform)platform);

				for (S32 i=0; i<platformListItems.Size(); ++i)
				{
					if( Util::FormatWindowsFileString(platformListItems[i].m_FileCopySource).ISearch(
						Util::FormatWindowsFileString(copyLocations[platform].workingPath)) == -1 )
					{
						LOGWARN( g_log, "File in file list is not under the working root: %s\n", platformListItems[i].m_FileCopySource.Chars() );
					}
					else
					{
						Tools::FileCopy::C_FileCopyItem copyToWorkingItem;
						copyToWorkingItem.m_FileCopySource = platformListItems[i].m_FileCopySource;
						copyToWorkingItem.m_FileCopyDestination =
							Util::ConvertLocalPathToABNetworkPath(
							platformListItems[i].m_FileCopySource, 
							launchComputerName);
						
						copyToWorkingList.AddFileCopyItem(Util::E_Platform_PC, copyToWorkingItem);
					}
				}
			}

			Tools::FileCopy::C_FileCopyListXml xmlList;
			xmlList.ReadFromFileCopyList(copyToWorkingList, copyLocations);

			copyToWorkingXmlFilePath = BuildXmlFileListName(inputFilePath, Util::C_String("_") + revision + "_copytoworking.xml");
			LOGINFO( g_log, "Creating XML file export list: %s\n", copyToWorkingXmlFilePath.Chars());
			FileCopy::XmlWrite(xmlList, copyToWorkingXmlFilePath.Chars());
			if (pCopyToWorking)
				*pCopyToWorking = copyToWorkingXmlFilePath;
		}
	}

	//Write out temp file describing the location of our exported files.
	{
		Util::C_String tmpFile = rootTempPath + "\\ubercopy\\";
		tmpFile = tmpFile + revision + Util::C_String(".ubercopy");

		Sys::C_XmlDom dom;
		Sys::C_XmlDomElement* pNode = new Sys::C_XmlDomElement();
		pNode->GetName() = "ubercopy";
		dom.SetRootNodePtr(pNode);

		pNode->GetAttributes().Add("copy_to_game", copyToGameXmlFilePath);
		pNode->GetAttributes().Add("copy_to_working", copyToWorkingXmlFilePath);

		LOGINFO( g_log, "Creating ubercopy info file: %s\n", tmpFile.Chars());
		if (!dom.Write(tmpFile.Chars()))
			return false;
	}

	return true;
}

}
}