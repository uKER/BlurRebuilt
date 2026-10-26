#ifndef CONVERSION_OPTIONS_H
#define CONVERSION_OPTIONS_H

#ifdef _MSC_VER
#pragma once
#endif

class C_Options : public Tools::Shim::C_Options
{
public:
	C_Options() : Tools::Shim::C_Options(),
		m_triangleLimit(32) {}

	bool ParseCommandLine(
		Util::C_String& xmlFilePath,
		int argc, 
		char* argv[]);

	bool ParseXml(const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom);

	const Util::C_String& GetInputTempFilePath() const;

	const Util::C_String& GetOuptutPath() const;

	U32 GetTriangleLimit() const;

private:
	//Generated a3d file lives within temp folder.
	Util::C_String	m_inputTempFilePath;

	//Where our copied file resides in the working folder.
	Util::C_String	m_outputPath;
	
	U32 m_triangleLimit;
};

#endif
