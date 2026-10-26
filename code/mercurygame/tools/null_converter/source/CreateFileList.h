#ifndef CREATE_FILE_LIST_H
#define CREATE_FILE_LIST_H

#ifdef _MSC_VER
#pragma once
#endif

class C_CreateFileList
{
public:
	static bool Process(
		Tools::FileCopy::C_FileCopyList& uberCopyList,
		const C_Options& options);

	static bool ParseOptions(
		Util::C_String& fileToCopy, 
		Sys::C_XmlDomBaseNode* pNode,
		const Util::C_String& outputDirectory);

private:
	static bool GetOutputDirectory(
		Util::C_String& outputDirectory,
		Sys::C_XmlDom& dom,
		const Util::C_String& rootExportFolder);

};

#endif
