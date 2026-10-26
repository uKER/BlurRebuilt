#ifndef CONVERSION_OPTIONS_H
#define CONVERSION_OPTIONS_H

#ifdef _MSC_VER
#pragma once
#endif

class C_Options : public Tools::Shim::C_Options
{
public:
	bool ParseCommandLine(
		Util::C_String& xmlFilePath,
		int argc, 
		char* argv[]);

	virtual bool ParseXml(const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom);

	const Util::C_String& GetCharlistFilePath() const;
	const Util::C_String& GetTextureFilePath() const;

	const Util::C_String& GetInputAssetFilePath() const;
	//Dummy filepath for input file in our temp folder.
	//This is where our ubercopy xml files will live.
	const Util::C_String& GetInputTempFilePath() const;
	
	//Where our copied file resides in the working folder.
	const Util::C_String& GetOutputPath() const;

private:
	bool ParseOutputDirectory(const Sys::C_XmlDom& dom);

	bool ParseInputAssetFilePath(
		const Sys::C_XmlDomBaseNode* pNode,
		const Util::C_String& partialDirectory);

	Util::C_String m_inputCharlistFile;
	Util::C_String m_inputTextureFile;
	//Dummy filepath for input file in our temp folder.
	//This is where our ubercopy xml files will live.
	Util::C_String m_inputTempFilePath;
	//Where our copied file resides in the working folder.
	Util::C_String m_outputPath;

};

#endif
