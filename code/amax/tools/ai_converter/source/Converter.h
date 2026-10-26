#ifndef AI_CONVERTER__H
#define AI_CONVERTER__H

#ifdef _MSC_VER
#pragma once
#endif

class C_AIConverter
{
public:
	static bool Process(
		Tools::FileCopy::C_FileCopyList& uberCopyList,
		const C_Options& options,
		const Sys::C_XmlDom& dom);

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
