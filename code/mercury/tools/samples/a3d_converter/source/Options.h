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

	bool ParseXml(const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom);

	const Util::C_String& GetInputTempFilePath() const;

	const Util::C_String& GetOuptutPath() const;

	S32 GetExampleExecuterInteger() const;

	const Util::C_String& GetExampleExecuterString() const;

private:
	//Generated a3d file lives within temp folder.
	Util::C_String	m_inputTempFilePath;

	//Where our copied file resides in the working folder.
	Util::C_String	m_outputPath;
	
	//The following two variables are filled out by parsing our Executer XML file.
	S32				m_exampleExecuterInteger;
	Util::C_String	m_exampleExecuterString;

private:
};

#endif
