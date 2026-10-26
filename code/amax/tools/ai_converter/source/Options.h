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

	const Util::C_String& GetOutputFilename() const;
	const Util::C_String& GetInputTempFilePath() const;
	const Util::C_String& GetOutputPath() const;
	const Util::C_String& GetMayaSceneValue() const;
	const Util::C_String& GetLevelPath() const;
	const Util::C_String& GetTrackName() const;

	bool ShouldExport() const;

private:
	bool ParseOutputDirectory(const Sys::C_XmlDom& dom);

	bool ParseInputAssetFilePath(
		const Sys::C_XmlDomBaseNode* pNode,
		const Util::C_String& partialDirectory);

	Util::C_String m_inputAssetFilePath;

	//Dummy filepath for input file in our temp folder.
	//This is where our ubercopy xml files will live.
	Util::C_String m_inputTempFilePath;

	//Where our copied file resides in the working folder.
	Util::C_String m_outputFilename;
	Util::C_String m_outputPath;
	Util::C_String m_mayaSceneValue;
	Util::C_String m_mergedPathValue;
	Util::C_String m_outputNameValue;
	Util::C_String m_levelPathValue;
	Util::C_String m_trackName;
	bool m_shouldExport;

};

#endif
