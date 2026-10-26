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

	Sys::ShellExec::E_LaunchState GetShellExecLaunchState() const;

	E_ShimMode GetMode() const;
	const Sys::C_XmlDomElement* GetActionNode() const;

	const Util::C_String& GetPlatformName() const;

	const Util::C_String& GetFlags() const;

	// Alienbrain data
	const Util::C_String& GetAlienbrainScript() const;
	const Util::C_String& GetAlienbrainUser(const Util::C_String& server) const;
	const Util::C_String& GetAlienbrainPass(const Util::C_String& server) const;
	const Util::C_String* GetAlienbrainServer(const Util::C_String& path) const;

private:
	E_ShimMode m_shimMode;
	const Sys::C_XmlDomElement*		m_pShimActionNode;

	Util::C_String					m_shimFlags;

	Sys::ShellExec::E_LaunchState	m_shimShellExecLaunchState;
	Util::C_String					m_shimPlatformName;
};

#endif
