#ifndef CONVERSION_OPTIONS_H
#define CONVERSION_OPTIONS_H

#ifdef _MSC_VER
#pragma once
#endif

enum E_CopyType
{
	E_CopyType_Unknown,
	E_CopyType_File,
	E_CopyType_Folder
};

struct C_Options : public Tools::Shim::C_Options
{
public:
	bool ParseCommandLine(
		Sys::C_FilePath& xmlFilePath,
		int argc, 
		char* argv[]);

	virtual bool ParseXml(const Sys::C_FilePath& xmlFilePath, const Sys::C_XmlDom& dom);


	Sys::C_FilePath inputAssetRootPath;

	Sys::C_FilePath inputAssetFilePath;
	//Dummy filepath for input file in our temp folder.
	//This is where our ubercopy xml files will live.
	Sys::C_FilePath inputTempFilePath;
	//Where our copied file resides in the working folder.
	Sys::C_FilePath outputPath;

	E_CopyType copyType;

	bool exportForPlatform[Util::E_Platform_Count];

private:
	bool ParseOutputDirectory(const Sys::C_XmlDomBaseNode* pExportType);

	bool ParseInputAssetFilePath(
		const Sys::C_XmlDomBaseNode* pExportType,
		const Sys::C_FilePath& partialDirectory);
};

#endif
